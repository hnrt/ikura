// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_INPUTBUFFER_H
#define IKURA_INPUTBUFFER_H


#include <vector>
#include <sigc++/sigc++.h>


namespace hnrt
{
    //
    // Input buffer for arithmetic expression
    //
    class InputBuffer : protected std::vector<char>
    {
    public:

        InputBuffer();
        virtual ~InputBuffer();
        std::vector<char>::size_type size() const { return std::vector<char>::size(); }
        operator const char* () const { return size() ? &at(0) : ""; }
        bool isJustEvaluated() const { return justEvaluated; }
        bool getGrouping() const;
        void setGrouping(bool value);
        bool getHexadecimal() const;
        void setHexadecimal(bool value);
        int getPrecision() const;
        void setPrecision(int value);
        void clear();
        void assign(const char* s);
        InputBuffer &operator =(const char *s) { assign(s); return *this; }
        void putChar(int c);
        void putString(const char* s);
        void parse();
        void evaluate();
        void deleteLastChar();
        sigc::signal<void, const char*> signalTextChange() { return sigTextChange; }
        sigc::signal<void, const char*> signalTooltipChange() { return sigTooltipChange; }
        sigc::signal<void> signalClear() { return sigClear; }
        sigc::signal<void> signalFirstChar() { return sigFirstChar; }
        sigc::signal<void, const char*, const char*> signalEvaluated() { return sigEvaluated; }
        sigc::signal<void> signalInvalidChar() { return sigInvalidChar; }
        sigc::signal<void> signalIncompleteExpression() { return sigIncompleteExpression; }
        sigc::signal<void> signalDivisionByZero() { return sigDivisionByZero; }
        sigc::signal<void> signalOverflow() { return sigOverflow; }
        sigc::signal<void> signalUnderflow() { return sigUnderflow; }
        sigc::signal<void> signalEvaluationInability() { return sigEvaluationInability; }
        sigc::signal<void, const char*> signalRecursiveVariableAccess() { return sigRecursiveVariableAccess; }

    protected:

        InputBuffer(const InputBuffer&) {}

        size_t validSize; // this is the size of the valid input; updated after successful parsing.
        bool justEvaluated; // set to true right after equal was received.
        int formatFlags;
        sigc::signal<void, const char*> sigTextChange;
        sigc::signal<void, const char*> sigTooltipChange;
        sigc::signal<void> sigClear;
        sigc::signal<void> sigFirstChar;
        sigc::signal<void, const char*, const char*> sigEvaluated; // first=expression, second=result
        sigc::signal<void> sigInvalidChar;
        sigc::signal<void> sigIncompleteExpression;
        sigc::signal<void> sigDivisionByZero;
        sigc::signal<void> sigOverflow;
        sigc::signal<void> sigUnderflow;
        sigc::signal<void> sigEvaluationInability;
        sigc::signal<void, const char*> sigRecursiveVariableAccess;
    };
}


#endif //!IKURA_INPUTBUFFER_H
