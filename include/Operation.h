#pragma once

namespace _8b {
    
enum class UnaryOperation {
    Increment,
    Decrement,
    ArithmeticInversion,
    LogicInversion,
    BooleanConversion,
    IntegerConversion,
    Addressing
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
    GreaterComparison
};

}
