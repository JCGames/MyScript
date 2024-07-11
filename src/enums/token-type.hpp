#ifndef TOKEN_TYPE
#define TOKEN_TYPE

/**
 * There is already an enum value with the name TokenType
 * Thus, conflicts... gotta go with _TokenType
 */
enum class _TokenType
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
    NAMESPACE,
    WHILE,
    _TRUE,
    _FALSE,

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

std::string token_type_name(_TokenType type)
{
    switch (type)
    {
        case _TokenType::END_OF_FILE: return "END_OF_FILE";
        case _TokenType::END_OF_LINE: return "END_OF_LINE";
        case _TokenType::SYMBOL: return "SYMBOL";
        case _TokenType::STRING: return "STRING";
        case _TokenType::NUMBER: return "NUMBER";
        case _TokenType::FUNCITON: return "FUNCITON";
        case _TokenType::RETURN: return "RETURN";
        case _TokenType::STRUCT: return "STRUCT";
        case _TokenType::IF: return "IF";
        case _TokenType::ELSE: return "ELSE";
        case _TokenType::_NULL: return "_NULL";
        case _TokenType::NAMESPACE: return "NAMESPACE";
        case _TokenType::WHILE: return "WHILE";
        case _TokenType::_TRUE: return "_TRUE";
        case _TokenType::_FALSE: return "_FALSE";

        case _TokenType::PLUS: return "PLUS";
        case _TokenType::MINUS: return "MINUS";
        case _TokenType::MULTIPLY: return "MULTIPLY";
        case _TokenType::DIVIDE: return "DIVIDE";
        case _TokenType::MODULUS: return "MODULUS";
        case _TokenType::ASSIGN: return "ASSIGN";

        case _TokenType::EQUALS: return "EQUALS";
        case _TokenType::NOT_EQUALS: return "NOT_EQUALS";
        case _TokenType::GREATER_THAN: return "GREATER_THAN";
        case _TokenType::LESS_THAN: return "LESS_THAN";
        case _TokenType::GREATER_THAN_E: return "GREATER_THAN_E";
        case _TokenType::LESS_THAN_E: return "LESS_THAN_E";

        case _TokenType::AND: return "AND";
        case _TokenType::OR: return "OR";

        case _TokenType::OPEN_PARAN: return "OPEN_PARAN";
        case _TokenType::CLOSE_PARAN: return "CLOSE_PARAN";
        case _TokenType::OPEN_BRACKET: return "OPEN_BRACKET";
        case _TokenType::CLOSE_BRACKET: return "CLOSE_BRACKET";
        case _TokenType::COMMA: return "COMMA";
        case _TokenType::DOT: return "DOT";
        default: return "That was not a token type";
    }
}

#endif