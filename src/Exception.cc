// Copyright (C) 2014-2017 Hideaki Narita


#include <libintl.h>
#include "Exception.h"


using namespace hnrt;


InvalidCharException::InvalidCharException()
    : Exception(gettext("Invalid character"))
{
}


DivideByZeroException::DivideByZeroException()
    : Exception(gettext("Division by zero"))
{
}


OverflowException::OverflowException()
    : Exception(gettext("Overflow"))
{
}


UnderflowException::UnderflowException()
    : Exception(gettext("Underflow"))
{
}


EvaluationInabilityException::EvaluationInabilityException()
    : Exception(gettext("Caclulation impossible"))
{
}


RecursiveVariableAccessException::RecursiveVariableAccessException(const Glib::ustring& key_)
    : Exception(Glib::ustring::compose(gettext("%1: Recursively referenced"), key_))
    , key(key_)
{
}
