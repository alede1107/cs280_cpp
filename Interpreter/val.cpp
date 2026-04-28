/*
 * val.cpp
 * Value class implementation
 * CS280 - Spring 2026
 * Programming Assignment 3
 */

#include "val_SP26.h"

// ADDITION
Value Value::operator+(const Value& op) const {

    // if either side is an error, return error
    if (IsErr() || op.IsErr()) {
        return Value();
    }

    // string + string = combined string
    if (IsString() && op.IsString()) {
        return Value(GetString() + op.GetString());
    }

    // string + char = combined string
    if (IsString() && op.IsChar()) {
        string result = GetString();
        result += op.GetChar();
        return Value(result);
    }

    // char + string = combined string
    if (IsChar() && op.IsString()) {
        string result = "";
        result += GetChar();
        result += op.GetString();
        return Value(result);
    }

    // char + char = combined string
    if (IsChar() && op.IsChar()) {
        string result = "";
        result += GetChar();
        result += op.GetChar();
        return Value(result);
    }

    // int + int
    if (IsInt() && op.IsInt()) {
        return Value(GetInt() + op.GetInt());
    }

    // real + real
    if (IsReal() && op.IsReal()) {
        return Value(GetReal() + op.GetReal());
    }

    // int + real (convert int to real)
    if (IsInt() && op.IsReal()) {
        return Value((double)GetInt() + op.GetReal());
    }

    // real + int (convert int to real)
    if (IsReal() && op.IsInt()) {
        return Value(GetReal() + (double)op.GetInt());
    }

    // anything else is an error
    return Value();
}

// SUBTRACTION
Value Value::operator-(const Value& op) const {

    if (IsErr() || op.IsErr()) {
        return Value();
    }

    if (IsInt() && op.IsInt()) {
        return Value(GetInt() - op.GetInt());
    }

    if (IsReal() && op.IsReal()) {
        return Value(GetReal() - op.GetReal());
    }

    if (IsInt() && op.IsReal()) {
        return Value((double)GetInt() - op.GetReal());
    }

    if (IsReal() && op.IsInt()) {
        return Value(GetReal() - (double)op.GetInt());
    }

    return Value();
}

// MULTIPLICATION
Value Value::operator*(const Value& op) const {

    if (IsErr() || op.IsErr()) {
        return Value();
    }

    if (IsInt() && op.IsInt()) {
        return Value(GetInt() * op.GetInt());
    }

    if (IsReal() && op.IsReal()) {
        return Value(GetReal() * op.GetReal());
    }

    if (IsInt() && op.IsReal()) {
        return Value((double)GetInt() * op.GetReal());
    }

    if (IsReal() && op.IsInt()) {
        return Value(GetReal() * (double)op.GetInt());
    }

    return Value();
}

// DIVISION (always gives back a real number)
Value Value::operator/(const Value& op) const {

    if (IsErr() || op.IsErr()) {
        return Value();
    }

    // check for division by zero
    if (op.IsInt() && op.GetInt() == 0) {
        return Value();
    }
    if (op.IsReal() && op.GetReal() == 0.0) {
        return Value();
    }

    // both sides must be numbers
    if (!IsInt() && !IsReal()) {
        return Value();
    }
    if (!op.IsInt() && !op.IsReal()) {
        return Value();
    }

    // convert both to double and divide
    double left = IsInt() ? (double)GetInt() : GetReal();
    double right = op.IsInt() ? (double)op.GetInt() : op.GetReal();

    return Value(left / right);
}

// INTEGER DIVISION (DIV) - both must be integers
Value Value::idiv(const Value& op) const {

    if (IsErr() || op.IsErr()) {
        return Value();
    }

    // both operands must be integers
    if (!IsInt() || !op.IsInt()) {
        return Value();
    }

    // check for division by zero
    if (op.GetInt() == 0) {
        return Value();
    }

    return Value(GetInt() / op.GetInt());
}

// MODULO (MOD) - both must be integers
Value Value::operator%(const Value& op) const {

    if (IsErr() || op.IsErr()) {
        return Value();
    }

    // both operands must be integers
    if (!IsInt() || !op.IsInt()) {
        return Value();
    }

    // check for division by zero
    if (op.GetInt() == 0) {
        return Value();
    }

    return Value(GetInt() % op.GetInt());
}

// EQUAL TO (=)
Value Value::operator==(const Value& op) const {

    if (IsErr() || op.IsErr()) {
        return Value();
    }

    if (IsInt() && op.IsInt()) {
        return Value(GetInt() == op.GetInt());
    }

    if (IsReal() && op.IsReal()) {
        return Value(GetReal() == op.GetReal());
    }

    if (IsInt() && op.IsReal()) {
        return Value((double)GetInt() == op.GetReal());
    }

    if (IsReal() && op.IsInt()) {
        return Value(GetReal() == (double)op.GetInt());
    }

    if (IsString() && op.IsString()) {
        return Value(GetString() == op.GetString());
    }

    if (IsChar() && op.IsChar()) {
        return Value(GetChar() == op.GetChar());
    }

    // string vs char comparison
    if (IsString() && op.IsChar()) {
        string s = "";
        s += op.GetChar();
        return Value(GetString() == s);
    }

    if (IsChar() && op.IsString()) {
        string s = "";
        s += GetChar();
        return Value(s == op.GetString());
    }

    if (IsBool() && op.IsBool()) {
        return Value(GetBool() == op.GetBool());
    }

    return Value();
}

// GREATER THAN (>)
Value Value::operator>(const Value& op) const {

    if (IsErr() || op.IsErr()) {
        return Value();
    }

    if (IsInt() && op.IsInt()) {
        return Value(GetInt() > op.GetInt());
    }

    if (IsReal() && op.IsReal()) {
        return Value(GetReal() > op.GetReal());
    }

    if (IsInt() && op.IsReal()) {
        return Value((double)GetInt() > op.GetReal());
    }

    if (IsReal() && op.IsInt()) {
        return Value(GetReal() > (double)op.GetInt());
    }

    if (IsString() && op.IsString()) {
        return Value(GetString() > op.GetString());
    }

    if (IsChar() && op.IsChar()) {
        return Value(GetChar() > op.GetChar());
    }

    // string vs char comparison
    if (IsString() && op.IsChar()) {
        string s = "";
        s += op.GetChar();
        return Value(GetString() > s);
    }

    if (IsChar() && op.IsString()) {
        string s = "";
        s += GetChar();
        return Value(s > op.GetString());
    }

    return Value();
}

// LESS THAN (<)
Value Value::operator<(const Value& op) const {

    if (IsErr() || op.IsErr()) {
        return Value();
    }

    if (IsInt() && op.IsInt()) {
        return Value(GetInt() < op.GetInt());
    }

    if (IsReal() && op.IsReal()) {
        return Value(GetReal() < op.GetReal());
    }

    if (IsInt() && op.IsReal()) {
        return Value((double)GetInt() < op.GetReal());
    }

    if (IsReal() && op.IsInt()) {
        return Value(GetReal() < (double)op.GetInt());
    }

    if (IsString() && op.IsString()) {
        return Value(GetString() < op.GetString());
    }

    if (IsChar() && op.IsChar()) {
        return Value(GetChar() < op.GetChar());
    }

    // string vs char comparison
    if (IsString() && op.IsChar()) {
        string s = "";
        s += op.GetChar();
        return Value(GetString() < s);
    }

    if (IsChar() && op.IsString()) {
        string s = "";
        s += GetChar();
        return Value(s < op.GetString());
    }

    return Value();
}

// LOGICAL AND
Value Value::operator&&(const Value& op) const {

    if (IsErr() || op.IsErr()) {
        return Value();
    }

    // both sides must be boolean
    if (!IsBool() || !op.IsBool()) {
        return Value();
    }

    return Value(GetBool() && op.GetBool());
}

// LOGICAL OR
Value Value::operator||(const Value& op) const {

    if (IsErr() || op.IsErr()) {
        return Value();
    }

    // both sides must be boolean
    if (!IsBool() || !op.IsBool()) {
        return Value();
    }

    return Value(GetBool() || op.GetBool());
}

// LOGICAL NOT
Value Value::operator!() const {

    if (IsErr()) {
        return Value();
    }

    // must be boolean
    if (!IsBool()) {
        return Value();
    }

    return Value(!GetBool());
}

// UNARY MINUS (negative sign or trim for strings)
Value Value::operator-() const {

    if (IsErr()) {
        return Value();
    }

    // negative number
    if (IsInt()) {
        return Value(-GetInt());
    }

    if (IsReal()) {
        return Value(-GetReal());
    }

    // for string or char: trim trailing spaces
    if (IsString() || IsChar()) {
        return Trim();
    }

    return Value();
}

// TRIM (removes trailing spaces from a string or char)
Value Value::Trim() const {

    if (IsErr()) {
        return Value();
    }

    if (IsString()) {
        string s = GetString();

        // find the last character that is NOT a space
        int last = (int)s.size() - 1;
        while (last >= 0 && s[last] == ' ') {
            last--;
        }

        // if all spaces, return empty string
        if (last < 0) {
            return Value(string(""));
        }

        return Value(s.substr(0, last + 1));
    }

    if (IsChar()) {
        char c = GetChar();

        // if the char is a space, return empty string
        if (c == ' ') {
            return Value(string(""));
        }

        // otherwise return it as a one-character string
        string result = "";
        result += c;
        return Value(result);
    }

    return Value();
}