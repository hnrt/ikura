// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_TERMINALSYMBOL_H
#define IKURA_TERMINALSYMBOL_H


namespace hnrt
{
    enum TerminalSymbol
    {
        SYM_EOF = -1,
        SYM_PLUS = '+',
        SYM_MINUS = '-',
        SYM_MULTIPLY = '*',
        SYM_DIVIDE = '/',
        SYM_LPAREN = '(',
        SYM_RPAREN = ')',
        SYM_ERROR = 0xE000, // using UNICODE private use area
        SYM_INTEGER,
        SYM_REALNUMBER,
        SYM_IDENTIFIER,
        SYM_E, // exponent of a real number
        SYM_ASSIGN,
        SYM_INCOMPLETE_OPERATOR,
        SYM_ABS,
        SYM_CBRT,
        SYM_COS,
        SYM_EXP,
        SYM_HYPOT,
        SYM_LOG,
        SYM_LOG2,
        SYM_LOG10,
        SYM_POW,
        SYM_SIN,
        SYM_SQRT,
        SYM_TAN,
    };
}


#endif //!IKURA_TERMINALSYMBOL_H
