// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_UTF8_H
#define IKURA_UTF8_H


#include <vector>
#include <glibmm/ustring.h>


namespace hnrt
{
    class UTF8
    {
    public:

        //
        // Reads from UTF-8 string and returns a UNICODE scalar value ranged from 0 to 0x10FFFF.
        // If it encounters an error, a value of -1 is returned.
        //
        // start ... pointer to the string to be read from
        // end ..... pointer to the character next to the last one of the string
        // next .... pointer to the pointer that receives the position of the next character
        //           This can be NULL if the caller doesn't need it.
        //
        static int getChar(const char *start, const char *end, const char **next = NULL);

        //
        // Encodes the given UNICODE scalar value in UTF-8 and
        // appends the encoded byte(s) to the std::vector<char> buffer.
        // If the given UNICODE scalar value is invalid, no bytes are appended to the buffer.
        //
        static void pushBack(std::vector<char> &buffer, int c);

        //
        // Returns the number of bytes that needs to store the given character in UTF-8.
        //
        static int getLength(int c);

        //
        // Returns the number of bytes in the given number of characters.
        //
        // s ... pointer to the string to check
        // n ... length of the string in bytes
        // x ... number of characters to check
        //
        static int getLength(const char *s, size_t n, int x);

        //
        // Returns the number of characters.
        //
        // s ... pointer to the string to check
        // n ... length of the string in bytes
        //
        static int count(const char *s, size_t n);

        //
        // Returns the number of occurrence of the given characters.
        //
        // s ... pointer to the string to check
        // n ... length of the string in bytes
        //
        static int count(const char *s, size_t n, int c);

        //
        // Replaces arithmetic signs in the given string with alternates which are visually distinct
        // such as MINUS_SIGN, MUL_SIGN, and DIV_SIGN (see the defines) and
        // returns the resulting string.
        //
        static Glib::ustring replaceArithmeticSignsWithAlternates(const char* s);
    };
}


#define MINUS_SIGN "\xE2\x88\x92" // U+2212 minus sign
#define MINUS_SIGN_LEN 3
#define MUL_SIGN "\xC3\x97" // U+00D7 multiplication sign
#define MUL_SIGN_LEN 2
#define DIV_SIGN "\xC3\xB7" // U+00F7 division sign
#define DIV_SIGN_LEN 2


#endif //!IKURA_UTF8_H
