#ifndef COMPILER
#define COMPILER

#include <stack>

#include "enums/token-type.hpp"
#include "enums/statement-type.hpp"
#include "enums/operator-type.hpp"
#include "enums/data-type.hpp"
#include "statements.hpp"
#include "lexer.hpp"

#define CAST(value, type) static_cast<type*>(value)
#define GLOBAL_NAMESPACE "Global"

unsigned int parserPosition = 0;
std::vector<Token*> parserTokens;

/// @brief Moves to the next token in the list of parserTokens.
void parser_move_next_token()
{
    if (parserPosition + 1 < parserTokens.size())
        ++parserPosition;
}

/// @brief Gets the token at the current parserPosition.
const Token* parser_get_token()
{
    return parserTokens[parserPosition];
}

/// @brief Peeks at the next token one parserPosition further than the current parserPosition. 
const Token* parser_peek_token()
{
    if (parserPosition + 1 < parserTokens.size())
        return parserTokens[parserPosition + 1];
    
    return parserTokens[parserTokens.size() - 1];
}

void parser_move_next_token_skip_eols()
{
    parser_move_next_token();

    while (parser_get_token()->type == _TokenType::END_OF_LINE)
        parser_move_next_token();
}

#pragma region Expressions

StatementExpression* parser_parse_exp();
StatementFunctionCall* parser_parse_function_call();

Statement* parser_parse_exp_term()
{
    Statement* result = nullptr;

    // FUNCTION CALL
    if (parser_get_token()->type == _TokenType::SYMBOL && parser_peek_token()->type == _TokenType::OPEN_PARAN)
    {
        result = parser_parse_function_call();
        return result;
    }
    if (parser_get_token()->type == _TokenType::NUMBER)
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
    else if (parser_get_token()->type == _TokenType::STRING)
    {
        result = new StatementValue(DataType::STRING, parser_get_token()->value);
    }
    else if (parser_get_token()->type == _TokenType::SYMBOL)
    {
        result = new StatementSymbol(parser_get_token()->value);
    }
    // NEGATE
    else if (parser_get_token()->type == _TokenType::MINUS)
    {
        parser_move_next_token();
        return new StatementUnaryOperator(OperatorType::NEGATE, parser_parse_exp_term());
    }
    // INNER EXPRESSIONS
    else if (parser_get_token()->type == _TokenType::OPEN_PARAN)
    {
        unsigned int startingLine = parser_get_token()->line;

        parser_move_next_token();

        result = parser_parse_exp();

        if (parser_get_token()->type != _TokenType::CLOSE_PARAN)
            CODE_ERROR("Expected a close paranthesis.", lexerFiles[lexerFIndex], startingLine + 1);
    }
    else if (parser_get_token()->type == _TokenType::_NULL)
    {
        result = new StatementValue(DataType::_NULL, EMPTY_STRING);
    }

    parser_move_next_token();
    return result;
}

Statement* parser_parse_exp_dot()
{
    std::stack<Statement*> stmtStack;
    stmtStack.push(parser_parse_exp_term());

    while (parser_get_token()->type == _TokenType::DOT)
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

    while (parser_get_token()->type == _TokenType::MULTIPLY ||
        parser_get_token()->type == _TokenType::DIVIDE ||
        parser_get_token()->type == _TokenType::MODULUS)
    {
        if (parser_get_token()->type == _TokenType::MULTIPLY)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::MULTIPLICATION, left, parser_parse_exp_dot());
        }
        else if (parser_get_token()->type == _TokenType::DIVIDE)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::DIVISION, left, parser_parse_exp_dot());
        }
        else if (parser_get_token()->type == _TokenType::MODULUS)
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

    while (parser_get_token()->type == _TokenType::PLUS ||
        parser_get_token()->type == _TokenType::MINUS)
    {
        if (parser_get_token()->type == _TokenType::PLUS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::ADDITION, left, parser_parse_exp_muldivmod());
        }
        else if (parser_get_token()->type == _TokenType::MINUS)
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

    while (parser_get_token()->type == _TokenType::EQUALS ||
        parser_get_token()->type == _TokenType::NOT_EQUALS ||
        parser_get_token()->type == _TokenType::GREATER_THAN ||
        parser_get_token()->type == _TokenType::LESS_THAN ||
        parser_get_token()->type == _TokenType::GREATER_THAN_E ||
        parser_get_token()->type == _TokenType::LESS_THAN_E)
    {
        if (parser_get_token()->type == _TokenType::EQUALS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::EQUALS, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == _TokenType::NOT_EQUALS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::NOT_EQUALS, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == _TokenType::GREATER_THAN)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::GREATER_THAN, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == _TokenType::LESS_THAN)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::LESS_THAN, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == _TokenType::GREATER_THAN_E)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::GREATER_THAN_E, left, parser_parse_exp_addsub());
        }
        else if (parser_get_token()->type == _TokenType::LESS_THAN_E)
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

    while (parser_get_token()->type == _TokenType::AND ||
        parser_get_token()->type == _TokenType::OR)
    {
        if (parser_get_token()->type == _TokenType::AND)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::AND, left, parser_parse_equality());
        }
        else if (parser_get_token()->type == _TokenType::OR)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::OR, left, parser_parse_equality());
        }
    }

    return left;
}

Statement* parser_parse_exp_assign()
{
    std::stack<Statement*> stmtStack;
    stmtStack.push(parser_parse_andor());

    while (parser_get_token()->type == _TokenType::ASSIGN)
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

Statement* parser_parse_a_single_statement(bool topLevel = false);

/// @brief Parses a block with the first token being an open bracket.
StatementBlock* parser_parse_block(bool withBrackets = true)
{
    auto block = new StatementBlock();

    if (withBrackets && parser_get_token()->type != _TokenType::OPEN_BRACKET)
        CODE_ERROR("Missing an open bracket.", lexerFiles[parser_get_token()->fIndex], parser_get_token()->line + 1);

    auto startingLine = parser_get_token()->line;

    if (withBrackets)
        parser_move_next_token();

    while (parser_get_token()->type != _TokenType::END_OF_FILE)
    {
        if (auto stmt = parser_parse_a_single_statement())
            block->statements.push_back(stmt);

        // if we encounter a close bracket then we are at the end of the block
        if (withBrackets && parser_get_token()->type == _TokenType::CLOSE_BRACKET)
            break;

        // statements should end with an end of line
        if (parser_get_token()->type != _TokenType::END_OF_LINE && parser_get_token()->type != _TokenType::END_OF_FILE)
            CODE_ERROR("Expected the end of a statment but got [" + parser_get_token()->value + "].", lexerFiles[parser_get_token()->fIndex], parser_get_token()->line + 1);

        parser_move_next_token();
    }

    if (withBrackets && parser_get_token()->type != _TokenType::CLOSE_BRACKET)
        CODE_ERROR("Expected a close bracket.", lexerFiles[parser_get_token()->fIndex], startingLine + 1);

    if (withBrackets)
        parser_move_next_token();

    return block;
}

StatementFunction* parser_parse_function()
{
    parser_move_next_token(); // first token is the fn keyword

    auto function = new StatementFunction(parser_get_token()->value);

    parser_move_next_token();

    if (parser_get_token()->type != _TokenType::OPEN_PARAN)
        CODE_ERROR("Missing an open paranthesis.", lexerFiles[parser_get_token()->fIndex], parser_get_token()->line + 1);

    unsigned int startingLine = parser_get_token()->line;
    int numberOfArgs = 0;

    while (parser_get_token()->type != _TokenType::END_OF_FILE)
    {
        parser_move_next_token();

        if (parser_get_token()->type == _TokenType::CLOSE_PARAN)
            break;

        if (parser_get_token()->type == _TokenType::SYMBOL) 
        {
            function->params.push_back(new StatementSymbol(parser_get_token()->value));
            ++numberOfArgs;
        }
        else
        {
            CODE_ERROR("Expected a symbol but got " + parser_get_token()->value + ".", lexerFiles[parser_get_token()->fIndex], parser_get_token()->line + 1);
        }
        
        parser_move_next_token();

        if (parser_get_token()->type != _TokenType::COMMA)
            break;
    }

    if (parser_get_token()->type != _TokenType::CLOSE_PARAN)
        CODE_ERROR("Missing a close paranthesis.", lexerFiles[parser_get_token()->fIndex], startingLine + 1);

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

    while (parser_get_token()->type != _TokenType::CLOSE_PARAN && parser_get_token()->type != _TokenType::END_OF_FILE)
    {
        functionCall->argExpressions.push_back(parser_parse_exp());
        ++numberOfArgs;

        if (parser_get_token()->type == _TokenType::CLOSE_PARAN)
            break;

        if (parser_get_token()->type != _TokenType::COMMA)
            CODE_ERROR("Missing a comma to seperate expressions got [" + parser_get_token()->value + "].", lexerFiles[parser_get_token()->fIndex], parser_get_token()->line);
        
        parser_move_next_token();
    }

    if (parser_get_token()->type != _TokenType::CLOSE_PARAN)
        CODE_ERROR("Expected a close parenthesis.", lexerFiles[parser_get_token()->fIndex], startingLine + 1);

    parser_move_next_token();

    functionCall->name += std::to_string(numberOfArgs);

    return functionCall;
}

StatementStruct* parser_parse_struct()
{
    parser_move_next_token();

    if (parser_get_token()->type != _TokenType::SYMBOL)
        CODE_ERROR("Missing a name of struct.", lexerFiles[parser_get_token()->fIndex], parser_get_token()->line);

    auto _struct = new StatementStruct(new StatementSymbol(parser_get_token()->value));

    parser_move_next_token_skip_eols();

    if (parser_get_token()->type != _TokenType::OPEN_BRACKET)
        CODE_ERROR("Missing an open bracket.", lexerFiles[parser_get_token()->fIndex], parser_get_token()->line);

    parser_move_next_token_skip_eols();

    unsigned int startingLine = parser_get_token()->line;

    // parse variables and functions in structure
    while (parser_get_token()->type != _TokenType::CLOSE_BRACKET && parser_get_token()->type != _TokenType::END_OF_FILE)
    {
        // VARIABLE
        if (parser_get_token()->type == _TokenType::SYMBOL && parser_peek_token()->type == _TokenType::ASSIGN)
        {
            _struct->variables.push_back(parser_parse_exp());
        }
        // FUNCTION
        else if (parser_get_token()->type == _TokenType::FUNCITON)
        {
            _struct->functions.push_back(parser_parse_function());
        }
        else 
        {
            CODE_ERROR("Invalid statement in struct but got [" + parser_get_token()->value + "].", lexerFiles[parser_get_token()->fIndex], parser_get_token()->line);
        }

        if (parser_get_token()->type == _TokenType::CLOSE_BRACKET)
            break;

        // make sure that we've reached the end of the statement
        if (parser_get_token()->type != _TokenType::END_OF_LINE && parser_get_token()->type != _TokenType::END_OF_FILE)
            CODE_ERROR("Expected the end of a statment but got [" + parser_get_token()->value + "].", lexerFiles[parser_get_token()->fIndex], parser_get_token()->line + 1);

        parser_move_next_token_skip_eols();
    }

    if (parser_get_token()->type != _TokenType::CLOSE_BRACKET)
        CODE_ERROR("Missing closing bracket on line.", lexerFiles[parser_get_token()->fIndex], startingLine + 1);

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

    if (parser_get_token()->type == _TokenType::END_OF_LINE)
        parser_move_next_token_skip_eols();

    auto block = parser_parse_block();

    unsigned int pos = parserPosition;

    if (parser_get_token()->type == _TokenType::END_OF_LINE)
        parser_move_next_token_skip_eols();

    auto _if = new StatementIf(exp, block, nullptr, nullptr);

    if (parser_get_token()->type == _TokenType::ELSE && parser_peek_token()->type == _TokenType::IF)
    {
        parser_move_next_token();
        _if->_elseIf = parser_parse_if();
    }
    else if (parser_get_token()->type == _TokenType::ELSE)
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
Statement* parser_parse_a_single_statement(bool topLevel)
{
    Statement* result = nullptr;

    if (parser_get_token()->type == _TokenType::IF)
    {
        result = parser_parse_if();
    }
    // FUNCTION CALL
    else if (parser_get_token()->type == _TokenType::SYMBOL && parser_peek_token()->type == _TokenType::OPEN_PARAN)
    {
        result = parser_parse_function_call();
    }
    // EXPRESSIONS
    else if (parser_get_token()->type == _TokenType::NUMBER ||
        parser_get_token()->type == _TokenType::STRING ||
        parser_get_token()->type == _TokenType::SYMBOL ||
        parser_get_token()->type == _TokenType::MINUS ||
        parser_get_token()->type == _TokenType::OPEN_PARAN)
    {
        result = parser_parse_exp();
    }
    // FUNCTIONS
    else if (parser_get_token()->type == _TokenType::FUNCITON)
    {
        result = parser_parse_function();
    }
    else if (parser_get_token()->type == _TokenType::STRUCT)
    {
        result = parser_parse_struct();
    }
    // RETURNS
    else if (parser_get_token()->type == _TokenType::RETURN)
    {
        parser_move_next_token();
        result = new StatementReturn(parser_parse_exp());
    }
    else if (parser_get_token()->type == _TokenType::NAMESPACE)
    {
        if (!topLevel)
            CODE_ERROR("Namespaces cannot be nested.", lexerFiles[parser_get_token()->fIndex], parser_get_token()->line);
        
        parser_move_next_token();
        auto ns = new StatementBlockNamespace(parser_get_token()->value);

        parser_move_next_token_skip_eols();

        StatementBlock* block;

        if (parser_get_token()->type == _TokenType::OPEN_BRACKET)
            block = parser_parse_block();
        else
            block = parser_parse_block(false);

        for (auto& stmt : block->statements)
        {
            ns->statements.push_back(stmt);

            // must set the statements to null after
            // otherwise the deconstructor for the block
            // will delete its child statements and
            // thus, no statements will actually be transfered
            // to the namespace block
            stmt = nullptr;
        }

        delete block;

        result = ns;
    }

    return result;
}

/// @brief Entry point for parsing a list of parserTokens.
void parser_get_syntax_tree_from_tokens(SyntaxTree* tree, std::vector<Token*>& tokens)
{
    parserPosition = 0;
    parserTokens = tokens;

    // read until the end of the file
    while (parser_get_token()->type != _TokenType::END_OF_FILE)
    {
        if (auto stmt = parser_parse_a_single_statement(true))
            tree->statements.push_back(stmt);

        // statements should end with an end of line
        if (parser_get_token()->type != _TokenType::END_OF_LINE && parser_get_token()->type != _TokenType::END_OF_FILE)
            CODE_ERROR("Expected the end of a statment but got [" + parser_get_token()->value + "].", lexerFiles[parser_get_token()->fIndex], parser_get_token()->line + 1);

        parser_move_next_token();
    }
}

#pragma endregion

#endif // COMPILER