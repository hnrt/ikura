// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_LEXER_H
#define IKURA_LEXER_H


#include <vector>
#include "TerminalSymbol.h"


namespace hnrt
{
    //
    // Lexical analyzer for Parser class
    //
    class Lexer
    {
    public:

        Lexer(const char *s, size_t n);
        int getSym();
        long getInteger() const { return v.integer; }
        long double getRealNumber() const { return v.realNumber; }
        const char *getString() const { return &buf[0]; }

    protected:

        Lexer(const Lexer&) {}
        int getChar();
        bool parseDecimalFractionPart();
        bool parseExponentPart();
        bool parseHexadecimal();

        const char *next;
        const char *stop;
        int c;
        union TokenValue
        {
            long integer;
            long double realNumber;
        } v;
        std::vector<char> buf;
    };
}


#endif //!IKURA_LEXER_H
