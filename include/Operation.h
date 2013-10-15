#pragma once

namespace _8b {

enum class UnaryOperation {
    Increment,
    Decrement,
    ArithmeticInversion,
    LogicInversion,
    BooleanConversion,
    IntegerConversion,
    RealConversion,
    PointerConversion
};

enum class BinaryOperation {
    Assignment,
    Addition,
    Subtraction,
    Multiplication,
    Division,
    LogicAnd,
    LogicOr,
    LessComparison,
    LessOrEqualComparison,
    GreaterComparison,
    GreaterOrEqualComparison,
    EqualComparison,
    NotEqualComparison
};

}
