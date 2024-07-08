#ifndef COMPILER
#define COMPILER

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stack>
#include <cstring>
#include <map>
#include <sstream>

// can save space in binary file to remove debug code
#define DEBUG

#define ERROR(msg) logger_error(msg, __FILE__, __LINE__)
#define CAST(value, type) static_cast<type*>(value)

using std::vector;
using std::string;
using std::ifstream;
using std::stack;
using std::map;

const string EMPTY_STRING = ""; 

void logger_error(string msg, const char* file, int line)
{
    std::cout << msg << " [" << file << ":" << line << "]" << std::endl;
    exit(1);
}

// =====================
// |       LEXER       |
// =====================

#pragma region Lexer

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

string token_type_name(TokenType type)
{
    // can save space in binary file to remove debug code
    #ifdef DEBUG
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
        default: ERROR("That was not a token type");
    }

    return EMPTY_STRING;
    #else
    return std::to_string((int)type);
    #endif
}

struct Token
{
    TokenType type;
    string value;
    unsigned int line;

    Token(TokenType type, string value, unsigned int line)
    {
        this->type = type;
        this->value = value;
        this->line = line;
    }
};

vector<Token*> tokens;

bool string_replace(std::string& str, const std::string& from, const std::string& to) 
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void lexer_lexify(string fileName)
{
    ifstream ifs(fileName);

    if (!ifs.is_open())
        ERROR("Was unable to open file with name " + fileName + ".");

    unsigned int line = 0;
    char c;

    // tokenize the entire file
    while (!ifs.eof())
    {
        c = ifs.get();

        if (c == '/' && ifs.peek() == '/')
        {
            while (!ifs.eof() && ifs.peek() != '\n')
                ifs.get();

            continue;
        }

        // END_OF_LINE
        if (c == '\n')
        {
            auto token = new Token(TokenType::END_OF_LINE, EMPTY_STRING, line);
            ++line;
            tokens.push_back(token);
            continue;
        }
        // SYMBOL
        else if (isalpha(c) || c == '_')
        {
            string symbol = string(1, c);
            char peek;

            while (true)
            {
                // only want to have to call ifs.peek once rather than twice
                peek = ifs.peek(); 

                if (!isalnum(peek) && peek != '_')
                    break;

                symbol += ifs.get();
            }

            if (symbol == "fn")
            {
                tokens.push_back(new Token(TokenType::FUNCITON, symbol, line));
                continue;
            }
            else if (symbol == "return")
            {
                tokens.push_back(new Token(TokenType::RETURN, symbol, line));
                continue;
            }
            else if (symbol == "struct")
            {
                tokens.push_back(new Token(TokenType::STRUCT, symbol, line));
                continue;
            }
            else if (symbol == "if")
            {
                tokens.push_back(new Token(TokenType::IF, symbol, line));
                continue;
            }
            else if (symbol == "else")
            {
                tokens.push_back(new Token(TokenType::ELSE, symbol, line));
                continue;
            }
            else if (symbol == "and")
            {
                tokens.push_back(new Token(TokenType::AND, symbol, line));
                continue;
            }
            else if (symbol == "or")
            {
                tokens.push_back(new Token(TokenType::OR, symbol, line));
                continue;
            }
            else if (symbol == "null")
            {
                tokens.push_back(new Token(TokenType::_NULL, symbol, line));
                continue;
            }

            tokens.push_back(new Token(TokenType::SYMBOL, symbol, line));
            continue;
        }
        // STRING
        else if (c == '"')
        {
            string str = EMPTY_STRING;
            unsigned int beginLine = line;

            while (ifs.peek() != '"' && !ifs.eof())
                str += ifs.get();

            c = ifs.get();

            if (c != '"')
                ERROR("Missing a closing quotes on line " + std::to_string(beginLine + 1) + ".");

            string_replace(str, "\\n", "\n");

            tokens.push_back(new Token(TokenType::STRING, str, line));
            continue;
        }
        // NUMBER
        else if (isdigit(c) || (c == '.' && isdigit(ifs.peek())))
        {
            string number = string(1, c);
            char peek;
            bool hasDecimalPoint = false;

            while (true)
            {
                peek = ifs.peek();

                if (!isdigit(peek) && peek != '.')
                    break;

                c = ifs.get(); 

                if (c == '.' && !hasDecimalPoint)
                {
                    hasDecimalPoint = true;
                } 
                else if (c == '.' && hasDecimalPoint)
                {
                    ERROR("Number on line " + std::to_string(line + 1) + " has too many decimal points.");
                }

                number += c;
            }

            tokens.push_back(new Token(TokenType::NUMBER, number, line));
            continue;
        }
        // DOUBLE OPERATORS
        else if (c == '-' && ifs.peek() == '>')
        {
            char next = ifs.get();
            tokens.push_back(new Token(TokenType::ASSIGN, string(1, c) + string(1, next), line));
        }
        else if (c == '!' && ifs.peek() == '=')
        {
            char next = ifs.get();
            tokens.push_back(new Token(TokenType::NOT_EQUALS, string(1, c) + string(1, next), line));
        }
        else if (c == '>' && ifs.peek() == '=')
        {
            char next = ifs.get();
            tokens.push_back(new Token(TokenType::GREATER_THAN_E, string(1, c) + string(1, next), line));
        }
        else if (c == '<' && ifs.peek() == '=')
        {
            char next = ifs.get();
            tokens.push_back(new Token(TokenType::LESS_THAN_E, string(1, c) + string(1, next), line));
        }
        // SINGLE CHARACTER OPERATORS
        else if (c == '=')
        {
            tokens.push_back(new Token(TokenType::EQUALS, string(1, c), line));
        }
        else if (c == '>')
        {
            tokens.push_back(new Token(TokenType::GREATER_THAN, string(1, c), line));
        }
        else if (c == '<')
        {
            tokens.push_back(new Token(TokenType::LESS_THAN, string(1, c), line));
        }
        else if (c == '+')
        {
            tokens.push_back(new Token(TokenType::PLUS, string(1, c), line));
        }
        else if (c == '-')
        {
            tokens.push_back(new Token(TokenType::MINUS, string(1, c), line));
        }
        else if (c == '*')
        {
            tokens.push_back(new Token(TokenType::MULTIPLY, string(1, c), line));
        }
        else if (c == '/')
        {
            tokens.push_back(new Token(TokenType::DIVIDE, string(1, c), line));
        }
        else if (c == '%')
        {
            tokens.push_back(new Token(TokenType::MODULUS, string(1, c), line));
        }
        else if (c == '(')
        {
            tokens.push_back(new Token(TokenType::OPEN_PARAN, string(1, c), line));
        }
        else if (c == ')')
        {
            tokens.push_back(new Token(TokenType::CLOSE_PARAN, string(1, c), line));
        }
        else if (c == '{')
        {
            tokens.push_back(new Token(TokenType::OPEN_BRACKET, string(1, c), line));
        }
        else if (c == '}')
        {
            tokens.push_back(new Token(TokenType::CLOSE_BRACKET, string(1, c), line));
        }
        else if (c == ',')
        {
            tokens.push_back(new Token(TokenType::COMMA, string(1, c), line));
        }
        else if (c == '.')
        {
            tokens.push_back(new Token(TokenType::DOT, string(1, c), line));
        }
    }

    tokens.push_back(new Token(TokenType::END_OF_FILE, EMPTY_STRING, line));

    ifs.close();
}

void lexer_print_tokens()
{
    for (const auto token : tokens)
    {
        std::cout << "Type: " << token_type_name(token->type) << " Value: |" << token->value << "| Line: " << token->line + 1 << std::endl;
    }
}

void lexer_delete_tokens()
{
    for (auto token : tokens)
    {
        if (token != nullptr)
            delete token;
    }
}

#pragma endregion

// =======================
// |       PARSER        |
// =======================

#pragma region Parser

#pragma region Enums

enum class DataType
{
    STRING,
    FLOAT,
    INT,
    BOOL,
    _NULL
};

string data_type_name(DataType type)
{
    switch (type)
    {
        case DataType::STRING: return "STRING";
        case DataType::FLOAT: return "FLOAT";
        case DataType::INT: return "INT";
        case DataType::BOOL: return "BOOL";
        case DataType::_NULL: return "_NULL";
        default: ERROR("That was not a token type");
    }

    return EMPTY_STRING;
}

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
    ELSE
};

string statement_type_name(StatementType type)
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
        default: ERROR("That was not a statement type");
    }

    return EMPTY_STRING;
}

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

string operator_type_name(OperatorType type)
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
        default: ERROR("That was not a statement type");
    }

    return EMPTY_STRING;
}

#pragma endregion

#pragma region Statements

struct Statement
{
    StatementType type;

    Statement(const StatementType& type)
    {
        this->type = type;
    }

    virtual ~Statement() { };

    virtual void print(string padding)
    {
        print_type(padding);
    }

protected:
    void print_type(string padding)
    {
        std::cout << padding << statement_type_name(type) << std::endl;
    }
};

struct StatementExpression : Statement
{
    Statement* root;

    StatementExpression(Statement* root) : Statement(StatementType::EXPRESSION)
    {
        this->root = root;
    };

    ~StatementExpression() override
    {
        delete root;
    }

    void print(string padding) override
    {
        print_type(padding);
        root->print(padding + '\t');
    }
};

struct StatementBinaryOperator : Statement
{
    OperatorType operatorType;
    Statement* left;
    Statement* right;

    StatementBinaryOperator(const OperatorType& operatorType, Statement* left, Statement* right) : Statement(StatementType::BINARY_OPERATOR)
    {
        this->operatorType = operatorType;
        this->left = left;
        this->right = right;
    }

    ~StatementBinaryOperator() override
    {
        delete left;
        delete right;
    }

    void print(string padding) override
    {
        print_type(padding);

        std::cout << padding << operator_type_name(operatorType) << std::endl;

        std::cout << padding << "LEFT:" << std::endl;
        left->print(padding + '\t');
        std::cout << padding << "RIGHT:" << std::endl;
        right->print(padding + '\t');
    }
};

struct StatementUnaryOperator : Statement
{
    OperatorType operatorType;
    Statement* root;

    StatementUnaryOperator(const OperatorType& operatorType, Statement* root) : Statement(StatementType::UNARY_OPERATOR)
    {
        this->operatorType = operatorType;
        this->root = root;
    }

    ~StatementUnaryOperator() override
    {
        delete root;
    }

    void print(string padding) override
    {
        print_type(padding);

        std::cout << padding << operator_type_name(operatorType) << std::endl;

        root->print(padding + '\t');
    }
};

struct StatementValue : Statement
{
    DataType dataType;
    string value;

    StatementValue(const DataType& dataType, const string& value) : Statement(StatementType::VALUE)
    {
        this->dataType = dataType;
        this->value = value;
    }

    void print(string padding) override
    {
        print_type(padding);
        std::cout << padding << data_type_name(dataType) << " " << value << std::endl;
    }
};

struct StatementSymbol : Statement
{
    string symbol;

    StatementSymbol(const string& symbol) : Statement(StatementType::SYMBOL)
    {
        this->symbol = symbol;
    }

    void print(string padding) override
    {
        print_type(padding);
        std::cout << padding << symbol << std::endl;
    }
};

struct StatementBlock : Statement
{
    vector<Statement*> statements;

    StatementBlock() : Statement(StatementType::BLOCK) { }

    ~StatementBlock() override
    {
        for (auto& stmt : statements)
            delete stmt;
    }

    void print(string padding) override
    {
        print_type(padding);
        std::cout << padding << "[" << std::endl;

        for (auto stmt : statements)
            stmt->print(padding + '\t');

        std::cout << padding << "]" << std::endl;
    }
};

struct StatementFunction : Statement
{
    StatementSymbol* name;
    vector<StatementSymbol*> params;
    StatementBlock* body;

    StatementFunction(StatementSymbol* name) : Statement(StatementType::FUNCTION) 
    {
        this->name = name;
    }

    ~StatementFunction() override
    {
        delete name;
        
        for (auto& stmt : params)
            delete stmt;
        
        delete body;
    }

    void print(string padding) override
    {
        print_type(padding);
        if (name != nullptr)
            std::cout << padding << "Name: " << name->symbol << std::endl;

        std::cout << padding << "Params: [" << std::endl;

        for (auto& stmtSymbol : params)
            std::cout << padding << '\t' << stmtSymbol->symbol << std::endl;

        std::cout << padding << "]" << std::endl;
        std::cout << padding << "Function Block:" << std::endl;

        if (body != nullptr)
            body->print(padding + '\t');
    }
};

struct StatementReturn : Statement
{
    StatementExpression* expression;

    StatementReturn(StatementExpression* expression) : Statement(StatementType::RETURN)
    {
        this->expression = expression;
    }

    ~StatementReturn()
    {
        delete expression;
    }

    void print(string padding) override
    {
        print_type(padding);
        expression->print(padding + '\t');
    }
};

struct StatementFunctionCall : Statement
{
    StatementSymbol* name;
    vector<StatementExpression*> argExpressions;

    StatementFunctionCall(StatementSymbol* name) : Statement(StatementType::FUNCTION_CALL)
    {
        this->name = name;
    }

    ~StatementFunctionCall()
    {
        delete this->name;

        for (auto& stmt : argExpressions)
            delete stmt;
    }

    void print(string padding) override
    {
        print_type(padding);

        if (this->name != nullptr)
            std::cout << padding << "Name: " << this->name->symbol << std::endl;
        std::cout << padding << "Arg Expressions: [" << std::endl;

        for (auto& stmt : argExpressions)
            stmt->print(padding + '\t');

        std::cout << padding << "]" << std::endl; 
    }
};

struct StatementStruct : Statement
{
    StatementSymbol* name;
    vector<StatementExpression*> variables;
    vector<StatementFunction*> functions;

    StatementStruct(StatementSymbol* name) : Statement(StatementType::STRUCT)
    {
        this->name = name;
    }

    ~StatementStruct()
    {
        delete this->name;

        for (auto& stmt : variables)
            delete stmt;

        for (auto& stmt : functions)
            delete stmt;
    }

    void print(string padding) override
    {
        print_type(padding);

        if (this->name != nullptr)
            std::cout << padding << "Name: " << this->name->symbol << std::endl;
        std::cout << padding << "Variables: [" << std::endl;

        for (auto& stmt : variables)
            stmt->print(padding + '\t');

        std::cout << padding << "]" << std::endl;
        std::cout << padding << "Functions: [" << std::endl;

        for (auto& stmt : functions)
            stmt->print(padding + '\t');

        std::cout << padding << "]" << std::endl;
    }
};

struct StatementElse : Statement
{
    StatementBlock* block;

    StatementElse(StatementBlock* block) : Statement(StatementType::ELSE)
    {
        this->block = block;
    }

    ~StatementElse()
    {
        delete this->block;
    }

    void print(string padding) override
    {
        print_type(padding);
        block->print(padding + '\t');
    }
};

struct StatementIf : Statement
{
    StatementExpression* condition;
    StatementBlock* block;
    StatementIf* _elseIf;
    StatementElse* _else;

    StatementIf(StatementExpression* condition, StatementBlock* block, StatementIf* _elseIf, StatementElse* _else) : Statement(StatementType::IF)
    {
        this->condition = condition;
        this->block = block;
        this->_elseIf = _elseIf;
        this->_else = _else;
    }

    ~StatementIf()
    {
        delete this->block;

        if (this->_elseIf != nullptr)
            delete this->_elseIf;

        if (this->_else != nullptr)
            delete this->_else;
    }

    void print(string padding) override
    {
        print_type(padding);

        std::cout << padding << "Condition: [" << std::endl;

        condition->print(padding + '\t');

        std::cout << padding << "]" << std::endl;

        block->print(padding + '\t');

        if (this->_elseIf != nullptr)
            _elseIf->print(padding + '\t');
        
        if (this->_else != nullptr)
            _else->print(padding + '\t');
    }
};

#pragma endregion

StatementBlock* parserAstRoot = nullptr;
unsigned int parserPosition = 0;

/// @brief Moves to the next token in the list of tokens.
void parser_move_next_token()
{
    if (parserPosition + 1 < tokens.size())
        ++parserPosition;
}

/// @brief Gets the token at the current parserPosition.
const Token* parser_get_token()
{
    return tokens[parserPosition];
}

/// @brief Peeks at the next token one parserPosition further than the current parserPosition. 
const Token* parser_peek_token()
{
    if (parserPosition + 1 < tokens.size())
        return tokens[parserPosition + 1];
    
    return tokens[tokens.size() - 1];
}

void parser_move_next_token_skip_eols()
{
    parser_move_next_token();

    while (parser_get_token()->type == TokenType::END_OF_LINE)
        parser_move_next_token();
}

#pragma region Expressions

StatementExpression* parser_parse_exp();
StatementFunctionCall* parser_parse_function_call();

Statement* parser_parse_exp_term()
{
    Statement* result = nullptr;

    // FUNCTION CALL
    if (parser_get_token()->type == TokenType::SYMBOL && parser_peek_token()->type == TokenType::OPEN_PARAN)
    {
        result = parser_parse_function_call();
        return result;
    }
    if (parser_get_token()->type == TokenType::NUMBER)
    {
        bool isDecimal = false;
        for (char c : parser_get_token()->value)
        {
            // is decimal?
            if (c == '.')
                isDecimal = true;
        }

        result = isDecimal ?
            new StatementValue(DataType::FLOAT, parser_get_token()->value) :
            new StatementValue(DataType::INT, parser_get_token()->value);
    }
    else if (parser_get_token()->type == TokenType::STRING)
    {
        result = new StatementValue(DataType::STRING, parser_get_token()->value);
    }
    else if (parser_get_token()->type == TokenType::SYMBOL)
    {
        result = new StatementSymbol(parser_get_token()->value);
    }
    // NEGATE
    else if (parser_get_token()->type == TokenType::MINUS)
    {
        parser_move_next_token();
        return new StatementUnaryOperator(OperatorType::NEGATE, parser_parse_exp_term());
    }
    // INNER EXPRESSIONS
    else if (parser_get_token()->type == TokenType::OPEN_PARAN)
    {
        unsigned int startingLine = parser_get_token()->line;

        parser_move_next_token();

        result = parser_parse_exp();

        if (parser_get_token()->type != TokenType::CLOSE_PARAN)
            ERROR("Expected a close paranthesis on line " + std::to_string(startingLine + 1) + ".");
    }
    else if (parser_get_token()->type == TokenType::_NULL)
    {
        result = new StatementValue(DataType::_NULL, EMPTY_STRING);
    }

    parser_move_next_token();
    return result;
}

Statement* parser_parse_exp_dot()
{
    stack<Statement*> stmtStack;
    stmtStack.push(parser_parse_exp_term());

    while (parser_get_token()->type == TokenType::DOT)
    {
        parser_move_next_token();
        stmtStack.push(parser_parse_exp_term());
    }

    if (stmtStack.size() > 1)
    {
        auto right = stmtStack.top();
        stmtStack.pop();
        auto left = stmtStack.top();
        stmtStack.pop();

        auto binaryOperator = new StatementBinaryOperator(OperatorType::MEMBER_ACCESSOR, left, right);

        while (stmtStack.size() > 0)
        {
            auto poped = stmtStack.top();
            stmtStack.pop();

            binaryOperator = new StatementBinaryOperator(OperatorType::MEMBER_ACCESSOR, poped, binaryOperator);
        }

        return binaryOperator;
    }
    else if (stmtStack.size() == 1)
    {
        return stmtStack.top();
    }

    return nullptr;
}

Statement* parser_parse_exp_muldivmod()
{
    Statement* left = parser_parse_exp_dot();

    while (parser_get_token()->type == TokenType::MULTIPLY ||
        parser_get_token()->type == TokenType::DIVIDE ||
        parser_get_token()->type == TokenType::MODULUS)
    {
        if (parser_get_token()->type == TokenType::MULTIPLY)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::MULTIPLICATION, left, parser_parse_exp_dot());
        }
        else if (parser_get_token()->type == TokenType::DIVIDE)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::DIVISION, left, parser_parse_exp_dot());
        }
        else if (parser_get_token()->type == TokenType::MODULUS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::MODULUS, left, parser_parse_exp_dot());
        }
    }

    return left;
}

Statement* parser_parse_exp_addsub()
{
    Statement* left = parser_parse_exp_muldivmod();

    while (parser_get_token()->type == TokenType::PLUS ||
        parser_get_token()->type == TokenType::MINUS)
    {
        if (parser_get_token()->type == TokenType::PLUS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::ADDITION, left, parser_parse_exp_muldivmod());
        }
        else if (parser_get_token()->type == TokenType::MINUS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::SUBTRACTION, left, parser_parse_exp_muldivmod());
        }
    }

    return left;
}

Statement* parser_parse_equality()
{
    Statement* left = parser_parse_exp_addsub();

    while (parser_get_token()->type == TokenType::EQUALS ||
        parser_get_token()->type == TokenType::NOT_EQUALS ||
        parser_get_token()->type == TokenType::GREATER_THAN ||
        parser_get_token()->type == TokenType::LESS_THAN ||
        parser_get_token()->type == TokenType::GREATER_THAN_E ||
        parser_get_token()->type == TokenType::LESS_THAN_E)
    {
        if (parser_get_token()->type == TokenType::EQUALS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::EQUALS, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == TokenType::NOT_EQUALS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::NOT_EQUALS, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == TokenType::GREATER_THAN)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::GREATER_THAN, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == TokenType::LESS_THAN)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::LESS_THAN, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == TokenType::GREATER_THAN_E)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::GREATER_THAN_E, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == TokenType::LESS_THAN_E)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::LESS_THAN_E, left, parser_parse_exp_addsub());
        }
    }

    return left;
}

Statement* parser_parse_andor()
{
    Statement* left = parser_parse_equality();

    while (parser_get_token()->type == TokenType::AND ||
        parser_get_token()->type == TokenType::OR)
    {
        if (parser_get_token()->type == TokenType::AND)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::AND, left, parser_parse_equality());
        }
        else if (parser_get_token()->type == TokenType::OR)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::OR, left, parser_parse_equality());
        }
    }

    return left;
}

Statement* parser_parse_exp_assign()
{
    stack<Statement*> stmtStack;
    stmtStack.push(parser_parse_andor());

    while (parser_get_token()->type == TokenType::ASSIGN)
    {
        parser_move_next_token();
        stmtStack.push(parser_parse_andor());
    }

    if (stmtStack.size() > 1)
    {
        auto right = stmtStack.top();
        stmtStack.pop();
        auto left = stmtStack.top();
        stmtStack.pop();

        auto binaryOperator = new StatementBinaryOperator(OperatorType::ASSIGNMENT, left, right);

        while (stmtStack.size() > 0)
        {
            auto poped = stmtStack.top();
            stmtStack.pop();

            binaryOperator = new StatementBinaryOperator(OperatorType::ASSIGNMENT, poped, binaryOperator);
        }

        return binaryOperator;
    }
    else if (stmtStack.size() == 1)
    {
        return stmtStack.top();
    }

    return nullptr;
}

StatementExpression* parser_parse_exp()
{
    return new StatementExpression(parser_parse_exp_assign());
}

#pragma endregion

Statement* parser_parse_statement();

/// @brief Parses a block with the first token being an open bracket.
StatementBlock* parser_parse_block()
{
    StatementBlock* block = new StatementBlock();

    if (parser_get_token()->type != TokenType::OPEN_BRACKET)
        ERROR("Missing an open bracket on line " + std::to_string(parser_get_token()->line + 1) + ".");

    unsigned int startingLine = parser_get_token()->line;

    parser_move_next_token();

    while (parser_get_token()->type != TokenType::END_OF_FILE)
    {
        auto stmt = parser_parse_statement();

        if (stmt != nullptr)
            block->statements.push_back(stmt);

        // if we encounter a close bracket then we are at the end of the block
        if (parser_get_token()->type == TokenType::CLOSE_BRACKET)
            break;

        // statements should end with an end of line
        if (parser_get_token()->type != TokenType::END_OF_LINE && parser_get_token()->type != TokenType::END_OF_FILE)
            ERROR("Expected the end of a statment on line " + std::to_string(parser_get_token()->line + 1) + " but got [" + parser_get_token()->value + "].");

        parser_move_next_token();
    }

    if (parser_get_token()->type != TokenType::CLOSE_BRACKET)
        ERROR("Expected a close bracket on line " + std::to_string(startingLine + 1) + ".");

    parser_move_next_token();

    return block;
}

StatementFunction* parser_parse_function()
{
    parser_move_next_token(); // first token is the fn keyword

    auto function = new StatementFunction(new StatementSymbol(parser_get_token()->value));

    parser_move_next_token();

    if (parser_get_token()->type != TokenType::OPEN_PARAN)
        ERROR("Missing an open paranthesis on line " + std::to_string(parser_get_token()->line + 1) + ".");

    unsigned int startingLine = parser_get_token()->line;

    while (parser_get_token()->type != TokenType::END_OF_FILE)
    {
        parser_move_next_token();

        if (parser_get_token()->type == TokenType::CLOSE_PARAN)
            break;

        if (parser_get_token()->type == TokenType::SYMBOL)
            function->params.push_back(new StatementSymbol(parser_get_token()->value));
        else
            ERROR("Expected a symbol on line " + std::to_string(parser_get_token()->line + 1) + " but got " + parser_get_token()->value + ".");
        
        parser_move_next_token();

        if (parser_get_token()->type != TokenType::COMMA)
            break;
    }

    if (parser_get_token()->type != TokenType::CLOSE_PARAN)
        ERROR("Missing a close paranthesis on line " + std::to_string(startingLine + 1) + ".");

    parser_move_next_token_skip_eols();

    function->body = parser_parse_block();

    return function;
}

StatementFunctionCall* parser_parse_function_call()
{
    auto functionCall = new StatementFunctionCall(new StatementSymbol(parser_get_token()->value));

    parser_move_next_token();
    parser_move_next_token();

    unsigned int startingLine = parser_get_token()->line;

    while (parser_get_token()->type != TokenType::CLOSE_PARAN && parser_get_token()->type != TokenType::END_OF_FILE)
    {
        functionCall->argExpressions.push_back(parser_parse_exp());

        if (parser_get_token()->type == TokenType::CLOSE_PARAN)
            break;

        if (parser_get_token()->type != TokenType::COMMA)
            ERROR("Missing a comma to seperate expressions on line " + std::to_string(parser_get_token()->line) + " got [" + parser_get_token()->value + "].");
        
        parser_move_next_token();
    }

    if (parser_get_token()->type != TokenType::CLOSE_PARAN)
        ERROR("Expected a close parenthesis on line " + std::to_string(startingLine + 1) + ".");

    parser_move_next_token();

    return functionCall;
}

StatementStruct* parser_parse_struct()
{
    parser_move_next_token();

    if (parser_get_token()->type != TokenType::SYMBOL)
        ERROR("Missing a name of struct on line " + std::to_string(parser_get_token()->line) + ".");

    auto _struct = new StatementStruct(new StatementSymbol(parser_get_token()->value));

    parser_move_next_token_skip_eols();

    if (parser_get_token()->type != TokenType::OPEN_BRACKET)
        ERROR("Missing an open bracket on line " + std::to_string(parser_get_token()->line) + ".");

    parser_move_next_token_skip_eols();

    unsigned int startingLine = parser_get_token()->line;

    // parse variables and functions in structure
    while (parser_get_token()->type != TokenType::CLOSE_BRACKET && parser_get_token()->type != TokenType::END_OF_FILE)
    {
        // VARIABLE
        if (parser_get_token()->type == TokenType::SYMBOL && parser_peek_token()->type == TokenType::ASSIGN)
        {
            _struct->variables.push_back(parser_parse_exp());
        }
        // FUNCTION
        else if (parser_get_token()->type == TokenType::FUNCITON)
        {
            _struct->functions.push_back(parser_parse_function());
        }
        else 
        {
            ERROR("Invalid statement in struct on line " + std::to_string(parser_get_token()->line) + " got [" + parser_get_token()->value + "].");
        }

        if (parser_get_token()->type == TokenType::CLOSE_BRACKET)
            break;

        // make sure that we've reached the end of the statement
        if (parser_get_token()->type != TokenType::END_OF_LINE && parser_get_token()->type != TokenType::END_OF_FILE)
            ERROR("Expected the end of a statment on line " + std::to_string(parser_get_token()->line + 1) + " but got [" + parser_get_token()->value + "].");

        parser_move_next_token_skip_eols();
    }

    if (parser_get_token()->type != TokenType::CLOSE_BRACKET)
        ERROR("Missing closing bracket on line " + std::to_string(startingLine + 1) + ".");

    parser_move_next_token();

    return _struct;
}

StatementElse* parser_parse_else()
{
    parser_move_next_token_skip_eols();
    return new StatementElse(parser_parse_block());
}

StatementIf* parser_parse_if()
{
    parser_move_next_token();

    auto exp = parser_parse_exp();

    if (parser_get_token()->type == TokenType::END_OF_LINE)
        parser_move_next_token_skip_eols();

    auto block = parser_parse_block();

    unsigned int pos = parserPosition;

    if (parser_get_token()->type == TokenType::END_OF_LINE)
        parser_move_next_token_skip_eols();

    auto _if = new StatementIf(exp, block, nullptr, nullptr);

    if (parser_get_token()->type == TokenType::ELSE && parser_peek_token()->type == TokenType::IF)
    {
        parser_move_next_token();
        _if->_elseIf = parser_parse_if();
    }
    else if (parser_get_token()->type == TokenType::ELSE)
    {
        _if->_else = parser_parse_else();
    }
    else
    {
        parserPosition = pos;
    }

    return _if;
}

/// @brief Parses the next statement.
Statement* parser_parse_statement()
{
    Statement* result = nullptr;

    if (parser_get_token()->type == TokenType::IF)
    {
        result = parser_parse_if();
    }
    // FUNCTION CALL
    else if (parser_get_token()->type == TokenType::SYMBOL && parser_peek_token()->type == TokenType::OPEN_PARAN)
    {
        result = parser_parse_function_call();
    }
    // EXPRESSIONS
    else if (parser_get_token()->type == TokenType::NUMBER ||
        parser_get_token()->type == TokenType::STRING ||
        parser_get_token()->type == TokenType::SYMBOL ||
        parser_get_token()->type == TokenType::MINUS ||
        parser_get_token()->type == TokenType::OPEN_PARAN)
    {
        result = parser_parse_exp();
    }
    // FUNCTIONS
    else if (parser_get_token()->type == TokenType::FUNCITON)
    {
        result = parser_parse_function();
    }
    else if (parser_get_token()->type == TokenType::STRUCT)
    {
        result = parser_parse_struct();
    }
    // RETURNS
    else if (parser_get_token()->type == TokenType::RETURN)
    {
        parser_move_next_token();
        result = new StatementReturn(parser_parse_exp());
    }

    return result;
}

/// @brief Entry point for parsing a list of tokens.
void parser_parse_statements()
{
    if (parserAstRoot == nullptr)
        parserAstRoot = new StatementBlock();

    // read until the end of the file
    while (parser_get_token()->type != TokenType::END_OF_FILE)
    {
        auto stmt = parser_parse_statement();

        if (stmt != nullptr)
            parserAstRoot->statements.push_back(stmt);

        // statements should end with an end of line
        if (parser_get_token()->type != TokenType::END_OF_LINE && parser_get_token()->type != TokenType::END_OF_FILE)
            ERROR("Expected the end of a statment on line " + std::to_string(parser_get_token()->line + 1) + " but got [" + parser_get_token()->value + "].");

        parser_move_next_token();
    }
}

#pragma endregion

#endif // COMPILER