// Copyright (C) 2014-2017 Hideaki Narita


#include <libintl.h>
#include "Parser.h"
#include "Exception.h"
#include "VariableStore.h"


using namespace hnrt;


//
// s .......... pointer to string to parse
// n .......... length of string in bytes
// complete ... true if the string is complete,
//              which means the parser needs to strictly check
//              if the string represents a complete arithmetic expression.
//
Parser::Parser(const char* s, size_t n, bool complete_)
    : lexer(s, n)
    , complete(complete_)
    , sym(0)
{
    sym = lexer.getSym();
}


// unused...just to make the compiler happy.
Parser::Parser(const Parser& other)
    : lexer(NULL, 0)
    , complete(false)
    , sym(0)
{
}


//
// Parses the string given to the constructor
// and returns a pointer to the resulting Expression data structure.
// If it encounters an error, it throws InvalidExpressionException or
// InvalidCharExpression, the latter of which is thrown by Lexer class.
//
Expression* Parser::run()
{
    Expression* expr = parseExpr1();
    if (sym == SYM_EOF)
    {
        return expr;
    }
    else
    {
        delete expr;
        throw InvalidExpressionException(gettext("Invalid syntax."));
    }
}


//
// Parses variable = expression.
//
Expression* Parser::parseExpr1()
{
    Expression* expr = parseExpr2();
    try
    {
        if (sym == SYM_ASSIGN)
        {
            if (expr->getType() == ET_VARIABLE)
            {
                Glib::ustring key = ((Variable *)expr)->getKey();
                if (!VariableStore::instance().hasKey(key))
                {
                    throw InvalidExpressionException(Glib::ustring::compose(gettext("%1: Not exist"), key));
                }
                else if (key.length() > 1)
                {
                    // Only variable A to Z are allowed to be changed; others are treated as read-only.
                    throw InvalidExpressionException(Glib::ustring::compose(gettext("%1: Read only"), key));
                }
                sym = lexer.getSym();
                delete expr;
                expr = NULL;
                expr = new AssignExpression(key, parseExpr1());
            }
            else
            {
                throw InvalidExpressionException(gettext("Non variable cannot be assigned expression"));
            }
        }
        return expr;
    }
    catch (...)
    {
        delete expr;
        throw;
    }
}


//
// Parses expression {+|-} expression.
//
Expression* Parser::parseExpr2()
{
    Expression* expr = parseExpr3();
    try
    {
        while (1)
        {
            switch (sym)
            {
            case SYM_PLUS:
                sym = lexer.getSym();
                expr = new AddExpression(expr, parseExpr3());
                break;
            case SYM_MINUS:
                sym = lexer.getSym();
                expr = new SubtractExpression(expr, parseExpr3());
                break;
            default:
                return expr;
            }
        }
    }
    catch (...)
    {
        delete expr;
        throw;
    }
}


//
// Parses expression {*|/} expression.
//
Expression* Parser::parseExpr3()
{
    Expression* expr = parseExpr4();
    try
    {
        while (1)
        {
            switch (sym)
            {
            case SYM_MULTIPLY:
                sym = lexer.getSym();
                expr = new MultiplyExpression(expr, parseExpr4());
                break;
            case SYM_DIVIDE:
                sym = lexer.getSym();
                expr = new DivideExpression(expr, parseExpr4());
                break;
            default:
                return expr;
            }
        }
    }
    catch (...)
    {
        delete expr;
        throw;
    }
}


//
// Parses custom binary operator expression
//
Expression* Parser::parseExpr4()
{
    Expression* expr = parseExpr5();
    try
    {
        while (1)
        {
            switch (sym)
            {
            case SYM_HYPOT:
                sym = lexer.getSym();
                expr = new HypotExpression(expr, parseExpr5());
                break;
            case SYM_POW:
                sym = lexer.getSym();
                expr = new PowExpression(expr, parseExpr5());
                break;
            case SYM_INCOMPLETE_OPERATOR:
                if (!complete)
                {
                    expr = new IncompleteExpression(expr, lexer.getString());
                    sym = lexer.getSym();
                    break;
                }
                //FALLTHROUGH
            default:
                return expr;
            }
        }
    }
    catch (...)
    {
        delete expr;
        throw;
    }
}


//
// Parses a number, a unary operator, or a block enclosed by a pair of parentheses.
//
Expression* Parser::parseExpr5()
{
    Expression* expr = NULL;
    try
    {
        switch (sym)
        {
        case SYM_INTEGER:
            expr = new Integer(lexer.getInteger(), lexer.getString());
            sym = lexer.getSym();
            break;
        case SYM_REALNUMBER:
            expr = new RealNumber(lexer.getRealNumber(), lexer.getString());
            sym = lexer.getSym();
            break;
        case SYM_LPAREN:
            sym = lexer.getSym();
            expr = new BlockExpression(parseExpr1());
            if (sym == SYM_RPAREN)
            {
                sym = lexer.getSym();
            }
            else if (complete || sym != SYM_EOF)
            {
                throw InvalidExpressionException(gettext("Right parenthesis is missing."));
            }
            else
            {
                ((BlockExpression*)expr)->setIncomplete();
            }
            break;
        case SYM_MINUS:
            sym = lexer.getSym();
            expr = new MinusExpression(parseExpr5());
            break;
        case SYM_IDENTIFIER:
            expr = new Variable(lexer.getString());
            if (complete && !VariableStore::instance().hasKey(((Variable*)expr)->getKey()))
            {
                throw InvalidExpressionException(Glib::ustring::compose(gettext("%1: Not exist"), ((Variable*)expr)->getKey()));
            }
            sym = lexer.getSym();
            break;
        case SYM_ABS:
            sym = lexer.getSym();
            expr = new AbsExpression(parseExpr5());
            break;
        case SYM_CBRT:
            sym = lexer.getSym();
            expr = new CbrtExpression(parseExpr5());
            break;
        case SYM_COS:
            sym = lexer.getSym();
            expr = new CosExpression(parseExpr5());
            break;
        case SYM_EXP:
            sym = lexer.getSym();
            expr = new ExpExpression(parseExpr5());
            break;
        case SYM_LOG:
            sym = lexer.getSym();
            expr = new LogExpression(parseExpr5());
            break;
        case SYM_LOG2:
            sym = lexer.getSym();
            expr = new Log2Expression(parseExpr5());
            break;
        case SYM_LOG10:
            sym = lexer.getSym();
            expr = new Log10Expression(parseExpr5());
            break;
        case SYM_SIN:
            sym = lexer.getSym();
            expr = new SinExpression(parseExpr5());
            break;
        case SYM_SQRT:
            sym = lexer.getSym();
            expr = new SqrtExpression(parseExpr5());
            break;
        case SYM_TAN:
            sym = lexer.getSym();
            expr = new TanExpression(parseExpr5());
            break;
        default:
            if (complete)
            {
                throw InvalidExpressionException(gettext("Invalid syntax."));
            }
            switch (sym)
            {
            case SYM_EOF:
                // This is allowed if complete is false, which means that input is not yet finished.
                break;
            case SYM_INCOMPLETE_OPERATOR:
                expr = new IncompleteExpression(lexer.getString());
                sym = lexer.getSym();
                break;
            default:
                throw InvalidExpressionException(gettext("Invalid syntax."));
            }
            break;
        }
        return expr;
    }
    catch (...)
    {
        if (expr)
        {
            delete expr;
        }
        throw;
    }
}
