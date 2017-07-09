// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_OPERATORINFO_H
#define IKURA_OPERATORINFO_H


#include <map>
#include <vector>
#include "TerminalSymbol.h"


namespace hnrt
{
    class OperatorKeyLessThan
    {
    public:

        bool operator ()(const char *a, const char *b) const;
    };


    typedef std::map<const char *, TerminalSymbol, OperatorKeyLessThan> OperatorMap;
    typedef std::pair<const char *, TerminalSymbol> OperatorMapEntry;


    //
    // Custom operator string-to-TerminalSymbol mapping singleton class
    //
    class OperatorInfo : protected OperatorMap
    {
    public:

        static const OperatorInfo &instance() { return singleton; }

        virtual ~OperatorInfo() {}

        //
        // Searchs the map for the given key and
        // returns the terminal symbol value associated with it.
        // If not found, SYM_ERROR is returned.
        //
        TerminalSymbol find(const char *key) const;

        //
        // Searchs the map for the given symbol and
        // returns the key string associated with it.
        // If not found, "?" is returned.
        //
        const char *find(TerminalSymbol sym) const;

        //
        // Tries to complement the given string (not null-terminated) with the existing operators.
        //
        void Complement(std::vector<char> &buffer) const;

    protected:

        static OperatorInfo singleton;

        OperatorInfo();
        OperatorInfo(const OperatorInfo &) {}
    };
}


#endif //!IKURA_OPERATORINFO_H
