// Copyright (C) 2014-2017 Hideaki Narita


#include <string.h>
#include "UTF8.h"


using namespace hnrt;


//
// Reads from UTF-8 string and returns a UNICODE scalar value ranged from 0 to 0x10FFFF.
// If it encounters an error, a value of -1 is returned.
//
// start ... pointer to the string to be read from
// end ..... pointer to the character next to the last one of the string
// next .... pointer to the pointer that receives the position of the next character
//           This can be NULL if the caller doesn't need it.
//
int UTF8::getChar(const char *start, const char *end, const char **next)
{
    const char *cur = start;
    int c;
    if (cur < end)
    {
        c = *cur++ & 0xFF;
    }
    else
    {
        goto failure;
    }
    if (c < 0x80)
    {
        // nothing to be done here
    }
    else if ((c & 0xE0) == 0xC0)
    {
        if (cur + 1 <= end &&
            (cur[0] & 0xC0) == 0x80)
        {
            c &= 0x1F;
            c = (c << 6) | (*cur++ & 0x3F);
        }
        else
        {
            goto failure;
        }
    }
    else if ((c & 0xF0) == 0xE0)
    {
        if (cur + 2 <= end &&
            (cur[0] & 0xC0) == 0x80 &&
            (cur[1] & 0xC0) == 0x80)
        {
            c &= 0x0F;
            c = (c << 6) | (*cur++ & 0x3F);
            c = (c << 6) | (*cur++ & 0x3F);
        }
        else
        {
            goto failure;
        }
    }
    else if ((c & 0xF8) == 0xF0)
    {
        if (cur + 3 <= end &&
            (cur[0] & 0xC0) == 0x80 &&
            (cur[1] & 0xC0) == 0x80 &&
            (cur[2] & 0xC0) == 0x80)
        {
            c &= 0x07;
            c = (c << 6) | (*cur++ & 0x3F);
            c = (c << 6) | (*cur++ & 0x3F);
            c = (c << 6) | (*cur++ & 0x3F);
            if (c > 0x10FFFF)
            {
                // The value exceeds the maximum value of UNICODE character.
                // It is considered to rarely happen...just ignore this error.
            }
        }
        else
        {
            goto failure;
        }
    }
    else
    {
        goto failure;
    }
    if (next)
    {
        *next = cur;
    }
    return c;
failure:
    if (next)
    {
        while (cur < end && (*cur & 0xC0) == 0x80)
        {
            cur++;
        }
        *next = cur;
    }
    return -1;
}


//
// Encodes the given UNICODE scalar value in UTF-8 and
// appends the encoded byte(s) to the std::vector<char> buffer.
// If the given UNICODE scalar value is invalid, no bytes are appended to the buffer.
//
void UTF8::pushBack(std::vector<char> &buffer, int c)
{
    if (c < 0x80)
    {
        buffer.push_back((char)c);
    }
    else if (c < 0x800)
    {
        char c2 = (char)(0x80 | (c & 0x3F));
        c >>= 6;
        char c1 = (char)(0xC0 | (c & 0x1F));
        buffer.push_back(c1);
        buffer.push_back(c2);
    }
    else if (c < 0x10000)
    {
        char c3 = (char)(0x80 | (c & 0x3F));
        c >>= 6;
        char c2 = (char)(0x80 | (c & 0x3F));
        c >>= 6;
        char c1 = (char)(0xE0 | (c & 0x0F));
        buffer.push_back(c1);
        buffer.push_back(c2);
        buffer.push_back(c3);
    }
    else if (c < 0x110000)
    {
        char c4 = (char)(0x80 | (c & 0x3F));
        c >>= 6;
        char c3 = (char)(0x80 | (c & 0x3F));
        c >>= 6;
        char c2 = (char)(0x80 | (c & 0x3F));
        c >>= 6;
        char c1 = (char)(0xF0 | (c & 0x07));
        buffer.push_back(c1);
        buffer.push_back(c2);
        buffer.push_back(c3);
        buffer.push_back(c4);
    }
}


//
// Returns the number of bytes that needs to store the given character in UTF-8.
//
int UTF8::getLength(int c)
{
    if (c < 0x80)
    {
        return 1;
    }
    else if (c < 0x800)
    {
        return 2;
    }
    else if (c < 0x10000)
    {
        return 3;
    }
    else if (c < 0x110000)
    {
        return 4;
    }
    else
    {
        return 0;
    }
}


//
// Returns the number of bytes in the given number of characters.
//
// s ... pointer to the string to check
// n ... length of the string in bytes
// x ... number of characters to check
//
int UTF8::getLength(const char *s, size_t n, int x)
{
    const char *p1 = s;
    const char *p2 = s + n;
    while (x > 0 && p1 < p2)
    {
        int c = getChar(p1, p2, &p1);
        (void)c;
        x--;
    }
    return p1 - s;
}


//
// Returns the number of characters.
//
// s ... pointer to the string to check
// n ... length of the string in bytes
//
int UTF8::count(const char *s, size_t n)
{
    int m = 0;
    const char *p1 = s;
    const char *p2 = s + n;
    while (p1 < p2)
    {
        int c = getChar(p1, p2, &p1);
        (void)c;
        m++;
    }
    return m;
}


//
// Returns the number of occurrence of the given characters.
//
// s ... pointer to the string to check
// n ... length of the string in bytes
//
int UTF8::count(const char *s, size_t n, int c)
{
    int m = 0;
    const char *p1 = s;
    const char *p2 = s + n;
    while (p1 < p2)
    {
        int d = getChar(p1, p2, &p1);
        if (d == c)
        {
            m++;
        }
    }
    return m;
}


//
// Replaces arithmetic signs in the given string with alternates which are visually distinct
// such as MINUS_SIGN, MUL_SIGN, and DIV_SIGN (see the defines) and
// returns the resulting string.
//
Glib::ustring UTF8::replaceArithmeticSignsWithAlternates(const char* s)
{
    const char* p = s;
    const char* q = s + strlen(s);
    std::vector<char>::size_type n = 0;
    while (p < q)
    {
        char c = *p++;
        if (c == '-')
        {
            n += MINUS_SIGN_LEN;
        }
        else if (c == '*')
        {
            n += MUL_SIGN_LEN;
        }
        else if (c == '/')
        {
            n += DIV_SIGN_LEN;
        }
        else
        {
            n++;
        }
    }
    n++; // for terminating null
    std::vector<char> buffer;
    buffer.resize(n);
    p = s;
    char *t = &buffer[0];
    while (p < q)
    {
        char c = *p++;
        if (c == '-')
        {
            memcpy(t, MINUS_SIGN, MINUS_SIGN_LEN);
            t += MINUS_SIGN_LEN;
        }
        else if (c == '*')
        {
            memcpy(t, MUL_SIGN, MUL_SIGN_LEN);
            t += MUL_SIGN_LEN;
        }
        else if (c == '/')
        {
            memcpy(t, DIV_SIGN, DIV_SIGN_LEN);
            t += DIV_SIGN_LEN;
        }
        else
        {
            *t++ = c;
        }
    }
    *t = 0;
    return Glib::ustring(&buffer[0]);
}
