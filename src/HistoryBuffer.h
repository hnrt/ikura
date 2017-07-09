// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_HISTORYBUFFER_H
#define IKURA_HISTORYBUFFER_H


#include <vector>
#include <glibmm/ustring.h>
#include <sigc++/sigc++.h>


namespace hnrt
{
    //
    // History buffer for arithmetic expression
    //
    class HistoryBuffer : protected std::vector<Glib::ustring>
    {
    public:

        HistoryBuffer();
        virtual ~HistoryBuffer();
        std::vector<Glib::ustring>::size_type size() { return std::vector<Glib::ustring>::size(); }
        const Glib::ustring& at(std::vector<Glib::ustring>::size_type i);
        const Glib::ustring& operator [](std::vector<Glib::ustring>::size_type i) { return at(i); }
        void clear();
        void push_back(const Glib::ustring& s);
        void push_back(const char *s) { push_back(Glib::ustring(s)); }
        std::vector<Glib::ustring>::size_type getIndex() const { return index; }
        void moveIndexToEnd();
        sigc::signal<void> signalContentsChange() { return sigContentsChange; }
        sigc::signal<void> signalIndexChange() { return sigIndexChange; }

    protected:

        HistoryBuffer(const HistoryBuffer&) {}

        std::vector<Glib::ustring>::size_type index;
        sigc::signal<void> sigContentsChange;
        sigc::signal<void> sigIndexChange;
    };
}


#endif //!IKURA_HISTORYBUFFER_H
