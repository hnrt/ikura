// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_PARSER_H
#define IKURA_PARSER_H


#include "Expression.h"
#include "Lexer.h"


namespace hnrt
{
    //
    // Parser for arithmetic expression represented in string form
    //
    class Parser
    {
    public:

        Parser(const char* s, size_t n, bool complete = false);
        Expression* run();

    protected:

        Parser(const Parser&);
        Expression* parseExpr1();
        Expression* parseExpr2();
        Expression* parseExpr3();
        Expression* parseExpr4();
        Expression* parseExpr5();

        Lexer lexer;
        bool complete;
        int sym;
    };
}


#endif //!IKURA_PARSER_H
