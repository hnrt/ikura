// Copyright (C) 2014-2017 Hideaki Narita


#include <libintl.h>
#include <string.h>
#include "InputBuffer.h"
#include "Exception.h"
#include "Expression.h"
#include "Lexer.h"
#include "LocaleInfo.h"
#include "UTF8.h"


using namespace hnrt;


static const char arithmeticOperators[] = "+-*/";


#define SUPER std::vector<char>


InputBuffer::InputBuffer()
    : validSize(0)
    , justEvaluated(false)
    , formatFlags(EF_GROUPING)
{
}


InputBuffer::~InputBuffer()
{
}


bool InputBuffer::getGrouping() const
{
    return (formatFlags & EF_GROUPING) ? true : false;
}


void InputBuffer::setGrouping(bool value)
{
    if (getGrouping() != value)
    {
        formatFlags ^= EF_GROUPING;
    }
}


bool InputBuffer::getHexadecimal() const
{
    return (formatFlags & EF_HEXADECIMAL) ? true : false;
}


void InputBuffer::setHexadecimal(bool value)
{
    if (getHexadecimal() != value)
    {
        formatFlags ^= EF_HEXADECIMAL;
    }
}


int InputBuffer::getPrecision() const
{
    int precision = 0;
    if ((formatFlags & EF_PRECISION10))
    {
        precision += 10;
    }
    if ((formatFlags & EF_PRECISION20))
    {
        precision += 20;
    }
    return precision;
}


void InputBuffer::setPrecision(int value)
{
    if (getPrecision() != value)
    {
        if (value < 10)
        {
            formatFlags &= ~(EF_PRECISION10 | EF_PRECISION20);
        }
        else if (value < 20)
        {
            formatFlags &= ~EF_PRECISION20;
            formatFlags |= EF_PRECISION10;
        }
        else if (value < 30)
        {
            formatFlags &= ~EF_PRECISION10;
            formatFlags |= EF_PRECISION20;
        }
        else
        {
            formatFlags |= EF_PRECISION10 | EF_PRECISION20;
        }
    }
}


void InputBuffer::clear()
{
    SUPER::clear();
    validSize = 0;
    justEvaluated = false;
    sigTextChange.emit("0");
    sigTooltipChange.emit(gettext("Please enter expression"));
    sigClear.emit();
}


void InputBuffer::assign(const char *s)
{
    size_t lastSize = SUPER::size();
    if (lastSize)
    {
        SUPER::clear();
    }
    validSize = 0;
    justEvaluated = false;
    putString(s);
    if (!SUPER::size() && lastSize)
    {
        sigTextChange.emit("0");
        sigTooltipChange.emit(gettext("Please enter expression"));
        sigClear.emit();
    }
}


void InputBuffer::putChar(int c)
{
    if (!(c & ~0x7F) && strchr(arithmeticOperators, c))
    {
        if (validSize && !(at(validSize - 1) & ~0x7F) && strchr(arithmeticOperators, at(validSize - 1)))
        {
            validSize--;
            at(validSize) = c;
        }
        else
        {
            push_back(c);
        }
    }
    else if (justEvaluated)
    {
        if (LocaleInfo::getDecimalPoint() == c)
        {
            clear();
            UTF8::pushBack(*this, c);
        }
        else if (!(c & ~0x7F) && isalnum(c))
        {
            clear();
            push_back((char)c);
        }
        else
        {
            UTF8::pushBack(*this, c);
        }
    }
    else
    {
        UTF8::pushBack(*this, c);
    }
    parse();
}


void InputBuffer::putString(const char* s)
{
    size_t lastSize = SUPER::size();
    try
    {
        Lexer lexer(s, strlen(s));
        int sym = lexer.getSym();
        while (sym != SYM_EOF)
        {
            const char* t = lexer.getString();
            size_t n = strlen(t);
            size_t curSize = lastSize + n;
            resize(curSize);
            memcpy(&SUPER::at(lastSize), t, n);
            try
            {
                Expression *expr = Expression::parse(*this, curSize);
                delete expr;
            }
            catch (...)
            {
                resize(lastSize);
                break;
            }
            lastSize = curSize;
            sym = lexer.getSym();
        }
    }
    catch (...)
    {
    }
    if (validSize != lastSize)
    {
        parse();
    }
}


void InputBuffer::parse()
{
    Glib::ustring tooltip;
    try
    {
        justEvaluated = false;
        Expression *expr = Expression::parse(*this, SUPER::size());
        bool first = !validSize;
        SUPER::clear();
        expr->format(*this, 0);
        validSize = SUPER::size();
        std::vector<char> buffer;
        expr->format(buffer, formatFlags | EF_PREPENDZERO);
        buffer.push_back('\0');
        sigTextChange.emit(&buffer[0]);
        try
        {
            Expression *expr2 = expr->evaluate(false);
            buffer.clear();
            expr2->format(buffer, formatFlags);
            buffer.push_back('\0');
            sigTooltipChange.emit(&buffer[0]);
            delete expr2;
        }
        catch (const Exception& ex)
        {
            sigTooltipChange.emit(ex.getWhat().c_str());
        }
        if (first)
        {
            sigFirstChar.emit();
        }
        delete expr;
    }
    catch (const OverflowException& ex)
    {
        sigOverflow.emit();
        resize(validSize);
    }
    catch (const UnderflowException& ex)
    {
        sigUnderflow.emit();
        resize(validSize);
    }
    catch (const Exception& ex)
    {
        sigInvalidChar.emit();
        resize(validSize);
    }
}


void InputBuffer::evaluate()
{
    try
    {
        size_t n = SUPER::size();
        // delete the operator at the end of input if it is there
        if (n && strchr(arithmeticOperators, at(n - 1)))
        {
            n--;
            resize(n);
        }
        // complement the right parentheses if necessary
        if (n)
        {
            int nL = UTF8::count(*this, n, '(');
            int nR = UTF8::count(*this, n, ')');
            while (nL > nR)
            {
                push_back(')');
                nR++;
            }
        }
        Expression *expr1 = Expression::parse(*this, SUPER::size(), true);
        try
        {
            if (expr1->getType() == ET_INTEGER_MAX_PLUS_ONE)
            {
                throw OverflowException();
            }
            Expression *expr2 = expr1->evaluate(true);
            std::vector<char> buffer2;
            expr2->format(buffer2, formatFlags);
            buffer2.push_back('\0');
            sigTextChange.emit(&buffer2[0]);
            std::vector<char> buffer;
            expr1->format(buffer, EF_PREPENDZERO);
            buffer.push_back('\0');
            sigTooltipChange.emit(&buffer[0]);
            SUPER::clear();
            expr2->format(*this, formatFlags & ~EF_GROUPING);
            validSize = SUPER::size();
            justEvaluated = true;
            buffer2 = *this;
            buffer2.push_back('\0');
            sigEvaluated.emit(&buffer[0], &buffer2[0]);
            delete expr2;
        }
        catch (const DivideByZeroException& ex)
        {
            sigDivisionByZero.emit();
        }
        catch (const OverflowException& ex)
        {
            sigOverflow.emit();
        }
        catch (const UnderflowException& ex)
        {
            sigUnderflow.emit();
        }
        catch (const EvaluationInabilityException& ex)
        {
            sigEvaluationInability.emit();
        }
        catch (const RecursiveVariableAccessException& ex)
        {
            sigRecursiveVariableAccess.emit(ex.getKey().c_str());
        }
        catch (...)
        {
            g_printerr("BUG@%s(%d)\n", __FILE__, __LINE__);
        }
        delete expr1;
    }
    catch (...)
    {
        sigIncompleteExpression.emit();
    }
}


void InputBuffer::deleteLastChar()
{
    if (SUPER::size())
    {
        validSize = 0;
        Lexer lexer(&SUPER::at(0), SUPER::size());
        int sym, lastSym;
        size_t length;
        try
        {
            sym = lexer.getSym();
        }
        catch (...)
        {
            goto done;
        }
        if (sym == SYM_EOF)
        {
            // no chance, but just in case
            goto done;
        }
        while (1)
        {
            lastSym = sym;
            length = strlen(lexer.getString());
            try
            {
                sym = lexer.getSym();
            }
            catch (...)
            {
                break;
            }
            if (sym == SYM_EOF)
            {
                break;
            }
            validSize += length;
        }
        if (lastSym == SYM_INTEGER ||
            lastSym == SYM_REALNUMBER ||
            lastSym == SYM_INCOMPLETE_OPERATOR)
        {
            validSize += length - 1;
        }
    done:
        if (validSize)
        {
            resize(validSize);
            parse();
        }
        else
        {
            clear();
        }
    }
}
