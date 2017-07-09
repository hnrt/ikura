// Copyright (C) 2014-2017 Hideaki Narita


#include <string.h>
#include "VariableStore.h"
#include "Exception.h"
#include "LocaleInfo.h"


using namespace hnrt;


VariableStore VariableStore::singleton;


bool VariableKeyLessThan::operator ()(const Glib::ustring& a, const Glib::ustring& b) const
{
    return a < b;
}


//
// Call this method once default values were added.
// e.g. In German locale settings, decimal point is comma, not period.
//
void VariableStore::periodToDecimalPoint()
{
    if (LocaleInfo::getDecimalPoint() == '.')
    {
        return;
    }
    for (VariableMap::iterator iter = VariableMap::begin(); iter != VariableMap::end(); iter++)
    {
        const Glib::ustring& key = iter->first;
        Glib::ustring& value = iter->second;
        ssize_t i = value.find('.');
        if (i == -1)
        {
            continue;
        }
        value = LocaleInfo::periodToDecimalPointString(value);
        sigChange.emit(key.c_str(), value.c_str());
    }
}


bool VariableStore::hasKey(const Glib::ustring& key) const
{
    VariableMap::const_iterator iter = VariableMap::find(key);
    return iter != VariableMap::end();
}


//
// Searchs the map for the given key and
// returns the value associated with it.
// If not found, an empty string is returned.
// If the given key is in evaluation, RecursiveVariableAccessException is thrown.
//
Glib::ustring VariableStore::getValue(const Glib::ustring& key) const
{
    if (isInEvaluation(key))
    {
        throw RecursiveVariableAccessException(key);
    }
    Glib::ustring value;
    VariableMap::const_iterator iter = VariableMap::find(key);
    if (iter != VariableMap::end())
    {
        value = iter->second;
    }
    return value;
}


void VariableStore::setValue(const Glib::ustring& key, const Glib::ustring& value)
{
    VariableMap::iterator iter = VariableMap::find(key);
    if (iter != VariableMap::end())
    {
        iter->second = value;
        sigChange.emit(key.c_str(), value.c_str());
    }
}


void VariableStore::add(const Glib::ustring& key)
{
    VariableMap::iterator iter = VariableMap::find(key);
    if (iter == VariableMap::end())
    {
        Glib::ustring value;
        VariableMap::insert(VariableMapEntry(key, value));
        sigAdd.emit(key.c_str(), value.c_str());
    }
}


void VariableStore::add(const Glib::ustring& key, const Glib::ustring& value)
{
    VariableMap::iterator iter = VariableMap::find(key);
    if (iter != VariableMap::end())
    {
        iter->second = value;
        sigChange.emit(key.c_str(), value.c_str());
    }
    else
    {
        VariableMap::insert(VariableMapEntry(key, value));
        sigAdd.emit(key.c_str(), value.c_str());
    }
}


bool VariableStore::isInEvaluation(const Glib::ustring& key) const
{
    VariableKeySet::const_iterator iter = inEvaluation.find(key);
    return iter != inEvaluation.end();
}


//
// Adds the given key to inEvaluation set.
// If the same key is already in this set, RecursiveVariableAccessException is thrown.
//
void VariableStore::setInEvaluation(const Glib::ustring& key)
{
    if (isInEvaluation(key))
    {
        throw RecursiveVariableAccessException(key);
    }
    inEvaluation.insert(key);
}


//
// Removes the given key from inEvaluation set.
//
void VariableStore::unsetInEvaluation(const Glib::ustring& key)
{
    VariableKeySet::iterator iter = inEvaluation.find(key);
    if (iter != inEvaluation.end())
    {
        inEvaluation.erase(iter);
    }
}


//
// Tries to complement the given string (not null-terminated) with the existing operators.
//
void VariableStore::Complement(std::vector<char> &buffer) const
{
    std::vector<const char *> match;
    size_t n = ~0;
    for (VariableMap::const_iterator iter = VariableMap::begin(); iter != VariableMap::end(); iter++)
    {
        const char *s = iter->first.c_str();
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
