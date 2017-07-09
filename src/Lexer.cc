// Copyright (C) 2014-2017 Hideaki Narita


#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include "Lexer.h"
#include "Exception.h"
#include "OperatorInfo.h"
#include "VariableStore.h"
#include "LocaleInfo.h"
#include "UTF8.h"


// It is observed that some of isw* functions behave unexpectedly.
// As such, I decided to use is* functions, instead.
// To do that, I need to check if the value is of US ASCII first.
// But isascii assumes that the given value is of unsigned char or EOF.
// This means that the value greater than 0xFF is not supported.
// That is the reason why this macro is necessary...
#define IS_USASCII(c) (!((c)&~0x7F))


using namespace hnrt;


//
// Input:
//
//   s ... pointer to string to parse
//   n ... length of string in bytes
//
Lexer::Lexer(const char *s, size_t n)
    : next(s)
    , stop(s + n)
    , c(0)
    , v()
    , buf()
{
    c = getChar();
}


//
// Returns the next character.
// If it reaches the end of string, it returns zero.
//
int Lexer::getChar()
{
    if (next < stop)
    {
        int c = UTF8::getChar(next, stop, &next);
        if (c < 0)
        {
            throw InvalidCharException();
        }
        return c;
    }
    else
    {
        return 0;
    }
}


//
// Returns the next terminal symbol.
// If it reaches the end of string, it returns SYM_EOF.
// If it encounters an error, it throws InvalidCharException
// or OverflowException.
//
int Lexer::getSym()
{
    int sym;
    buf.clear();
    if (c == 0)
    {
        sym = SYM_EOF;
    }
    else if (IS_USASCII(c) && isdigit(c))
    {
        buf.push_back(c);
        c = getChar();
        if (buf[0] == '0' && parseHexadecimal())
        {
            sym = SYM_INTEGER;
            buf.push_back('\0');
            if (&buf[2]) // check the character next to "0x"
            {
                errno = 0;
                unsigned long value = strtoul(&buf[2], NULL, 16);
                if (errno == ERANGE)
                {
                    throw OverflowException();
                }
                v.integer = (long)value;
            }
            else
            {
                v.integer = 0;
            }
            goto done;
        }
        while (IS_USASCII(c) && isdigit(c))
        {
            buf.push_back(c);
            c = getChar();
        }
        if (parseDecimalFractionPart() || parseExponentPart())
        {
            sym = SYM_REALNUMBER;
            buf.push_back('\0');
            errno = 0;
            v.realNumber = strtold(&buf[0], NULL);
            if (errno == ERANGE)
            {
                if (v.realNumber == HUGE_VALL)
                {
                    throw OverflowException();
                }
                else
                {
                    throw UnderflowException();
                }
            }
        }
        else
        {
            sym = SYM_INTEGER;
            buf.push_back('\0');
            errno = 0;
            unsigned long value = strtoul(&buf[0], NULL, 10);
            if (errno == ERANGE || value > 9223372036854775808UL)
            {
                throw OverflowException();
            }
            // note: 9223372036854775808UL must with a minus sign. Otherwise, take it as a overflow case.
            v.integer = (long)value;
        }
    }
    else if (parseDecimalFractionPart())
    {
        sym = SYM_REALNUMBER;
        buf.push_back('\0');
        errno = 0;
        v.realNumber = strtold(&buf[0], NULL);
        if (errno == ERANGE)
        {
            if (v.realNumber == HUGE_VALL)
            {
                throw OverflowException();
            }
            else
            {
                throw UnderflowException();
            }
        }
    }
    else if (c == '+' || c == '-' || c == '*' || c == '/')
    {
        sym = c;
        buf.push_back(c);
        c = getChar();
        buf.push_back('\0');
        if (c == '+' || c == '-' || c == '*' || c == '/')
        {
            throw InvalidCharException();
        }
    }
    else if (c == '(' || c == ')')
    {
        sym = c;
        buf.push_back(c);
        c = getChar();
        buf.push_back('\0');
    }
    else if (c == '{')
    {
        buf.push_back(c);
        c = getChar();
        while (c != '}')
        {
            if (c == 0)
            {
                if (buf.size() > 1)
                {
                    OperatorInfo::instance().Complement(buf);
                    buf.push_back('\0');
                    sym = OperatorInfo::instance().find(&buf[0]);
                    if (sym == SYM_ERROR)
                    {
                        sym = SYM_INCOMPLETE_OPERATOR;
                    }
                }
                else
                {
                    buf.push_back('\0');
                    sym = SYM_INCOMPLETE_OPERATOR;
                }
                goto done;
            }
            if (!IS_USASCII(c))
            {
                throw InvalidCharException();
            }
            else if (isalpha(c))
            {
                c = tolower(c);
            }
            else if (isdigit(c))
            {
                // ok to continue
            }
            else
            {
                throw InvalidCharException();
            }
            buf.push_back(c);
            c = getChar();
        }
        buf.push_back(c);
        c = getChar();
        buf.push_back('\0');
        sym = OperatorInfo::instance().find(&buf[0]);
    }
    else if ((IS_USASCII(c) && isalpha(c)) || c == '_' || c == '$' || c == '@')
    {
        buf.push_back(c);
        c = getChar();
        while ((IS_USASCII(c) && isalnum(c)) || c == '_' || c == '$' || c == '@')
        {
            if (isalpha(c))
            {
                c = toupper(c);
            }
            buf.push_back(c);
            c = getChar();
        }
        if (c == 0 && buf.size() > 1)
        {
            VariableStore::instance().Complement(buf);
        }
        sym = SYM_IDENTIFIER;
        buf.push_back('\0');
    }
    else if (c == '=')
    {
        buf.push_back(c);
        c = getChar();
        sym = SYM_ASSIGN;
        buf.push_back('\0');
    }
    else
    {
        throw InvalidCharException();
    }
done:
    return sym;
}


//
// Tries to parse the decimal fraction part of a real number
// and returns true if successful, false if it does nothing.
// If encounters an error, it throws InvalidCharException.
//
bool Lexer::parseDecimalFractionPart()
{
    if (c == LocaleInfo::getDecimalPoint())
    {
        UTF8::pushBack(buf, c);
        c = getChar();
        while (IS_USASCII(c) && isdigit(c))
        {
            buf.push_back(c);
            c = getChar();
        }
        parseExponentPart();
        return true;
    }
    return false;
}


//
// Tries to parse the exponent part of a real number
// and returns true if successful, false if it does nothing.
// If encounters an error, it throws InvalidCharException.
//
bool Lexer::parseExponentPart()
{
    if (c == L'E' || c == L'e' || c == SYM_E)
    {
        buf.push_back('e');
        c = getChar();
        if (c == L'+' || c == L'-')
        {
            buf.push_back(c);
            c = getChar();
        }
        if (IS_USASCII(c) && isdigit(c))
        {
            do
            {
                buf.push_back(c);
                c = getChar();
            }
            while (IS_USASCII(c) && isdigit(c));
        }
        else if (c)
        {
            throw InvalidCharException();
        }
        return true;
    }
    return false;
}


//
// Tries to parse a hexadecimal integer which begins with X or x.
// and returns true if successful, false if it does nothing.
// If encounters an error, it throws InvalidCharException.
//
bool Lexer::parseHexadecimal()
{
    if (c == L'X' || c == L'x')
    {
        buf.push_back('x');
        c = getChar();
        if (IS_USASCII(c) && isxdigit(c))
        {
            do
            {
                buf.push_back(tolower(c));
                c = getChar();
            }
            while (IS_USASCII(c) && isxdigit(c));
        }
        else if (c == 0)
        {
            // assume hexadecimal follows in the next input
        }
        else
        {
            throw InvalidCharException();
        }
        return true;
    }
    return false;
}
