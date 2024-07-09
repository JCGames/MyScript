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

#include "enums/token-type.hpp"
#include "enums/statement-type.hpp"
#include "enums/operator-type.hpp"
#include "enums/data-type.hpp"

#define LOG_ERROR(msg) logger_error(msg, __FILE__, __LINE__)
#define CODE_ERROR(msg, file, line) logger_code_error(msg, file, line, __FILE__, __LINE__)
#define CAST(value, type) static_cast<type*>(value)
#define GLOBAL_NAMESPACE "Global"

using std::vector;
using std::string;
using std::ifstream;
using std::stack;
using std::map;

const string EMPTY_STRING = "";

/// @brief Holds the names of all the files that have been lexified and parsed.
vector<string> lexeFiles;

/// @brief Gets the current file that is being lexified
unsigned int lexerFIndex;

void logger_error(string msg, const char* file, int line)
{
    std::cout << msg << " <<COMPILER:[" << file << ":" << line << "]>>" << std::endl;
    exit(1);
}

void logger_code_error(string msg, std::string codeFile, unsigned int codeLine, const char* file, int line)
{
    std::cout << msg << " [" << codeFile << ":" << codeLine << "] <<COMPILER:[" << file << ":" << line << "]>>" << std::endl;
    exit(1);
}

bool string_replace(std::string& str, const std::string& from, const std::string& to) 
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

// =====================
// |       LEXER       |
// =====================

#pragma region Lexer

struct Token
{
    pop::TokenType type;
    string value;
    unsigned int fIndex;
    unsigned int line;

    Token(pop::TokenType type, string value, unsigned int fIndex, unsigned int line)
    {
        this->type = type;
        this->value = value;
        this->fIndex = fIndex;
        this->line = line;
    }
};

vector<Token*> lexerTokens;

void lexer_delete_tokens()
{
    for (auto token : lexerTokens)
    {
        if (token != nullptr)
            delete token;
    }
}

void lexer_lexify(string fileName)
{
    ifstream ifs(fileName);

    if (!ifs.is_open())
        LOG_ERROR("Was unable to open file with name " + fileName + ".");

    lexeFiles.push_back(fileName);
    lexerFIndex = lexeFiles.size() - 1;

    // reset the tokens list
    if (lexerTokens.size() > 0)
        lexer_delete_tokens();
    lexerTokens.clear();

    unsigned int line = 0;
    char c;

    // tokenize the entire file
    while (!ifs.eof())
    {
        c = ifs.get();

        // COMMENT
        if (c == '/' && ifs.peek() == '/')
        {
            while (!ifs.eof() && ifs.peek() != '\n')
                ifs.get();

            continue;
        }

        // END_OF_LINE
        if (c == '\n')
        {
            auto token = new Token(pop::TokenType::END_OF_LINE, EMPTY_STRING, lexerFIndex, line);
            ++line;
            lexerTokens.push_back(token);
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
                lexerTokens.push_back(new Token(pop::TokenType::FUNCITON, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "return")
            {
                lexerTokens.push_back(new Token(pop::TokenType::RETURN, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "struct")
            {
                lexerTokens.push_back(new Token(pop::TokenType::STRUCT, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "if")
            {
                lexerTokens.push_back(new Token(pop::TokenType::IF, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "else")
            {
                lexerTokens.push_back(new Token(pop::TokenType::ELSE, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "and")
            {
                lexerTokens.push_back(new Token(pop::TokenType::AND, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "or")
            {
                lexerTokens.push_back(new Token(pop::TokenType::OR, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "null")
            {
                lexerTokens.push_back(new Token(pop::TokenType::_NULL, symbol, lexerFIndex, line));
                continue;
            }

            lexerTokens.push_back(new Token(pop::TokenType::SYMBOL, symbol, lexerFIndex, line));
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
                CODE_ERROR("Missing a closing quotes.", lexeFiles[lexerFIndex], beginLine + 1);

            string_replace(str, "\\n", "\n");

            lexerTokens.push_back(new Token(pop::TokenType::STRING, str, lexerFIndex, line));
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
                    CODE_ERROR("Number has too many decimal points.", lexeFiles[lexerFIndex], line + 1);
                }

                number += c;
            }

            lexerTokens.push_back(new Token(pop::TokenType::NUMBER, number, lexerFIndex, line));
            continue;
        }
        // DOUBLE OPERATORS
        else if (c == '-' && ifs.peek() == '>')
        {
            char next = ifs.get();
            lexerTokens.push_back(new Token(pop::TokenType::ASSIGN, string(1, c) + string(1, next), lexerFIndex, line));
        }
        else if (c == '!' && ifs.peek() == '=')
        {
            char next = ifs.get();
            lexerTokens.push_back(new Token(pop::TokenType::NOT_EQUALS, string(1, c) + string(1, next), lexerFIndex, line));
        }
        else if (c == '>' && ifs.peek() == '=')
        {
            char next = ifs.get();
            lexerTokens.push_back(new Token(pop::TokenType::GREATER_THAN_E, string(1, c) + string(1, next), lexerFIndex, line));
        }
        else if (c == '<' && ifs.peek() == '=')
        {
            char next = ifs.get();
            lexerTokens.push_back(new Token(pop::TokenType::LESS_THAN_E, string(1, c) + string(1, next), lexerFIndex, line));
        }
        // SINGLE CHARACTER OPERATORS
        else if (c == '=')
        {
            lexerTokens.push_back(new Token(pop::TokenType::EQUALS, string(1, c), lexerFIndex, line));
        }
        else if (c == '>')
        {
            lexerTokens.push_back(new Token(pop::TokenType::GREATER_THAN, string(1, c), lexerFIndex, line));
        }
        else if (c == '<')
        {
            lexerTokens.push_back(new Token(pop::TokenType::LESS_THAN, string(1, c), lexerFIndex, line));
        }
        else if (c == '+')
        {
            lexerTokens.push_back(new Token(pop::TokenType::PLUS, string(1, c), lexerFIndex, line));
        }
        else if (c == '-')
        {
            lexerTokens.push_back(new Token(pop::TokenType::MINUS, string(1, c), lexerFIndex, line));
        }
        else if (c == '*')
        {
            lexerTokens.push_back(new Token(pop::TokenType::MULTIPLY, string(1, c), lexerFIndex, line));
        }
        else if (c == '/')
        {
            lexerTokens.push_back(new Token(pop::TokenType::DIVIDE, string(1, c), lexerFIndex, line));
        }
        else if (c == '%')
        {
            lexerTokens.push_back(new Token(pop::TokenType::MODULUS, string(1, c), lexerFIndex, line));
        }
        else if (c == '(')
        {
            lexerTokens.push_back(new Token(pop::TokenType::OPEN_PARAN, string(1, c), lexerFIndex, line));
        }
        else if (c == ')')
        {
            lexerTokens.push_back(new Token(pop::TokenType::CLOSE_PARAN, string(1, c), lexerFIndex, line));
        }
        else if (c == '{')
        {
            lexerTokens.push_back(new Token(pop::TokenType::OPEN_BRACKET, string(1, c), lexerFIndex, line));
        }
        else if (c == '}')
        {
            lexerTokens.push_back(new Token(pop::TokenType::CLOSE_BRACKET, string(1, c), lexerFIndex, line));
        }
        else if (c == ',')
        {
            lexerTokens.push_back(new Token(pop::TokenType::COMMA, string(1, c), lexerFIndex, line));
        }
        else if (c == '.')
        {
            lexerTokens.push_back(new Token(pop::TokenType::DOT, string(1, c), lexerFIndex, line));
        }
    }

    lexerTokens.push_back(new Token(pop::TokenType::END_OF_FILE, EMPTY_STRING, lexerFIndex, line));

    ifs.close();
}

void lexer_print_tokens()
{
    for (const auto token : lexerTokens)
    {
        std::cout << "Type: " << token_type_name(token->type) << " Value: |" << token->value << "| Line: " << token->line + 1 << std::endl;
    }
}

#pragma endregion

// =======================
// |       PARSER        |
// =======================

#pragma region Parser

#include "statements.hpp"

unsigned int parserPosition = 0;

/// @brief Moves to the next token in the list of lexerTokens.
void parser_move_next_token()
{
    if (parserPosition + 1 < lexerTokens.size())
        ++parserPosition;
}

/// @brief Gets the token at the current parserPosition.
const Token* parser_get_token()
{
    return lexerTokens[parserPosition];
}

/// @brief Peeks at the next token one parserPosition further than the current parserPosition. 
const Token* parser_peek_token()
{
    if (parserPosition + 1 < lexerTokens.size())
        return lexerTokens[parserPosition + 1];
    
    return lexerTokens[lexerTokens.size() - 1];
}

void parser_move_next_token_skip_eols()
{
    parser_move_next_token();

    while (parser_get_token()->type == pop::TokenType::END_OF_LINE)
        parser_move_next_token();
}

#pragma region Expressions

StatementExpression* parser_parse_exp();
StatementFunctionCall* parser_parse_function_call();

Statement* parser_parse_exp_term()
{
    Statement* result = nullptr;

    // FUNCTION CALL
    if (parser_get_token()->type == pop::TokenType::SYMBOL && parser_peek_token()->type == pop::TokenType::OPEN_PARAN)
    {
        result = parser_parse_function_call();
        return result;
    }
    if (parser_get_token()->type == pop::TokenType::NUMBER)
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
    else if (parser_get_token()->type == pop::TokenType::STRING)
    {
        result = new StatementValue(DataType::STRING, parser_get_token()->value);
    }
    else if (parser_get_token()->type == pop::TokenType::SYMBOL)
    {
        result = new StatementSymbol(parser_get_token()->value);
    }
    // NEGATE
    else if (parser_get_token()->type == pop::TokenType::MINUS)
    {
        parser_move_next_token();
        return new StatementUnaryOperator(OperatorType::NEGATE, parser_parse_exp_term());
    }
    // INNER EXPRESSIONS
    else if (parser_get_token()->type == pop::TokenType::OPEN_PARAN)
    {
        unsigned int startingLine = parser_get_token()->line;

        parser_move_next_token();

        result = parser_parse_exp();

        if (parser_get_token()->type != pop::TokenType::CLOSE_PARAN)
            CODE_ERROR("Expected a close paranthesis.", lexeFiles[lexerFIndex], startingLine + 1);
    }
    else if (parser_get_token()->type == pop::TokenType::_NULL)
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

    while (parser_get_token()->type == pop::TokenType::DOT)
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

    while (parser_get_token()->type == pop::TokenType::MULTIPLY ||
        parser_get_token()->type == pop::TokenType::DIVIDE ||
        parser_get_token()->type == pop::TokenType::MODULUS)
    {
        if (parser_get_token()->type == pop::TokenType::MULTIPLY)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::MULTIPLICATION, left, parser_parse_exp_dot());
        }
        else if (parser_get_token()->type == pop::TokenType::DIVIDE)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::DIVISION, left, parser_parse_exp_dot());
        }
        else if (parser_get_token()->type == pop::TokenType::MODULUS)
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

    while (parser_get_token()->type == pop::TokenType::PLUS ||
        parser_get_token()->type == pop::TokenType::MINUS)
    {
        if (parser_get_token()->type == pop::TokenType::PLUS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::ADDITION, left, parser_parse_exp_muldivmod());
        }
        else if (parser_get_token()->type == pop::TokenType::MINUS)
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

    while (parser_get_token()->type == pop::TokenType::EQUALS ||
        parser_get_token()->type == pop::TokenType::NOT_EQUALS ||
        parser_get_token()->type == pop::TokenType::GREATER_THAN ||
        parser_get_token()->type == pop::TokenType::LESS_THAN ||
        parser_get_token()->type == pop::TokenType::GREATER_THAN_E ||
        parser_get_token()->type == pop::TokenType::LESS_THAN_E)
    {
        if (parser_get_token()->type == pop::TokenType::EQUALS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::EQUALS, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == pop::TokenType::NOT_EQUALS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::NOT_EQUALS, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == pop::TokenType::GREATER_THAN)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::GREATER_THAN, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == pop::TokenType::LESS_THAN)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::LESS_THAN, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == pop::TokenType::GREATER_THAN_E)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::GREATER_THAN_E, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == pop::TokenType::LESS_THAN_E)
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

    while (parser_get_token()->type == pop::TokenType::AND ||
        parser_get_token()->type == pop::TokenType::OR)
    {
        if (parser_get_token()->type == pop::TokenType::AND)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::AND, left, parser_parse_equality());
        }
        else if (parser_get_token()->type == pop::TokenType::OR)
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

    while (parser_get_token()->type == pop::TokenType::ASSIGN)
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

Statement* parser_parse_a_single_statement();

/// @brief Parses a block with the first token being an open bracket.
StatementBlock* parser_parse_block()
{
    StatementBlock* block = new StatementBlock();

    if (parser_get_token()->type != pop::TokenType::OPEN_BRACKET)
        CODE_ERROR("Missing an open bracket.", lexeFiles[parser_get_token()->fIndex], parser_get_token()->line + 1);

    unsigned int startingLine = parser_get_token()->line;

    parser_move_next_token();

    while (parser_get_token()->type != pop::TokenType::END_OF_FILE)
    {
        auto stmt = parser_parse_a_single_statement();

        if (stmt != nullptr)
            block->statements.push_back(stmt);

        // if we encounter a close bracket then we are at the end of the block
        if (parser_get_token()->type == pop::TokenType::CLOSE_BRACKET)
            break;

        // statements should end with an end of line
        if (parser_get_token()->type != pop::TokenType::END_OF_LINE && parser_get_token()->type != pop::TokenType::END_OF_FILE)
            CODE_ERROR("Expected the end of a statment but got [" + parser_get_token()->value + "].", lexeFiles[parser_get_token()->fIndex], parser_get_token()->line + 1);

        parser_move_next_token();
    }

    if (parser_get_token()->type != pop::TokenType::CLOSE_BRACKET)
        CODE_ERROR("Expected a close bracket.", lexeFiles[parser_get_token()->fIndex], startingLine + 1);

    parser_move_next_token();

    return block;
}

StatementFunction* parser_parse_function()
{
    parser_move_next_token(); // first token is the fn keyword

    auto function = new StatementFunction(parser_get_token()->value);

    parser_move_next_token();

    if (parser_get_token()->type != pop::TokenType::OPEN_PARAN)
        CODE_ERROR("Missing an open paranthesis.", lexeFiles[parser_get_token()->fIndex], parser_get_token()->line + 1);

    unsigned int startingLine = parser_get_token()->line;
    int numberOfArgs = 0;

    while (parser_get_token()->type != pop::TokenType::END_OF_FILE)
    {
        parser_move_next_token();

        if (parser_get_token()->type == pop::TokenType::CLOSE_PARAN)
            break;

        if (parser_get_token()->type == pop::TokenType::SYMBOL) 
        {
            function->params.push_back(new StatementSymbol(parser_get_token()->value));
            ++numberOfArgs;
        }
        else
        {
            CODE_ERROR("Expected a symbol but got " + parser_get_token()->value + ".", lexeFiles[parser_get_token()->fIndex], parser_get_token()->line + 1);
        }
        
        parser_move_next_token();

        if (parser_get_token()->type != pop::TokenType::COMMA)
            break;
    }

    if (parser_get_token()->type != pop::TokenType::CLOSE_PARAN)
        CODE_ERROR("Missing a close paranthesis.", lexeFiles[parser_get_token()->fIndex], startingLine + 1);

    parser_move_next_token_skip_eols();

    function->name += std::to_string(numberOfArgs);
    function->body = parser_parse_block();

    return function;
}

StatementFunctionCall* parser_parse_function_call()
{
    auto functionCall = new StatementFunctionCall(parser_get_token()->value);

    parser_move_next_token();
    parser_move_next_token();

    unsigned int startingLine = parser_get_token()->line;
    int numberOfArgs = 0;

    while (parser_get_token()->type != pop::TokenType::CLOSE_PARAN && parser_get_token()->type != pop::TokenType::END_OF_FILE)
    {
        functionCall->argExpressions.push_back(parser_parse_exp());
        ++numberOfArgs;

        if (parser_get_token()->type == pop::TokenType::CLOSE_PARAN)
            break;

        if (parser_get_token()->type != pop::TokenType::COMMA)
            CODE_ERROR("Missing a comma to seperate expressions got [" + parser_get_token()->value + "].", lexeFiles[parser_get_token()->fIndex], parser_get_token()->line);
        
        parser_move_next_token();
    }

    if (parser_get_token()->type != pop::TokenType::CLOSE_PARAN)
        CODE_ERROR("Expected a close parenthesis.", lexeFiles[parser_get_token()->fIndex], startingLine + 1);

    parser_move_next_token();

    functionCall->name += std::to_string(numberOfArgs);

    return functionCall;
}

StatementStruct* parser_parse_struct()
{
    parser_move_next_token();

    if (parser_get_token()->type != pop::TokenType::SYMBOL)
        CODE_ERROR("Missing a name of struct.", lexeFiles[parser_get_token()->fIndex], parser_get_token()->line);

    auto _struct = new StatementStruct(new StatementSymbol(parser_get_token()->value));

    parser_move_next_token_skip_eols();

    if (parser_get_token()->type != pop::TokenType::OPEN_BRACKET)
        CODE_ERROR("Missing an open bracket.", lexeFiles[parser_get_token()->fIndex], parser_get_token()->line);

    parser_move_next_token_skip_eols();

    unsigned int startingLine = parser_get_token()->line;

    // parse variables and functions in structure
    while (parser_get_token()->type != pop::TokenType::CLOSE_BRACKET && parser_get_token()->type != pop::TokenType::END_OF_FILE)
    {
        // VARIABLE
        if (parser_get_token()->type == pop::TokenType::SYMBOL && parser_peek_token()->type == pop::TokenType::ASSIGN)
        {
            _struct->variables.push_back(parser_parse_exp());
        }
        // FUNCTION
        else if (parser_get_token()->type == pop::TokenType::FUNCITON)
        {
            _struct->functions.push_back(parser_parse_function());
        }
        else 
        {
            CODE_ERROR("Invalid statement in struct but got [" + parser_get_token()->value + "].", lexeFiles[parser_get_token()->fIndex], parser_get_token()->line);
        }

        if (parser_get_token()->type == pop::TokenType::CLOSE_BRACKET)
            break;

        // make sure that we've reached the end of the statement
        if (parser_get_token()->type != pop::TokenType::END_OF_LINE && parser_get_token()->type != pop::TokenType::END_OF_FILE)
            CODE_ERROR("Expected the end of a statment but got [" + parser_get_token()->value + "].", lexeFiles[parser_get_token()->fIndex], parser_get_token()->line + 1);

        parser_move_next_token_skip_eols();
    }

    if (parser_get_token()->type != pop::TokenType::CLOSE_BRACKET)
        CODE_ERROR("Missing closing bracket on line.", lexeFiles[parser_get_token()->fIndex], startingLine + 1);

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

    if (parser_get_token()->type == pop::TokenType::END_OF_LINE)
        parser_move_next_token_skip_eols();

    auto block = parser_parse_block();

    unsigned int pos = parserPosition;

    if (parser_get_token()->type == pop::TokenType::END_OF_LINE)
        parser_move_next_token_skip_eols();

    auto _if = new StatementIf(exp, block, nullptr, nullptr);

    if (parser_get_token()->type == pop::TokenType::ELSE && parser_peek_token()->type == pop::TokenType::IF)
    {
        parser_move_next_token();
        _if->_elseIf = parser_parse_if();
    }
    else if (parser_get_token()->type == pop::TokenType::ELSE)
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
Statement* parser_parse_a_single_statement()
{
    Statement* result = nullptr;

    if (parser_get_token()->type == pop::TokenType::IF)
    {
        result = parser_parse_if();
    }
    // FUNCTION CALL
    else if (parser_get_token()->type == pop::TokenType::SYMBOL && parser_peek_token()->type == pop::TokenType::OPEN_PARAN)
    {
        result = parser_parse_function_call();
    }
    // EXPRESSIONS
    else if (parser_get_token()->type == pop::TokenType::NUMBER ||
        parser_get_token()->type == pop::TokenType::STRING ||
        parser_get_token()->type == pop::TokenType::SYMBOL ||
        parser_get_token()->type == pop::TokenType::MINUS ||
        parser_get_token()->type == pop::TokenType::OPEN_PARAN)
    {
        result = parser_parse_exp();
    }
    // FUNCTIONS
    else if (parser_get_token()->type == pop::TokenType::FUNCITON)
    {
        result = parser_parse_function();
    }
    else if (parser_get_token()->type == pop::TokenType::STRUCT)
    {
        result = parser_parse_struct();
    }
    // RETURNS
    else if (parser_get_token()->type == pop::TokenType::RETURN)
    {
        parser_move_next_token();
        result = new StatementReturn(parser_parse_exp());
    }

    return result;
}

/// @brief Entry point for parsing a list of lexerTokens.
StatementBlockNamespace* parser_parse_statements()
{
    parserPosition = 0;

    auto ast = new StatementBlockNamespace(GLOBAL_NAMESPACE);

    // read until the end of the file
    while (parser_get_token()->type != pop::TokenType::END_OF_FILE)
    {
        auto stmt = parser_parse_a_single_statement();

        if (stmt != nullptr)
            ast->statements.push_back(stmt);

        // statements should end with an end of line
        if (parser_get_token()->type != pop::TokenType::END_OF_LINE && parser_get_token()->type != pop::TokenType::END_OF_FILE)
            CODE_ERROR("Expected the end of a statment but got [" + parser_get_token()->value + "].", lexeFiles[parser_get_token()->fIndex], parser_get_token()->line + 1);

        parser_move_next_token();
    }

    return ast;
}

#pragma endregion

#endif // COMPILER