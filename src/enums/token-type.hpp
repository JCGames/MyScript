#ifndef TOKEN_TYPE
#define TOKEN_TYPE

#include <string>

enum class TokenType
{
    END_OF_FILE,
    END_OF_LINE,
    SYMBOL,
    STRING,
    NUMBER,
    FUNCITON,
    RETURN,
    STRUCT,
    IF,
    ELSE,
    _NULL,

    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULUS,
    ASSIGN,

    EQUALS,
    NOT_EQUALS,
    GREATER_THAN,
    LESS_THAN,
    GREATER_THAN_E,
    LESS_THAN_E,

    AND,
    OR,
    
    OPEN_PARAN,
    CLOSE_PARAN,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    COMMA,
    DOT
};

std::string token_type_name(TokenType type)
{
    switch (type)
    {
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        case TokenType::END_OF_LINE: return "END_OF_LINE";
        case TokenType::SYMBOL: return "SYMBOL";
        case TokenType::STRING: return "STRING";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::FUNCITON: return "FUNCITON";
        case TokenType::RETURN: return "RETURN";
        case TokenType::STRUCT: return "STRUCT";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::_NULL: return "_NULL";

        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MULTIPLY: return "MULTIPLY";
        case TokenType::DIVIDE: return "DIVIDE";
        case TokenType::MODULUS: return "MODULUS";
        case TokenType::ASSIGN: return "ASSIGN";

        case TokenType::EQUALS: return "EQUALS";
        case TokenType::NOT_EQUALS: return "NOT_EQUALS";
        case TokenType::GREATER_THAN: return "GREATER_THAN";
        case TokenType::LESS_THAN: return "LESS_THAN";
        case TokenType::GREATER_THAN_E: return "GREATER_THAN_E";
        case TokenType::LESS_THAN_E: return "LESS_THAN_E";

        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";

        case TokenType::OPEN_PARAN: return "OPEN_PARAN";
        case TokenType::CLOSE_PARAN: return "CLOSE_PARAN";
        case TokenType::OPEN_BRACKET: return "OPEN_BRACKET";
        case TokenType::CLOSE_BRACKET: return "CLOSE_BRACKET";
        case TokenType::COMMA: return "COMMA";
        case TokenType::DOT: return "DOT";
        default: return "That was not a token type";
    }
}

#endif