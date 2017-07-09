// Copyright (C) 2014-2017 Hideaki Narita


#include "HistoryBuffer.h"
#include "Exception.h"


using namespace hnrt;


#define SUPER std::vector<Glib::ustring>


HistoryBuffer::HistoryBuffer()
    : index(0)
{
}


HistoryBuffer::~HistoryBuffer()
{
}


const Glib::ustring& HistoryBuffer::at(std::vector<Glib::ustring>::size_type i)
{
    if (i < SUPER::size())
    {
        if (index != i)
        {
            index = i;
            sigIndexChange.emit();
        }
        return SUPER::at(index);
    }
    else
    {
        throw HistoryIndexOutOfBoundsException();
    }
}


void HistoryBuffer::clear()
{
    if (SUPER::size())
    {
        SUPER::clear();
        index = 0;
        sigContentsChange.emit();
    }
}


//
// This method searches its vector for the same entry as the given string.
// If the same entry is found, it is deleted.
// Then, a new entry for the given string is appended to this vector.
//
void HistoryBuffer::push_back(const Glib::ustring& s)
{
    for (SUPER::iterator iter = begin(); iter != end(); iter++)
    {
        if (*iter == s)
        {
            erase(iter);
            break;
        }
    }
    SUPER::push_back(s);
    index = SUPER::size();
    sigContentsChange.emit();
}


void HistoryBuffer::moveIndexToEnd()
{
    if (index != size())
    {
        index = size();
        sigIndexChange.emit();
    }
}
