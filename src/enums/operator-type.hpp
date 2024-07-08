#ifndef OPERATOR_TYPE
#define OPERATOR_TYPE

#include <string>

enum class OperatorType
{
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    MODULUS,
    ASSIGNMENT,
    NEGATE,
    MEMBER_ACCESSOR,

    EQUALS,
    NOT_EQUALS,
    GREATER_THAN,
    LESS_THAN,
    GREATER_THAN_E,
    LESS_THAN_E,

    AND,
    OR
};

std::string operator_type_name(OperatorType type)
{
    switch (type)
    {
        case OperatorType::ADDITION: return "ADDITION";
        case OperatorType::SUBTRACTION: return "SUBTRACTION";
        case OperatorType::MULTIPLICATION: return "MULTIPLICATION";
        case OperatorType::DIVISION: return "DIVISION";
        case OperatorType::MODULUS: return "MODULUS";
        case OperatorType::ASSIGNMENT: return "ASSIGNMENT";
        case OperatorType::NEGATE: return "NEGATE";
        case OperatorType::MEMBER_ACCESSOR: return "MEMBER_ACCESSOR";

        case OperatorType::EQUALS: return "EQUALS";
        case OperatorType::NOT_EQUALS: return "NOT_EQUALS";
        case OperatorType::GREATER_THAN: return "GREATER_THAN";
        case OperatorType::LESS_THAN: return "LESS_THAN";
        case OperatorType::GREATER_THAN_E: return "GREATER_THAN_E";
        case OperatorType::LESS_THAN_E: return "LESS_THAN_E";

        case OperatorType::AND: return "AND";
        case OperatorType::OR: return "OR";
        default: return "That was not a statement type";
    }
}

#endif