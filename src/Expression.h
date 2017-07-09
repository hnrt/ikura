// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_EXPRESSION_H
#define IKURA_EXPRESSION_H


#include <vector>
#include <glibmm/ustring.h>


namespace hnrt
{
    enum ExpressionType
    {
        ET_INTEGER,
        ET_REALNUMBER,
        ET_ADD,
        ET_SUBTRACT,
        ET_MULTIPLY,
        ET_DIVIDE,
        ET_UNARY_MINUS,
        ET_INTEGER_MAX_PLUS_ONE,
        ET_BLOCK,
        ET_INCOMPLETE_BLOCK, // block missing expression or right parenthesis
        ET_INCOMPLETE, // operator missing right brace
        ET_VARIABLE,
        ET_ASSIGN,
        ET_ABS,
        ET_CBRT,
        ET_COS,
        ET_EXP,
        ET_HYPOT,
        ET_LOG,
        ET_LOG2,
        ET_LOG10,
        ET_POW,
        ET_SIN,
        ET_SQRT,
        ET_TAN,
    };


    enum ExpressionFormat // bitwise flag
    {
        EF_GROUPING = 1, // thousands' grouping
        EF_HEXADECIMAL = 2, // integer in hexadecimal format
        EF_PREPENDZERO = 4, // prepend zero if real number begins with decimal point
        EF_PRECISION10 = 8, // precision 10 for real number
        EF_PRECISION20 = 16, // precision 20 for real number
    };


    //
    // Base class for handling arithmetic expression
    //
    class Expression
    {
    public:

        Expression(ExpressionType type_) : type(type_) {}
        virtual ~Expression() {}
        enum ExpressionType getType() const { return type; }
        virtual void format(std::vector<char> &buffer, int flags) = 0;
        virtual Expression* evaluate(bool permanent) = 0;

        static Expression* parse(const char *s, size_t n, bool complete = false);

    protected:

        Expression() {}
        Expression(const Expression&) {}

        enum ExpressionType type;
    };


    //
    // Base class for handling "X operator Y"-style arithmetic expression
    //
    class BinaryExpression : public Expression
    {
    public:

        BinaryExpression(ExpressionType type, Expression* left_, Expression* right_) 
            : Expression(type), left(left_), right(right_)
        {
        }
        virtual ~BinaryExpression()
        {
            if (left)
            {
                delete left;
            }
            if (right)
            {
                delete right;
            }
        }

    protected:

        BinaryExpression() {}
        BinaryExpression(const BinaryExpression&) {}

        Expression* left;
        Expression* right;
    };


    //
    // Base class for handling "operator X"-style arithmetic expression
    //
    class UnaryExpression : public Expression
    {
    public:

        UnaryExpression(ExpressionType type, Expression* expr_)
            : Expression(type), expr(expr_)
        {
        }
        virtual ~UnaryExpression()
        {
            if (expr)
            {
                delete expr;
            }
        }

    protected:

        UnaryExpression() {}
        UnaryExpression(const UnaryExpression&) {}

        Expression* expr;
    };


    class AddExpression : public BinaryExpression
    {
    public:

        AddExpression(Expression* left, Expression* right)
            : BinaryExpression(ET_ADD, left, right)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        AddExpression(const AddExpression&) {}
    };


    class SubtractExpression : public BinaryExpression
    {
    public:

        SubtractExpression(Expression* left, Expression* right)
            : BinaryExpression(ET_SUBTRACT, left, right)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        SubtractExpression(const SubtractExpression&) {}
    };


    class MultiplyExpression : public BinaryExpression
    {
    public:

        MultiplyExpression(Expression* left, Expression* right)
            : BinaryExpression(ET_MULTIPLY, left, right)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        MultiplyExpression(const MultiplyExpression&) {}
    };


    class DivideExpression : public BinaryExpression
    {
    public:

        DivideExpression(Expression* left, Expression* right)
            : BinaryExpression(ET_DIVIDE, left, right)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        DivideExpression(const DivideExpression&) {}
    };


    class MinusExpression : public UnaryExpression
    {
    public:

        MinusExpression(Expression* expr = NULL)
            : UnaryExpression(ET_UNARY_MINUS, expr)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        MinusExpression(const MinusExpression&) {}
    };


    class Integer : public Expression
    {
    public:

        Integer(long v = 0)
            : Expression(ET_INTEGER), value(v), string()
        {
        }
        Integer(long v, const Glib::ustring& s)
            : Expression(ET_INTEGER), value(v), string(s)
        {
            if (value == LONG_MIN)
            {
                type = ET_INTEGER_MAX_PLUS_ONE;
            }
        }
        Integer(const Integer& other)
            : Expression(other.type), value(other.value), string() // string is not copied to use format flag
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);
        long getValue() const { return value; }

    protected:

        long value;
        Glib::ustring string;
    };


    class RealNumber : public Expression
    {
    public:

        RealNumber(long double v = 0)
            : Expression(ET_REALNUMBER), value(v), string()
        {
        }
        RealNumber(long double v, const Glib::ustring& s)
            : Expression(ET_REALNUMBER), value(v), string(s)
        {
        }
        RealNumber(const RealNumber& other)
            : Expression(other.type), value(other.value), string() // string is not copied to use format flag
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);
        long double getValue() const { return value; }

    protected:

        long double value;
        Glib::ustring string;
    };


    class BlockExpression : public UnaryExpression
    {
    public:

        BlockExpression(Expression* expr = NULL)
            : UnaryExpression(expr ? ET_BLOCK : ET_INCOMPLETE_BLOCK, expr)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);
        void setIncomplete() { type = ET_INCOMPLETE_BLOCK; }

    protected:

        BlockExpression(const BlockExpression&) {}
    };


    class IncompleteExpression : public Expression
    {
    public:

        IncompleteExpression(const Glib::ustring& s)
            : Expression(ET_INCOMPLETE), expr(NULL), string(s)
        {
        }
        IncompleteExpression(Expression* e, const Glib::ustring& s)
            : Expression(ET_INCOMPLETE), expr(e), string(s)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        IncompleteExpression(const IncompleteExpression&) {}

        Expression* expr;
        Glib::ustring string;
    };


    class Variable : public Expression
    {
    public:

        Variable(const Glib::ustring& k)
            : Expression(ET_VARIABLE), key(k)
        {
        }
        Variable(const Variable& other)
            : Expression(other.type), key(other.key)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);
        const Glib::ustring& getKey() const { return key; }

    protected:

        Glib::ustring key;
    };


    class AssignExpression : public Expression
    {
    public:

        AssignExpression(const Glib::ustring& k, Expression* e = NULL)
            : Expression(ET_ASSIGN), key(k), expr(e)
        {
        }
        virtual ~AssignExpression()
        {
            if (expr)
            {
                delete expr;
            }
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        Glib::ustring key;
        Expression* expr;
    };


    class AbsExpression : public UnaryExpression
    {
    public:

        AbsExpression(Expression* expr = NULL)
            : UnaryExpression(ET_CBRT, expr)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        AbsExpression(const AbsExpression&) {}
    };


    class CbrtExpression : public UnaryExpression
    {
    public:

        CbrtExpression(Expression* expr = NULL)
            : UnaryExpression(ET_CBRT, expr)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        CbrtExpression(const CbrtExpression&) {}
    };


    class CosExpression : public UnaryExpression
    {
    public:

        CosExpression(Expression* expr = NULL)
            : UnaryExpression(ET_COS, expr)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        CosExpression(const CosExpression&) {}
    };


    class ExpExpression : public UnaryExpression
    {
    public:

        ExpExpression(Expression* expr = NULL)
            : UnaryExpression(ET_EXP, expr)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        ExpExpression(const ExpExpression&) {}
    };


    class HypotExpression : public BinaryExpression
    {
    public:

        HypotExpression(Expression* left, Expression* right)
            : BinaryExpression(ET_HYPOT, left, right)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        HypotExpression(const HypotExpression&) {}
    };


    class LogExpression : public UnaryExpression
    {
    public:

        LogExpression(Expression* expr = NULL)
            : UnaryExpression(ET_LOG, expr)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        LogExpression(const LogExpression&) {}
    };


    class Log2Expression : public UnaryExpression
    {
    public:

        Log2Expression(Expression* expr = NULL)
            : UnaryExpression(ET_LOG2, expr)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        Log2Expression(const Log2Expression&) {}
    };


    class Log10Expression : public UnaryExpression
    {
    public:

        Log10Expression(Expression* expr = NULL)
            : UnaryExpression(ET_LOG10, expr)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        Log10Expression(const Log10Expression&) {}
    };


    class PowExpression : public BinaryExpression
    {
    public:

        PowExpression(Expression* left, Expression* right)
            : BinaryExpression(ET_POW, left, right)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        PowExpression(const PowExpression&) {}
    };


    class SinExpression : public UnaryExpression
    {
    public:

        SinExpression(Expression* expr = NULL)
            : UnaryExpression(ET_SIN, expr)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        SinExpression(const SinExpression&) {}
    };


    class SqrtExpression : public UnaryExpression
    {
    public:

        SqrtExpression(Expression* expr = NULL)
            : UnaryExpression(ET_SQRT, expr)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        SqrtExpression(const SqrtExpression&) {}
    };


    class TanExpression : public UnaryExpression
    {
    public:

        TanExpression(Expression* expr = NULL)
            : UnaryExpression(ET_TAN, expr)
        {
        }
        virtual void format(std::vector<char> &buffer, int flags);
        virtual Expression* evaluate(bool permanent);

    protected:

        TanExpression(const TanExpression&) {}
    };
}


#endif //!IKURA_EXPRESSION_H
