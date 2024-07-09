#ifndef STATEMENT_TYPE
#define STATEMENT_TYPE

#include <string>

enum class StatementType
{
    VALUE,
    EXPRESSION,
    BINARY_OPERATOR,
    UNARY_OPERATOR,
    SYMBOL,
    BLOCK,
    FUNCTION,
    RETURN,
    FUNCTION_CALL,
    STRUCT,
    IF,
    ELSE,
    NAMESPACE
};

std::string statement_type_name(StatementType type)
{
    switch (type)
    {
        case StatementType::VALUE: return "VALUE";
        case StatementType::EXPRESSION: return "EXPRESSION";
        case StatementType::BINARY_OPERATOR: return "BINARY_OPERATOR";
        case StatementType::UNARY_OPERATOR: return "UNARY_OPERATOR";
        case StatementType::SYMBOL: return "SYMBOL";
        case StatementType::BLOCK: return "BLOCK";
        case StatementType::FUNCTION: return "FUNCTION";
        case StatementType::RETURN: return "RETURN";
        case StatementType::FUNCTION_CALL: return "FUNCTION_CALL";
        case StatementType::STRUCT: return "STRUCT";
        case StatementType::IF: return "IF";
        case StatementType::ELSE: return "ELSE";
        case StatementType::NAMESPACE: return "NAMESPACE";
        default: return "That was not a statement type";
    }
}

#endif