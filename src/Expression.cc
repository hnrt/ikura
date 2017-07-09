// Copyright (C) 2014-2017 Hideaki Narita


#include <errno.h>
#include <libintl.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Expression.h"
#include "Exception.h"
#include "Parser.h"
#include "OperatorInfo.h"
#include "VariableStore.h"
#include "SigfpeHandler.h"
#include "LocaleInfo.h"


using namespace hnrt;


//////////////////////////////////////////////////////////////////////
//
// Helper functions
//
//////////////////////////////////////////////////////////////////////


//
// This helper function extracts a long value from each of the given expressions if both are ET_INTEGER.
// If successful, true is returned and the expressions are freed.
// Otherwise, false is returned.
//
static bool getIntegers(Expression* expr1, Expression* expr2, long& value1, long& value2)
{
    if (expr1->getType() == ET_INTEGER &&
        expr2->getType() == ET_INTEGER)
    {
        value1 = ((Integer*)expr1)->getValue();
        value2 = ((Integer*)expr2)->getValue();
        delete expr1;
        delete expr2;
        return true;
    }
    else
    {
        return false;
    }
}


//
// This helper function extracts a long double value from each of the given expressions.
// If successful, true is returned.
// Otherwise, false is returned.
// In either case, the given expressions are freed.
//
static bool getRealNumbers(Expression* expr1, Expression* expr2, long double& value1, long double& value2)
{
    bool retval = true;
    switch (expr1->getType())
    {
    case ET_REALNUMBER:
        value1 = ((RealNumber*)expr1)->getValue();
        break;
    case ET_INTEGER:
        value1 = (long double)((Integer*)expr1)->getValue();
        break;
    case ET_INTEGER_MAX_PLUS_ONE:
        value1 = (long double)((unsigned long)LONG_MAX + 1);
        break;
    default:
        retval = false;
        break;
    }
    switch (expr2->getType())
    {
    case ET_REALNUMBER:
        value2 = ((RealNumber*)expr2)->getValue();
        break;
    case ET_INTEGER:
        value2 = (long double)((Integer*)expr2)->getValue();
        break;
    case ET_INTEGER_MAX_PLUS_ONE:
        value2 = (long double)((unsigned long)LONG_MAX + 1);
        break;
    default:
        retval = false;
        break;
    }
    delete expr1;
    delete expr2;
    return retval;
}


//
// Integer multiplication being aware of the resulting value's overflow / underflow.
//
static long multiply(long value1, long value2)
{
    if (value1 > 1)
    {
        if (value2 > 1)
        {
            if ((unsigned long)value1 > (unsigned long)LONG_MAX / (unsigned long)value2)
            {
                throw OverflowException();
            }
        }
        else if (value2 == 1)
        {
            return value1;
        }
        else if (value2 == 0)
        {
            return 0;
        }
        else if (value2 == -1)
        {
            return -value1;
        }
        else if (value2 == LONG_MIN)
        {
            throw UnderflowException();
        }
        else // if (LONG_MIN < value2 && value2 < -1)
        {
            if ((unsigned long)value1 > ((unsigned long)LONG_MAX + 1UL) / (unsigned long)(-value2))
            {
                throw UnderflowException();
            }
        }
    }
    else if (value1 == 1)
    {
        return value2;
    }
    else if (value1 == 0)
    {
        return 0;
    }
    else if (value1 == -1)
    {
        if (value2 == LONG_MIN)
        {
            throw OverflowException();
        }
    }
    else if (value1 == LONG_MIN)
    {
        if (value2 > 1)
        {
            throw UnderflowException();
        }
        else if (value2 == 1)
        {
            return LONG_MIN;
        }
        else if (value2 == 0)
        {
            return 0;
        }
        else
        {
            throw OverflowException();
        }
    }
    else // if (LONG_MIN < value1 && value1 < -1)
    {
        if (value2 > 1)
        {
            if ((unsigned long)(-value1) > ((unsigned long)LONG_MAX + 1UL) / (unsigned long)value2)
            {
                throw UnderflowException();
            }
        }
        else if (value2 == 1)
        {
            return value1;
        }
        else if (value2 == 0)
        {
            return 0;
        }
        else if (value2 == -1)
        {
            return -value1;
        }
        else if (value2 == LONG_MIN)
        {
            throw OverflowException();
        }
        else // if (LONG_MIN < value2 && value2 < -1)
        {
            if ((unsigned long)(-value1) > (unsigned long)LONG_MAX / (unsigned long)(-value2))
            {
                throw OverflowException();
            }
        }
    }
    long value = value1 * value2;
    return value;
}


//
// This helper function checks if the given floating point number is valid or not.
// If not, it throws an Exception accordingly.
//
static void validate(long double value)
{
    int c = fpclassify(value);
    if (c == FP_INFINITE)
    {
        throw OverflowException();
    }
    else if (c == FP_SUBNORMAL)
    {
        throw UnderflowException();
    }
    else if (c == FP_NAN)
    {
        throw EvaluationInabilityException();
    }
}


static void checkFpeCode(int code)
{
    switch (code)
    {
    case FPE_INTDIV: // integer divide by zero
    case FPE_FLTDIV: // floating-point divide by zero
        throw DivideByZeroException();
    case FPE_INTOVF: // integer overflow
    case FPE_FLTOVF: // floating-point overflow
        throw OverflowException();
    case FPE_FLTUND: // floating-point underflow
        throw UnderflowException();
    case FPE_FLTRES: // floating-point inexact result
        throw Exception(gettext("Floating-point inexact result"));
    case FPE_FLTINV: // floating-point invalid operation
        throw Exception(gettext("Floating-point invalid operation"));
    case FPE_FLTSUB: // subscript out of range
        throw Exception(gettext("Subscript out of range"));
    default:
        break;
    }
}


//////////////////////////////////////////////////////////////////////
//
// Parse
//
//////////////////////////////////////////////////////////////////////


//
// Parses the given portion of string and returns the resulting Expression data structure.
// The caller must free the returned data structure by delete operator when it is no longer needed.
// If this static method encounters an error, it throws InvalidExpressionException or
// InvalidCharExpression, the latter of which is thrown by Lexer class.
// See Parser.cc and Lexer.cc for details.
//
// s .......... pointer to the string
// n .......... length of the string in bytes
// complete ... true if the string is complete,
//              which means that the parser needs to strictly check
//              if the string represents a complete arithmetic expression.
// 
Expression* Expression::parse(const char *s, size_t n, bool complete)
{
    Parser parser(s, n, complete);
    return parser.run();
}


//////////////////////////////////////////////////////////////////////
//
// Add
//
//////////////////////////////////////////////////////////////////////


void AddExpression::format(std::vector<char> &buffer, int flags)
{
    left->format(buffer, flags);
    buffer.push_back('+');
    if (right)
    {
        right->format(buffer, flags);
    }
}


Expression* AddExpression::evaluate(bool permanent)
{
    Expression* expr1 = left->evaluate(permanent);
    if (!right)
    {
        return expr1;
    }
    Expression* expr2;
    try
    {
        expr2 = right->evaluate(permanent);
    }
    catch (...)
    {
        delete expr1;
        throw;
    }
    SigfpeHandler sigfpeHandler;
    sigfpeHandler.resetCode();
    if (sigsetjmp(SigfpeHandler::env, 1) == 0)
    {
        long value1 = 0, value2 = 0;
        if (getIntegers(expr1, expr2, value1, value2))
        {
            if (value2 > 0)
            {
                if (value1 > LONG_MAX - value2)
                {
                    throw OverflowException();
                }
            }
            else if (value2 == 0)
            {
                // ok
            }
            else if (value2 == LONG_MIN)
            {
                if (value1 < 0)
                {
                    throw UnderflowException();
                }
            }
            else // if (LONG_MIN < value2 && value2 < 0)
            {
                if (value1 < LONG_MIN - value2)
                {
                    throw UnderflowException();
                }
            }
            long value = value1 + value2;
            return new Integer(value);
        }
        else
        {
            long double value1 = 0, value2 = 0;
            if (getRealNumbers(expr1, expr2, value1, value2))
            {
                long double value = value1 + value2;
                validate(value);
                return new RealNumber(value);
            }
        }
    }
    checkFpeCode(sigfpeHandler.getCode());
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Subtract
//
//////////////////////////////////////////////////////////////////////


void SubtractExpression::format(std::vector<char> &buffer, int flags)
{
    left->format(buffer, flags);
    buffer.push_back('-');
    if (right)
    {
        right->format(buffer, flags);
    }
}


Expression* SubtractExpression::evaluate(bool permanent)
{
    Expression* expr1 = left->evaluate(permanent);
    if (!right)
    {
        return expr1;
    }
    Expression* expr2;
    try
    {
        expr2 = right->evaluate(permanent);
    }
    catch (...)
    {
        delete expr1;
        throw;
    }
    SigfpeHandler sigfpeHandler;
    sigfpeHandler.resetCode();
    if (sigsetjmp(SigfpeHandler::env, 1) == 0)
    {
        long value1 = 0, value2 = 0;
        if (getIntegers(expr1, expr2, value1, value2))
        {
            if (value2 > 0)
            {
                if (value1 < LONG_MIN + value2)
                {
                    throw UnderflowException();
                }
            }
            else if (value2 == 0)
            {
                // ok
            }
            else if (value2 == LONG_MIN)
            {
                if (value1 >= 0)
                {
                    throw UnderflowException();
                }
            }
            else // if (LONG_MIN < value2 && value2 < 0)
            {
                if (value1 > LONG_MAX + value2)
                {
                    throw OverflowException();
                }
            }
            long value = value1 - value2;
            return new Integer(value);
        }
        else
        {
            long double value1 = 0, value2 = 0;
            if (getRealNumbers(expr1, expr2, value1, value2))
            {
                long double value = value1 - value2;
                validate(value);
                return new RealNumber(value);
            }
        }
    }
    checkFpeCode(sigfpeHandler.getCode());
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Multiply
//
//////////////////////////////////////////////////////////////////////


void MultiplyExpression::format(std::vector<char> &buffer, int flags)
{
    left->format(buffer, flags);
    buffer.push_back('*');
    if (right)
    {
        right->format(buffer, flags);
    }
}


Expression* MultiplyExpression::evaluate(bool permanent)
{
    Expression* expr1 = left->evaluate(permanent);
    if (!right)
    {
        return expr1;
    }
    Expression* expr2;
    try
    {
        expr2 = right->evaluate(permanent);
    }
    catch (...)
    {
        delete expr1;
        throw;
    }
    SigfpeHandler sigfpeHandler;
    sigfpeHandler.resetCode();
    if (sigsetjmp(SigfpeHandler::env, 1) == 0)
    {
        long value1 = 0, value2 = 0;
        if (getIntegers(expr1, expr2, value1, value2))
        {
            long value = multiply(value1, value2);
            return new Integer(value);
        }
        else
        {
            long double value1 = 0, value2 = 0;
            if (getRealNumbers(expr1, expr2, value1, value2))
            {
                long double value = value1 * value2;
                validate(value);
                return new RealNumber(value);
            }
        }
    }
    checkFpeCode(sigfpeHandler.getCode());
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Divide
//
//////////////////////////////////////////////////////////////////////


void DivideExpression::format(std::vector<char> &buffer, int flags)
{
    left->format(buffer, flags);
    buffer.push_back('/');
    if (right)
    {
        right->format(buffer, flags);
    }
}


Expression* DivideExpression::evaluate(bool permanent)
{
    Expression* expr1 = left->evaluate(permanent);
    if (!right)
    {
        return expr1;
    }
    Expression* expr2;
    try
    {
        expr2 = right->evaluate(permanent);
    }
    catch (...)
    {
        delete expr1;
        throw;
    }
    SigfpeHandler sigfpeHandler;
    long ivalue1 = 0, ivalue2 = 0;
    long double rvalue1 = 0, rvalue2 = 0;
    if (getIntegers(expr1, expr2, ivalue1, ivalue2))
    {
        if (ivalue2 == 0)
        {
            throw DivideByZeroException();
        }
        sigfpeHandler.resetCode();
        if (sigsetjmp(SigfpeHandler::env, 1) == 0)
        {
            ldiv_t result = ldiv(ivalue1, ivalue2);
            if (result.rem == 0)
            {
                return new Integer(result.quot);
            }
        }
        else if (ivalue1 == LONG_MIN && ivalue2 == -1)
        {
            // this checking is needed as it is observed that
            // SIGFPE handler may receive FPE_INTDIV (division by zero) in this case
            throw OverflowException();
        }
        (void)sigfpeHandler.getCode(); // just block SIGFPE temporarily
        rvalue1 = ivalue1;
        rvalue2 = ivalue2;
    }
    else if (getRealNumbers(expr1, expr2, rvalue1, rvalue2))
    {
        if (rvalue2 == 0)
        {
            // Unless zero-check is done here, we will get FP_INFINITE.
            throw DivideByZeroException();
        }
    }
    else
    {
        throw EvaluationInabilityException();
    }
    sigfpeHandler.resetCode();
    if (sigsetjmp(SigfpeHandler::env, 1) == 0)
    {
        long double value = rvalue1 / rvalue2;
        validate(value);
        return new RealNumber(value);
    }
    checkFpeCode(sigfpeHandler.getCode());
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Unary Minus
//
//////////////////////////////////////////////////////////////////////


void MinusExpression::format(std::vector<char> &buffer, int flags)
{
    buffer.push_back('-');
    if (expr)
    {
        expr->format(buffer, flags);
    }
}


Expression* MinusExpression::evaluate(bool permanent)
{
    if (expr)
    {
        Expression* expr1 = expr->evaluate(permanent);
        if (expr1->getType() == ET_INTEGER)
        {
            long value1 = ((Integer*)expr1)->getValue();
            delete expr1;
            if (value1 == LONG_MIN)
            {
                throw OverflowException();
            }
            long value = -value1;
            return new Integer(value);
        }
        else if (expr1->getType() == ET_INTEGER_MAX_PLUS_ONE)
        {
            delete expr1;
            return new Integer(LONG_MIN);
        }
        else if (expr1->getType() == ET_REALNUMBER)
        {
            long double value1 = ((RealNumber*)expr1)->getValue();
            delete expr1;
            long double value = -value1;
            validate(value);
            return new RealNumber(value);
        }
        delete expr1;
    }
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Integer
//
//////////////////////////////////////////////////////////////////////


void Integer::format(std::vector<char> &buffer, int flags)
{
    if (string.empty())
    {
        char tmp[64];
        if ((flags & EF_HEXADECIMAL))
        {
            if (!(value & ~0xffffL))
            {
                sprintf(tmp, "0x%04lx", value);
            }
            else if (!(value & ~0xffffffffL))
            {
                sprintf(tmp, "0x%08lx", value);
            }
            else
            {
                sprintf(tmp, "0x%016lx", value);
            }
        }
        else if ((flags & EF_GROUPING))
        {
            sprintf(tmp, "%'ld", value);
        }
        else
        {
            sprintf(tmp, "%ld", value);
        }
        size_t n1 = buffer.size();
        size_t n2 = strlen(tmp);
        buffer.resize(n1 + n2);
        memcpy(&buffer[n1], tmp, n2);
    }
    else
    {
        size_t n2 = string.bytes();
        size_t n1 = buffer.size();
        buffer.resize(n1 + n2);
        memcpy(&buffer[n1], string.c_str(), n2);
    }
}


Expression* Integer::evaluate(bool permanent)
{
    return new Integer(*this);
}


//////////////////////////////////////////////////////////////////////
//
// Real Number
//
//////////////////////////////////////////////////////////////////////


void RealNumber::format(std::vector<char> &buffer, int flags)
{
    if (string.empty())
    {
        char tmp[64];
        if ((flags & (EF_PRECISION10 | EF_PRECISION20)))
        {
            int precision = ((flags & EF_PRECISION10) ? 10 : 0) + ((flags & EF_PRECISION20) ? 20 : 0);
            if ((flags & EF_GROUPING))
            {
                sprintf(tmp, "%'.*Lg", precision, value);
            }
            else
            {
                sprintf(tmp, "%.*Lg", precision, value);
            }
        }
        else if ((flags & EF_GROUPING))
        {
            sprintf(tmp, "%'Lg", value);
        }
        else
        {
            sprintf(tmp, "%Lg", value);
        }
        size_t n1 = buffer.size();
        size_t n2 = strlen(tmp);
        buffer.resize(n1 + n2);
        memcpy(&buffer[n1], tmp, n2);
    }
    else if ((flags & EF_PREPENDZERO) &&
             LocaleInfo::getDecimalPoint() == (int)string[0]) // not work as expected if [] is byte oriented and decimal point is not in US-ASCII
    {
        size_t n2 = string.bytes();
        size_t n1 = buffer.size();
        buffer.resize(n1 + 1 + n2);
        buffer[n1] = '0';
        memcpy(&buffer[n1 + 1], string.c_str(), n2);
    }
    else
    {
        size_t n2 = string.bytes();
        size_t n1 = buffer.size();
        buffer.resize(n1 + n2);
        memcpy(&buffer[n1], string.c_str(), n2);
    }
}


Expression* RealNumber::evaluate(bool permanent)
{
    validate(value);
    return new RealNumber(*this);
}


//////////////////////////////////////////////////////////////////////
//
// Block -- portion enclosed by parentheses
//
//////////////////////////////////////////////////////////////////////


void BlockExpression::format(std::vector<char> &buffer, int flags)
{
    buffer.push_back('(');
    if (expr)
    {
        expr->format(buffer, flags);
        if (type == ET_BLOCK)
        {
            buffer.push_back(')');
        }
    }
}


Expression* BlockExpression::evaluate(bool permanent)
{
    if (expr)
    {
        return expr->evaluate(permanent);
    }
    else
    {
        throw EvaluationInabilityException(gettext("Incomplete block"));
    }
}


//////////////////////////////////////////////////////////////////////
//
// Incomplete
//
//////////////////////////////////////////////////////////////////////


void IncompleteExpression::format(std::vector<char> &buffer, int flags)
{
    if (expr)
    {
        expr->format(buffer, flags);
    }
    size_t n2 = string.bytes();
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], string.c_str(), n2);
}


Expression* IncompleteExpression::evaluate(bool permanent)
{
    throw EvaluationInabilityException(gettext("Invalid operator"));
}


//////////////////////////////////////////////////////////////////////
//
// Variable
//
//////////////////////////////////////////////////////////////////////


void Variable::format(std::vector<char> &buffer, int flags)
{
    size_t n2 = key.bytes();
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], key.c_str(), n2);
}


Expression* Variable::evaluate(bool permanent)
{
    if (!VariableStore::instance().hasKey(key))
    {
        throw EvaluationInabilityException(Glib::ustring::compose(gettext("%1: Not exist"), key));
    }
    Glib::ustring value = VariableStore::instance().getValue(key);
    if (value.empty())
    {
        return new Integer(0);
    }
    else
    {
        Expression* expr1 = parse(value.c_str(), value.bytes(), true);
        try
        {
            VariableStore::instance().setInEvaluation(key);
            Expression* expr2 = expr1->evaluate(permanent);
            VariableStore::instance().unsetInEvaluation(key);
            delete expr1;
            return expr2;
        }
        catch (...)
        {
            VariableStore::instance().unsetInEvaluation(key);
            delete expr1;
            throw;
        }
    }
}


//////////////////////////////////////////////////////////////////////
//
// Assign
//
//////////////////////////////////////////////////////////////////////


void AssignExpression::format(std::vector<char> &buffer, int flags)
{
    size_t n2 = key.bytes();
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2 + 1);
    memcpy(&buffer[n1], key.c_str(), n2);
    buffer[n1 + n2] = '=';
    if (expr)
    {
        expr->format(buffer, flags);
    }
}


Expression* AssignExpression::evaluate(bool permanent)
{
    if (!VariableStore::instance().hasKey(key))
    {
        throw EvaluationInabilityException(Glib::ustring::compose(gettext("%1: Not exist"), key));
    }
    try
    {
        VariableStore::instance().setInEvaluation(key);
        Expression* expr2 = expr ? expr->evaluate(permanent) : new Integer(0);
        VariableStore::instance().unsetInEvaluation(key);
        if (permanent)
        {
            std::vector<char> buffer;
            expr->format(buffer, false);
            buffer.push_back(0);
            VariableStore::instance().setValue(key, &buffer[0]);
        }
        return expr2;
    }
    catch (...)
    {
        VariableStore::instance().unsetInEvaluation(key);
        throw;
    }
}


//////////////////////////////////////////////////////////////////////
//
// Absolute Value
//
//////////////////////////////////////////////////////////////////////


void AbsExpression::format(std::vector<char> &buffer, int flags)
{
    const char *op = OperatorInfo::instance().find(SYM_ABS);
    size_t n2 = strlen(op);
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], op, n2);
    if (expr)
    {
        expr->format(buffer, flags);
    }
}


Expression* AbsExpression::evaluate(bool permanent)
{
    if (expr)
    {
        Expression* expr1 = expr->evaluate(permanent);
        if (expr1->getType() == ET_INTEGER)
        {
            long value = ((Integer*)expr1)->getValue();
            delete expr1;
            if (value == LONG_MIN)
            {
                throw OverflowException();
            }
            if (value < 0)
            {
                value *= -1;
            }
            return new Integer(value);
        }
        else if (expr1->getType() == ET_REALNUMBER)
        {
            long double value1 = ((RealNumber*)expr1)->getValue();
            delete expr1;
            long double value = fabsl(value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_INTEGER_MAX_PLUS_ONE)
        {
            delete expr1;
            throw OverflowException();
        }
        delete expr1;
    }
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Cube Root
//
//////////////////////////////////////////////////////////////////////


void CbrtExpression::format(std::vector<char> &buffer, int flags)
{
    const char *op = OperatorInfo::instance().find(SYM_CBRT);
    size_t n2 = strlen(op);
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], op, n2);
    if (expr)
    {
        expr->format(buffer, flags);
    }
}


Expression* CbrtExpression::evaluate(bool permanent)
{
    if (expr)
    {
        Expression* expr1 = expr->evaluate(permanent);
        if (expr1->getType() == ET_INTEGER)
        {
            long value1 = ((Integer*)expr1)->getValue();
            delete expr1;
            long double value = cbrtl((long double)value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_REALNUMBER)
        {
            long double value1 = ((RealNumber*)expr1)->getValue();
            delete expr1;
            long double value = cbrtl(value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_INTEGER_MAX_PLUS_ONE)
        {
            delete expr1;
            throw OverflowException();
        }
        delete expr1;
    }
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Cosine
//
//////////////////////////////////////////////////////////////////////


void CosExpression::format(std::vector<char> &buffer, int flags)
{
    const char *op = OperatorInfo::instance().find(SYM_COS);
    size_t n2 = strlen(op);
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], op, n2);
    if (expr)
    {
        expr->format(buffer, flags);
    }
}


Expression* CosExpression::evaluate(bool permanent)
{
    if (expr)
    {
        Expression* expr1 = expr->evaluate(permanent);
        if (expr1->getType() == ET_INTEGER)
        {
            long value1 = ((Integer*)expr1)->getValue();
            delete expr1;
            long double value = cosl((long double)value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_REALNUMBER)
        {
            long double value1 = ((RealNumber*)expr1)->getValue();
            delete expr1;
            long double value = cosl(value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_INTEGER_MAX_PLUS_ONE)
        {
            delete expr1;
            throw OverflowException();
        }
        delete expr1;
    }
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Base-e Exponential
//
//////////////////////////////////////////////////////////////////////


void ExpExpression::format(std::vector<char> &buffer, int flags)
{
    const char *op = OperatorInfo::instance().find(SYM_EXP);
    size_t n2 = strlen(op);
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], op, n2);
    if (expr)
    {
        expr->format(buffer, flags);
    }
}


Expression* ExpExpression::evaluate(bool permanent)
{
    if (expr)
    {
        Expression* expr1 = expr->evaluate(permanent);
        if (expr1->getType() == ET_INTEGER)
        {
            long value1 = ((Integer*)expr1)->getValue();
            delete expr1;
            long double value = expl((long double)value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_REALNUMBER)
        {
            long double value1 = ((RealNumber*)expr1)->getValue();
            delete expr1;
            long double value = expl(value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_INTEGER_MAX_PLUS_ONE)
        {
            delete expr1;
            throw OverflowException();
        }
        delete expr1;
    }
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Euclidean Distance
//
//////////////////////////////////////////////////////////////////////


void HypotExpression::format(std::vector<char> &buffer, int flags)
{
    left->format(buffer, flags);
    const char *op = OperatorInfo::instance().find(SYM_HYPOT);
    size_t n2 = strlen(op);
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], op, n2);
    if (right)
    {
        right->format(buffer, flags);
    }
}


Expression* HypotExpression::evaluate(bool permanent)
{
    Expression* expr1 = left->evaluate(permanent);
    if (!right)
    {
        return expr1;
    }
    Expression* expr2;
    try
    {
        expr2 = right->evaluate(permanent);
    }
    catch (...)
    {
        delete expr1;
        throw;
    }
    long double value1 = 0, value2 = 0;
    if (getRealNumbers(expr1, expr2, value1, value2))
    {
        long double value = hypotl(value1, value2);
        validate(value);
        return new RealNumber(value);
    }
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Log
//
//////////////////////////////////////////////////////////////////////


void LogExpression::format(std::vector<char> &buffer, int flags)
{
    const char *op = OperatorInfo::instance().find(SYM_LOG);
    size_t n2 = strlen(op);
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], op, n2);
    if (expr)
    {
        expr->format(buffer, flags);
    }
}


Expression* LogExpression::evaluate(bool permanent)
{
    if (expr)
    {
        Expression* expr1 = expr->evaluate(permanent);
        if (expr1->getType() == ET_INTEGER)
        {
            long value1 = ((Integer*)expr1)->getValue();
            delete expr1;
            long double value = logl((long double)value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_REALNUMBER)
        {
            long double value1 = ((RealNumber*)expr1)->getValue();
            delete expr1;
            long double value = logl(value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_INTEGER_MAX_PLUS_ONE)
        {
            delete expr1;
            throw OverflowException();
        }
        delete expr1;
    }
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Log2
//
//////////////////////////////////////////////////////////////////////


void Log2Expression::format(std::vector<char> &buffer, int flags)
{
    const char *op = OperatorInfo::instance().find(SYM_LOG2);
    size_t n2 = strlen(op);
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], op, n2);
    if (expr)
    {
        expr->format(buffer, flags);
    }
}


Expression* Log2Expression::evaluate(bool permanent)
{
    if (expr)
    {
        Expression* expr1 = expr->evaluate(permanent);
        if (expr1->getType() == ET_INTEGER)
        {
            long value1 = ((Integer*)expr1)->getValue();
            delete expr1;
            long double value = log2l((long double)value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_REALNUMBER)
        {
            long double value1 = ((RealNumber*)expr1)->getValue();
            delete expr1;
            long double value = log2l(value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_INTEGER_MAX_PLUS_ONE)
        {
            delete expr1;
            throw OverflowException();
        }
        delete expr1;
    }
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Log10
//
//////////////////////////////////////////////////////////////////////


void Log10Expression::format(std::vector<char> &buffer, int flags)
{
    const char *op = OperatorInfo::instance().find(SYM_LOG10);
    size_t n2 = strlen(op);
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], op, n2);
    if (expr)
    {
        expr->format(buffer, flags);
    }
}


Expression* Log10Expression::evaluate(bool permanent)
{
    if (expr)
    {
        Expression* expr1 = expr->evaluate(permanent);
        if (expr1->getType() == ET_INTEGER)
        {
            long value1 = ((Integer*)expr1)->getValue();
            delete expr1;
            long double value = log10l((long double)value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_REALNUMBER)
        {
            long double value1 = ((RealNumber*)expr1)->getValue();
            delete expr1;
            long double value = log10l(value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_INTEGER_MAX_PLUS_ONE)
        {
            delete expr1;
            throw OverflowException();
        }
        delete expr1;
    }
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Power
//
//////////////////////////////////////////////////////////////////////


void PowExpression::format(std::vector<char> &buffer, int flags)
{
    left->format(buffer, flags);
    const char *op = OperatorInfo::instance().find(SYM_POW);
    size_t n2 = strlen(op);
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], op, n2);
    if (right)
    {
        right->format(buffer, flags);
    }
}


Expression* PowExpression::evaluate(bool permanent)
{
    Expression* expr1 = left->evaluate(permanent);
    if (!right)
    {
        return expr1;
    }
    Expression* expr2;
    try
    {
        expr2 = right->evaluate(permanent);
    }
    catch (...)
    {
        delete expr1;
        throw;
    }
    SigfpeHandler sigfpeHandler;
    sigfpeHandler.resetCode();
    if (sigsetjmp(SigfpeHandler::env, 1) == 0)
    {
        long value1 = 0, value2 = 0;
        if (getIntegers(expr1, expr2, value1, value2))
        {
            if (value2 >= 1)
            {
                if (value1 == 0)
                {
                    return new Integer(0);
                }
                else if (value1 == 1)
                {
                    return new Integer(1);
                }
                else if (value1 == -1)
                {
                    return new Integer((value2 & 1) ? -1 : 1);
                }
                else
                {
                    long value = 1;
                    while (value2)
                    {
                        value = multiply(value, value1);
                        value2--;
                    }
                    return new Integer(value);
                }
            }
            else if (value2 == 0)
            {
                return new Integer(1);
            }
            else
            {
                long double value = powl((long double)value1, (long double)value2);
                validate(value);
                return new RealNumber(value);
            }
        }
        else
        {
            long double value1 = 0, value2 = 0;
            if (getRealNumbers(expr1, expr2, value1, value2))
            {
                long double value = powl(value1, value2);
                validate(value);
                return new RealNumber(value);
            }
        }
    }
    checkFpeCode(sigfpeHandler.getCode());
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Sine
//
//////////////////////////////////////////////////////////////////////


void SinExpression::format(std::vector<char> &buffer, int flags)
{
    const char *op = OperatorInfo::instance().find(SYM_SIN);
    size_t n2 = strlen(op);
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], op, n2);
    if (expr)
    {
        expr->format(buffer, flags);
    }
}


Expression* SinExpression::evaluate(bool permanent)
{
    if (expr)
    {
        Expression* expr1 = expr->evaluate(permanent);
        if (expr1->getType() == ET_INTEGER)
        {
            long value1 = ((Integer*)expr1)->getValue();
            delete expr1;
            long double value = sinl((long double)value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_REALNUMBER)
        {
            long double value1 = ((RealNumber*)expr1)->getValue();
            delete expr1;
            long double value = sinl(value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_INTEGER_MAX_PLUS_ONE)
        {
            delete expr1;
            throw OverflowException();
        }
        delete expr1;
    }
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Square Root
//
//////////////////////////////////////////////////////////////////////


void SqrtExpression::format(std::vector<char> &buffer, int flags)
{
    const char *op = OperatorInfo::instance().find(SYM_SQRT);
    size_t n2 = strlen(op);
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], op, n2);
    if (expr)
    {
        expr->format(buffer, flags);
    }
}


Expression* SqrtExpression::evaluate(bool permanent)
{
    if (expr)
    {
        Expression* expr1 = expr->evaluate(permanent);
        if (expr1->getType() == ET_INTEGER)
        {
            long value1 = ((Integer*)expr1)->getValue();
            delete expr1;
            long double value = sqrtl((long double)value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_REALNUMBER)
        {
            long double value1 = ((RealNumber*)expr1)->getValue();
            delete expr1;
            long double value = sqrtl(value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_INTEGER_MAX_PLUS_ONE)
        {
            delete expr1;
            throw OverflowException();
        }
        delete expr1;
    }
    throw EvaluationInabilityException();
}


//////////////////////////////////////////////////////////////////////
//
// Tangent
//
//////////////////////////////////////////////////////////////////////


void TanExpression::format(std::vector<char> &buffer, int flags)
{
    const char *op = OperatorInfo::instance().find(SYM_TAN);
    size_t n2 = strlen(op);
    size_t n1 = buffer.size();
    buffer.resize(n1 + n2);
    memcpy(&buffer[n1], op, n2);
    if (expr)
    {
        expr->format(buffer, flags);
    }
}


Expression* TanExpression::evaluate(bool permanent)
{
    if (expr)
    {
        Expression* expr1 = expr->evaluate(permanent);
        if (expr1->getType() == ET_INTEGER)
        {
            long value1 = ((Integer*)expr1)->getValue();
            delete expr1;
            long double value = tanl((long double)value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_REALNUMBER)
        {
            long double value1 = ((RealNumber*)expr1)->getValue();
            delete expr1;
            long double value = tanl(value1);
            validate(value);
            return new RealNumber(value);
        }
        else if (expr1->getType() == ET_INTEGER_MAX_PLUS_ONE)
        {
            delete expr1;
            throw OverflowException();
        }
        delete expr1;
    }
    throw EvaluationInabilityException();
}
