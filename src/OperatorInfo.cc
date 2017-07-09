// Copyright (C) 2014-2017 Hideaki Narita


#include <string.h>
#include "OperatorInfo.h"


using namespace hnrt;


OperatorInfo OperatorInfo::singleton;


bool OperatorKeyLessThan::operator ()(const char *a, const char *b) const
{
    return strcmp(a, b) < 0 ? true : false;
}


OperatorInfo::OperatorInfo()
{
    insert(OperatorMapEntry("{abs}", SYM_ABS));
    insert(OperatorMapEntry("{cbrt}", SYM_CBRT));
    insert(OperatorMapEntry("{cos}", SYM_COS));
    insert(OperatorMapEntry("{exp}", SYM_EXP));
    insert(OperatorMapEntry("{hypot}", SYM_HYPOT));
    insert(OperatorMapEntry("{log}", SYM_LOG));
    insert(OperatorMapEntry("{log2}", SYM_LOG2));
    insert(OperatorMapEntry("{log10}", SYM_LOG10));
    insert(OperatorMapEntry("{pow}", SYM_POW));
    insert(OperatorMapEntry("{sin}", SYM_SIN));
    insert(OperatorMapEntry("{sqrt}", SYM_SQRT));
    insert(OperatorMapEntry("{tan}", SYM_TAN));
}


//
// Searchs the map for the given key and
// returns the terminal symbol value associated with it.
// If not found, SYM_ERROR is returned.
//
TerminalSymbol OperatorInfo::find(const char *key) const
{
    OperatorMap::const_iterator iter = OperatorMap::find(key);
    if (iter != OperatorMap::end())
    {
        return iter->second;
    }
    else
    {
        return SYM_ERROR;
    }
}


//
// Searchs the map for the given symbol and
// returns the key string associated with it.
// If not found, "?" is returned.
//
const char *OperatorInfo::find(TerminalSymbol sym) const
{
    for (OperatorMap::const_iterator iter = OperatorMap::begin(); iter != OperatorMap::end(); iter++)
    {
        if (iter->second == sym)
        {
            return iter->first;
        }
    }
    return "?";
}


//
// Tries to complement the given string (not null-terminated) with the existing operators.
//
void OperatorInfo::Complement(std::vector<char> &buffer) const
{
    std::vector<const char *> match;
    size_t n = ~0;
    for (OperatorMap::const_iterator iter = OperatorMap::begin(); iter != OperatorMap::end(); iter++)
    {
        const char *s = iter->first;
        if (!strncmp(s, &buffer[0], buffer.size()))
        {
            match.push_back(s);
            size_t m = strlen(s);
            if (n > m)
            {
                n = m;
            }
        }
    }
    if (match.size() == 1)
    {
        buffer.resize(n);
        memcpy(&buffer[0], match[0], n);
    }
    else if (match.size() > 1)
    {
        const char *s = match[0];
        for (size_t i = buffer.size(); i < n; i++)
        {
            for (size_t j = 1; j < match.size(); j++)
            {
                const char *t = match[j];
                if (t[i] != s[i])
                {
                    return;
                }
            }
            buffer.push_back(s[i]);
        }
    }
}
