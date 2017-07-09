// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_VARIABLESTORE_H
#define IKURA_VARIABLESTORE_H


#include <map>
#include <set>
#include <vector>
#include <glibmm/ustring.h>
#include <sigc++/sigc++.h>


namespace hnrt
{
    class VariableKeyLessThan
    {
    public:

        bool operator ()(const Glib::ustring&, const Glib::ustring&) const;
    };


    typedef std::map<Glib::ustring, Glib::ustring, VariableKeyLessThan> VariableMap;
    typedef std::pair<Glib::ustring, Glib::ustring> VariableMapEntry;
    typedef std::set<Glib::ustring, VariableKeyLessThan> VariableKeySet;


    //
    // Variable name-to-value mapping singleton class
    //
    class VariableStore : public VariableMap
    {
    public:

        static VariableStore &instance() { return singleton; }

        virtual ~VariableStore() {}

        //
        // Call this method once default values were added.
        // e.g. In German locale settings, decimal point is comma, not period.
        //
        void periodToDecimalPoint();

        bool hasKey(const Glib::ustring& key) const;

        //
        // Searchs the map for the given key and
        // returns the value associated with it.
        // If not found, an empty string is returned.
        // If the given key is in evaluation, RecursiveVariableAccessException is thrown.
        //
        Glib::ustring getValue(const Glib::ustring& key) const;

        void setValue(const Glib::ustring& key, const Glib::ustring& value);

        void add(const Glib::ustring& key);
        void add(const Glib::ustring& key, const Glib::ustring& value);

        bool isInEvaluation(const Glib::ustring& key) const;

        //
        // Adds the given key to inEvaluation set.
        // If the same key is already in this set, RecursiveVariableAccessException is thrown.
        //
        void setInEvaluation(const Glib::ustring& key);

        //
        // Removes the given key from inEvaluation set.
        //
        void unsetInEvaluation(const Glib::ustring& key);

        //
        // Tries to complement the given string (not null-terminated) with the existing operators.
        //
        void Complement(std::vector<char> &buffer) const;

        sigc::signal<void, const char*, const char*> signalAdd() { return sigAdd; }
        sigc::signal<void, const char*, const char*> signalChange() { return sigChange; }

    protected:

        static VariableStore singleton;

        VariableStore() {}
        VariableStore(const VariableStore &) {}

        VariableKeySet inEvaluation;
        sigc::signal<void, const char*, const char*> sigAdd;
        sigc::signal<void, const char*, const char*> sigChange;
    };
}


#endif //!IKURA_VARIABLESTORE_H
