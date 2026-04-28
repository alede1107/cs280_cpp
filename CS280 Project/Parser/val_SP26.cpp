#include "val_SP26.h"

// operator+: numeric addition OR string/char concatenation
Value Value::operator+(const Value& op) const {
    // String/char concatenation
    if ((IsString() || IsChar()) && (op.IsString() || op.IsChar())) {
        string left  = IsString()    ? GetString() : string(1, GetChar());
        string right = op.IsString() ? op.GetString() : string(1, op.GetChar());
        return Value(left + right);
    }

    // Numeric addition
    if (IsInt() && op.IsInt())
        return Value(Itemp + op.Itemp);

    if (IsReal() && op.IsReal())
        return Value(Rtemp + op.Rtemp);

    if (IsInt() && op.IsReal())
        return Value((double)Itemp + op.Rtemp);

    if (IsReal() && op.IsInt())
        return Value(Rtemp + (double)op.Itemp);

    // Incompatible types
    return Value();
}

// operator-: numeric subtraction (binary)
Value Value::operator-(const Value& op) const {
    if (IsInt() && op.IsInt())
        return Value(Itemp - op.Itemp);

    if (IsReal() && op.IsReal())
        return Value(Rtemp - op.Rtemp);

    if (IsInt() && op.IsReal())
        return Value((double)Itemp - op.Rtemp);

    if (IsReal() && op.IsInt())
        return Value(Rtemp - (double)op.Itemp);

    return Value();
}

// operator*: numeric multiplication
Value Value::operator*(const Value& op) const {
    if (IsInt() && op.IsInt())
        return Value(Itemp * op.Itemp);

    if (IsReal() && op.IsReal())
        return Value(Rtemp * op.Rtemp);

    if (IsInt() && op.IsReal())
        return Value((double)Itemp * op.Rtemp);

    if (IsReal() && op.IsInt())
        return Value(Rtemp * (double)op.Itemp);

    return Value();
}

// operator/: numeric division — always produces a real result
Value Value::operator/(const Value& op) const {
    double divisor = 0.0;
    double dividend = 0.0;

    if (op.IsInt())       divisor = (double)op.Itemp;
    else if (op.IsReal()) divisor = op.Rtemp;
    else return Value();

    if (IsInt())       dividend = (double)Itemp;
    else if (IsReal()) dividend = Rtemp;
    else return Value();

    if (divisor == 0.0)
        return Value();  // division by zero => error

    return Value(dividend / divisor);
}

// idiv: integer division (both operands must be int)
Value Value::idiv(const Value& oper) const {
    if (!IsInt() || !oper.IsInt())
        return Value();

    if (oper.Itemp == 0)
        return Value();  // division by zero => error

    return Value(Itemp / oper.Itemp);
}

// operator%: modulus (both operands must be int)
Value Value::operator%(const Value& op) const {
    if (!IsInt() || !op.IsInt())
        return Value();

    if (op.Itemp == 0)
        return Value();  // modulus by zero => error

    return Value(Itemp % op.Itemp);
}

// operator==: equality
Value Value::operator==(const Value& op) const {
    if (IsInt() && op.IsInt())
        return Value(Itemp == op.Itemp);

    if (IsReal() && op.IsReal())
        return Value(Rtemp == op.Rtemp);

    if (IsInt() && op.IsReal())
        return Value((double)Itemp == op.Rtemp);

    if (IsReal() && op.IsInt())
        return Value(Rtemp == (double)op.Itemp);

    if (IsString() && op.IsString())
        return Value(Stemp == op.Stemp);

    if (IsChar() && op.IsChar())
        return Value(Ctemp == op.Ctemp);

    if (IsBool() && op.IsBool())
        return Value(Btemp == op.Btemp);

    return Value();
}

// operator<: less than
Value Value::operator<(const Value& op) const {
    if (IsInt() && op.IsInt())
        return Value(Itemp < op.Itemp);

    if (IsReal() && op.IsReal())
        return Value(Rtemp < op.Rtemp);

    if (IsInt() && op.IsReal())
        return Value((double)Itemp < op.Rtemp);

    if (IsReal() && op.IsInt())
        return Value(Rtemp < (double)op.Itemp);

    if (IsString() && op.IsString())
        return Value(Stemp < op.Stemp);

    if (IsChar() && op.IsChar())
        return Value(Ctemp < op.Ctemp);

    return Value();
}

// operator>: greater than
Value Value::operator>(const Value& op) const {
    if (IsInt() && op.IsInt())
        return Value(Itemp > op.Itemp);

    if (IsReal() && op.IsReal())
        return Value(Rtemp > op.Rtemp);

    if (IsInt() && op.IsReal())
        return Value((double)Itemp > op.Rtemp);

    if (IsReal() && op.IsInt())
        return Value(Rtemp > (double)op.Itemp);

    if (IsString() && op.IsString())
        return Value(Stemp > op.Stemp);

    if (IsChar() && op.IsChar())
        return Value(Ctemp > op.Ctemp);

    return Value();
}

// operator&&: logical AND (both operands must be bool)
Value Value::operator&&(const Value& op) const {
    if (!IsBool() || !op.IsBool())
        return Value();

    return Value(Btemp && op.Btemp);
}

// operator||: logical OR (both operands must be bool)
Value Value::operator||(const Value& op) const {
    if (!IsBool() || !op.IsBool())
        return Value();

    return Value(Btemp || op.Btemp);
}

// operator!: logical NOT (operand must be bool)
Value Value::operator!(void) const {
    if (!IsBool())
        return Value();

    return Value(!Btemp);
}

// unary minus: negation of numeric value; Trim for string/char
Value Value::operator-(void) const {
    if (IsInt())
        return Value(-Itemp);

    if (IsReal())
        return Value(-Rtemp);

    if (IsString() || IsChar())
        return Trim();

    return Value();
}

// Trim: remove trailing blanks from string or char operand
Value Value::Trim(void) const {
    if (IsString()) {
        string s = Stemp;
        size_t last = s.find_last_not_of(' ');
        if (last == string::npos)
            return Value(string(""));
        return Value(s.substr(0, last + 1));
    }

    if (IsChar()) {
        if (Ctemp == ' ')
            return Value(string(""));
        return Value(string(1, Ctemp));
    }

    return Value();
}
