// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_LOCALEINFO_H
#define IKURA_LOCALEINFO_H


#include <glibmm/ustring.h>


namespace hnrt
{
    class LocaleInfo
    {
    public:

        static LocaleInfo& instance() { return singleton; }

        //
        // Returns the locale dependent decimal point string in the current charset/encoding (UTF-8).
        //
        static const char* getDecimalPointString() { return singleton.decimalPointString.c_str(); }

        //
        // Returns the UNICODE scalar value of the locale dependent decimal point.
        //
        static int getDecimalPoint() { return singleton.decimalPoint; }

        //
        // Replaces each of all periods in the given string with the locale-dependent decimal point and
        // returns the resulting string.
        //
        static Glib::ustring periodToDecimalPointString(const Glib::ustring& s);

        //
        // IMPORTANT: Call this method at start time
        //
        void init();

        //
        // Returns the root directory path of the message catalogs.
        //
        Glib::ustring getMessageCatalogDir(const char* textdomainname);

    private:

        static LocaleInfo singleton;

        LocaleInfo();
        LocaleInfo(const LocaleInfo&) {}

        Glib::ustring locale;
        Glib::ustring decimalPointString;
        int decimalPoint;
    };
}


#endif //!IKURA_LOCALEINFO_H
