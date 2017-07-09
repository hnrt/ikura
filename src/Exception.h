// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_EXCEPTION_H
#define IKURA_EXCEPTION_H


#include <glibmm/ustring.h>


namespace hnrt
{
    class Exception
    {
    public:

        Exception()
        {
        }

        Exception(const Glib::ustring& what_)
            : what(what_)
        {
        }

        Exception(const Exception& other)
            : what(other.what)
        {
        }

        virtual ~Exception()
        {
        }

        const Glib::ustring& getWhat() const { return what; }

    protected:

        Glib::ustring what;
    };


    class InvalidCharException : public Exception
    {
    public:

        InvalidCharException();

        InvalidCharException(const InvalidCharException& other)
            : Exception(other)
        {
        }
    };


    class InvalidExpressionException : public Exception
    {
    public:

        InvalidExpressionException(const Glib::ustring& what)
            : Exception(what)
        {
        }

        InvalidExpressionException(const InvalidExpressionException& other)
            : Exception(other)
        {
        }
    };


    class DivideByZeroException : public Exception
    {
    public:

        DivideByZeroException();

        DivideByZeroException(const DivideByZeroException& other)
            : Exception(other)
        {
        }
    };


    class OverflowException : public Exception
    {
    public:

        OverflowException();

        OverflowException(const OverflowException& other)
            : Exception(other)
        {
        }
    };


    class UnderflowException : public Exception
    {
    public:

        UnderflowException();

        UnderflowException(const UnderflowException& other)
            : Exception(other)
        {
        }
    };


    class EvaluationInabilityException : public Exception
    {
    public:

        EvaluationInabilityException();

        EvaluationInabilityException(const Glib::ustring& what)
            : Exception(what)
        {
        }

        EvaluationInabilityException(const EvaluationInabilityException& other)
            : Exception(other)
        {
        }
    };


    class IndexOutOfBoundsException : public Exception
    {
    public:

        IndexOutOfBoundsException()
        {
        }

        IndexOutOfBoundsException(const IndexOutOfBoundsException& other)
            : Exception(other)
        {
        }
    };


    class HistoryIndexOutOfBoundsException : public IndexOutOfBoundsException
    {
    public:

        HistoryIndexOutOfBoundsException()
        {
        }

        HistoryIndexOutOfBoundsException(const HistoryIndexOutOfBoundsException& other)
            : IndexOutOfBoundsException(other)
        {
        }
    };


    class RecursiveVariableAccessException : public Exception
    {
    public:

        RecursiveVariableAccessException(const Glib::ustring& key_);

        RecursiveVariableAccessException(const RecursiveVariableAccessException& other)
            : Exception(other), key(other.key)
        {
        }

        const Glib::ustring& getKey() const { return key; }
        
    protected:

        Glib::ustring key;
    };
}


#endif //!IKURA_EXCEPTION_H
