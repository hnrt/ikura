// Copyright (C) 2014-2017 Hideaki Narita


#include <errno.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include "LocaleInfo.h"
#include "UTF8.h"


using namespace hnrt;


LocaleInfo LocaleInfo::singleton;


LocaleInfo::LocaleInfo()
    : locale(setlocale(LC_ALL, ""))
    , decimalPointString(".")
    , decimalPoint('.')
{
    struct lconv* lconvptr = localeconv();
    if (lconvptr)
    {
        if (lconvptr->decimal_point)
        {
            decimalPointString = lconvptr->decimal_point;
            decimalPoint = UTF8::getChar(lconvptr->decimal_point, lconvptr->decimal_point + strlen(lconvptr->decimal_point));
        }
    }
}


void LocaleInfo::init()
{
    // it is important for this method to be called at start time
    // but actually nothing to do at this moment because singleton constructor did it
}


//
// Returns the root directory path of the message catalogs.
//
Glib::ustring LocaleInfo::getMessageCatalogDir(const char* textdomainname)
{
    static const char defaultPath[] = { "./" };

    Glib::ustring firstPart = defaultPath;

    {
        std::vector<Glib::ustring> candidates;

        Glib::ustring lastPart = Glib::ustring::compose("/LC_MESSAGES/%1.mo", textdomainname);
        candidates.push_back(Glib::ustring::compose("%1%2", locale, lastPart)); // full locale specification
        ssize_t i = locale.find('@'); // find @modifier part
        if (i > 0)
        {
            candidates.push_back(Glib::ustring::compose("%1%2", locale.substr(0, i), lastPart)); // language_territory.codeset
        }
        i = locale.find('.'); // find .codeset part
        if (i > 0)
        {
            candidates.push_back(Glib::ustring::compose("%1%2", locale.substr(0, i), lastPart)); // language_territory
        }
        i = locale.find('_'); // find _territory part
        if (i > 0)
        {
            candidates.push_back(Glib::ustring::compose("%1%2", locale.substr(0, i), lastPart)); // language
        }

        //
        // first get executable path
        //
        std::vector<char> path(32);
        while (1)
        {
            // this is linux specific
            ssize_t len = readlink("/proc/self/exe", &path[0], path.size());
            if (len < 0)
            {
                g_printerr("Error: Unable to read /proc/self/exe: %s\n", strerror(errno));
                goto done;
            }
            if (len < (ssize_t)path.size())
            {
                path[len] = '\0';
                break;
            }
            path.resize(path.size() * 2);
        }
        //
        // then get directory where executable path is located.
        // the resulting path ends with a separator.
        //
        char* sep = strrchr(&path[0], '/');
        if (!sep)
        {
            goto done;
        }
        sep[1] = '\0';

        //
        // directory to check first
        //
        firstPart = &path[0];

        for (std::vector<Glib::ustring>::const_iterator iter = candidates.begin(); iter != candidates.end(); iter++)
        {
            Glib::ustring filename = Glib::ustring::compose("%1%2", firstPart, *iter);
            if (!access(filename.c_str(), R_OK))
            {
                goto done;
            }
        }

        //
        // directory to check second
        //
        firstPart = "/usr/local/share/locale/";

        for (std::vector<Glib::ustring>::const_iterator iter = candidates.begin(); iter != candidates.end(); iter++)
        {
            Glib::ustring filename = Glib::ustring::compose("%1%2", firstPart, *iter);
            if (!access(filename.c_str(), R_OK))
            {
                goto done;
            }
        }

        //
        // directory to check last
        //
        firstPart = "/usr/share/locale/";

        for (std::vector<Glib::ustring>::const_iterator iter = candidates.begin(); iter != candidates.end(); iter++)
        {
            Glib::ustring filename = Glib::ustring::compose("%1%2", firstPart, *iter);
            if (!access(filename.c_str(), R_OK))
            {
                goto done;
            }
        }

        //
        // all attempts failed
        //
        firstPart = defaultPath;
    }

done:

    //
    // delete the last separator
    //
    firstPart.resize(firstPart.size() - 1);

    return firstPart;
}


//
// Replaces each of all periods in the given string with the locale-dependent decimal point and
// returns the resulting string.
//
Glib::ustring LocaleInfo::periodToDecimalPointString(const Glib::ustring& s)
{
    int dp = getDecimalPoint();
    if (dp == '.')
    {
        return Glib::ustring(s);
    }
    std::vector<char> buffer;
    const char *p = s.c_str();
    const char *q = p + s.bytes();
    while (p < q)
    {
        int c = UTF8::getChar(p, q, &p);
        if (c < 0)
        {
            break;
        }
        else if (c == '.')
        {
            UTF8::pushBack(buffer, dp);
        }
        else
        {
            UTF8::pushBack(buffer, c);
        }
    }
    buffer.push_back('\0');
    return Glib::ustring(&buffer[0]);
}
