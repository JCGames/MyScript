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
unsigned int parserUniqueInt = 0;
std::vector<Token*> parserTokens;

// Statement rule symbol descriptions
// *    '-' means must be same line
// *    '?' whatever comes next is optional
// *    '...' one or more
// *    '<>' encloses a statement

/// @brief Moves to the next token in the list of parserTokens.
void parser_move_next_token()
{
    if (parserPosition + 1 < parserTokens.size())
    {
        ++parserPosition;
        statementLocationMetadata = parserTokens[parserPosition]->loc;
    }
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

unsigned int& parser_get_next_unique_int()
{
    ++parserUniqueInt;
    return parserUniqueInt;
}

#pragma region Expressions

StatementExpression* parser_parse_exp();
StatementFunctionCall* parser_parse_function_call();
StatementFunction* parser_parse_function(const bool& hasName = true);

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
            new StatementConstant(DataType::FLOAT, parser_get_token()->value) :
            new StatementConstant(DataType::INT, parser_get_token()->value);
    }
    else if (parser_get_token()->type == _TokenType::STRING)
    {
        result = new StatementConstant(DataType::STRING, parser_get_token()->value);
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
        unsigned int errorLine = parser_get_token()->loc.line;

        parser_move_next_token();

        result = parser_parse_exp();

        if (parser_get_token()->type != _TokenType::CLOSE_PARAN)
            CODE_ERROR("Expected a close paranthesis.", parser_get_token()->loc.fileName, errorLine + 1);
    }
    else if (parser_get_token()->type == _TokenType::_NULL)
    {
        result = new StatementConstant(DataType::_NULL, EMPTY_STRING);
    }
    else if (parser_get_token()->type == _TokenType::_TRUE)
    {
        result = new StatementConstant(DataType::BOOL, "1");
    }
    else if (parser_get_token()->type == _TokenType::_FALSE)
    {
        result = new StatementConstant(DataType::BOOL, "0");
    }
    else if (parser_get_token()->type == _TokenType::FUNCITON)
    {
        return parser_parse_function(false);
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

/**
 * Always ends on the next token.
 */
StatementExpression* parser_parse_exp()
{
    return new StatementExpression(parser_parse_exp_assign());
}

#pragma endregion

#pragma region Statements

Statement* parser_parse_a_single_statement(const bool& topLevel = false);

/**
 * Statements should end with an END_OF_LINE or END_OF_FILE token.
 */
void parser_assert_end_of_statement()
{
    if (parser_get_token()->type != _TokenType::END_OF_LINE && parser_get_token()->type != _TokenType::END_OF_FILE)
        CODE_ERROR("Expected the end of a statment but got [" + parser_get_token()->value + "].", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);
}

/**
 * Parses a block.
 * @param withBrackets if true, will read until a closed bracket, and if false, will read until an END_OF_FILE token.
 */
StatementBlock* parser_parse_block(const bool& withBrackets = true)
{
    auto block = new StatementBlock();

    // for errors
    auto errorLine = parser_get_token()->loc.line;

    if (withBrackets)
    {
        if (parser_get_token()->type != _TokenType::OPEN_BRACKET)
            CODE_ERROR("Expected an open bracket.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);

        parser_move_next_token();
    }

    // read statements consecutively until reaching a close bracket
    while (parser_get_token()->type != _TokenType::END_OF_FILE)
    {
        if (auto stmt = parser_parse_a_single_statement())
            block->statements.push_back(stmt);

        if (withBrackets && parser_get_token()->type == _TokenType::CLOSE_BRACKET)
            break;

        // if it is not the end of the block 
        // make sure the statement ends with
        // an END_OF_LINE token
        parser_assert_end_of_statement();
        parser_move_next_token();
    }

    if (withBrackets)
    {
        if (parser_get_token()->type != _TokenType::CLOSE_BRACKET)
            CODE_ERROR("Expected a closed bracket.", parser_get_token()->loc.fileName, errorLine + 1);

        parser_move_next_token();
    }

    return block;
}

/**
 * fn-`Name`-(? `symbol`... , ) {
 * 
 * }-EOL
 */
StatementFunction* parser_parse_function(const bool& hasName)
{
    auto function = new StatementFunction(EMPTY_STRING);

    if (hasName)
    {
        parser_move_next_token();

        if (parser_get_token()->type != _TokenType::SYMBOL)
            CODE_ERROR("Expected a function name.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);

        function->name = parser_get_token()->value;
    }
    else
    {
        function->name = std::to_string(parser_get_next_unique_int()) + "_";
    }

    parser_move_next_token();

    if (parser_get_token()->type != _TokenType::OPEN_PARAN)
        CODE_ERROR("Expected an open paranthesis.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);

    parser_move_next_token();

    // for errors
    unsigned int errorLine = parser_get_token()->loc.line;

    int numberOfParams = 0;

    // if the function parameter list starts with a closed 
    // parentheses there are no parameters to parse
    if (parser_get_token()->type != _TokenType::CLOSE_PARAN)
    {
        while (parser_get_token()->type != _TokenType::END_OF_FILE)
        {
            if (parser_get_token()->type != _TokenType::SYMBOL)
                CODE_ERROR("Expected a function parameter.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);

            function->params.push_back(new StatementSymbol(parser_get_token()->value));
            ++numberOfParams;
            parser_move_next_token();

            if (parser_get_token()->type == _TokenType::CLOSE_PARAN)
                break;

            if (parser_get_token()->type != _TokenType::COMMA)
                CODE_ERROR("Missing a comma to seperate function parameters.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);

            parser_move_next_token();
        }

        if (parser_get_token()->type != _TokenType::CLOSE_PARAN)
            CODE_ERROR("Expected a closed paranthesis.", parser_get_token()->loc.fileName, errorLine + 1);
    }

    parser_move_next_token_skip_eols();

    // function names consist of the name of the function concatinated
    // with the number of parameters the function has
    function->name += std::to_string(numberOfParams);

    function->body = parser_parse_block();

    return function;
}

/**
 * `FunctionName`-(? `expression`... , )-EOL
 */
StatementFunctionCall* parser_parse_function_call()
{
    auto functionCall = new StatementFunctionCall(parser_get_token()->value);

    parser_move_next_token(); // skip symbol
    parser_move_next_token(); // skip open parentheses

    // for errors
    unsigned int errorLine = parser_get_token()->loc.line;

    int numberOfExpressions = 0;

    // if the function call expression list starts with a closed 
    // parentheses there are no expressions to parse
    if (parser_get_token()->type != _TokenType::CLOSE_PARAN)
    {
        while (parser_get_token()->type != _TokenType::END_OF_FILE)
        {
            auto exp = parser_parse_exp();

            if (exp->root == nullptr)
                CODE_ERROR("Expected an argument.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);

            functionCall->argExpressions.push_back(exp);
            ++numberOfExpressions;

            if (parser_get_token()->type == _TokenType::CLOSE_PARAN)
                break;

            if (parser_get_token()->type != _TokenType::COMMA)
                CODE_ERROR("Missing a comma to seperate function call arguments.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);
            
            parser_move_next_token();
        }

        if (parser_get_token()->type != _TokenType::CLOSE_PARAN)
            CODE_ERROR("Expected a closed parentheses.", parser_get_token()->loc.fileName, errorLine + 1);
    }

    parser_move_next_token();

    // function call names consist of the name of the function concatinated
    // with the number of arguments the function has been passed
    functionCall->name += std::to_string(numberOfExpressions);

    return functionCall;
}

/**
 * struct-`Name` {
 * 
 * <`name` -> null>...
 * 
 * <fn-`Name`-(? `symbol`...) { }>...
 * 
 * }
 */
StatementStruct* parser_parse_struct()
{
    parser_move_next_token();

    if (parser_get_token()->type != _TokenType::SYMBOL)
        CODE_ERROR("Expected a name for the structure.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);

    auto _struct = new StatementStruct(parser_get_token()->value);

    // move to the open bracket
    parser_move_next_token_skip_eols();

    if (parser_get_token()->type != _TokenType::OPEN_BRACKET)
        CODE_ERROR("Expected an open bracket.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);

    // move to the first statement
    parser_move_next_token_skip_eols();

    unsigned int errorLine = parser_get_token()->loc.line;

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
            CODE_ERROR("Invalid statement in structure.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);
        }

        if (parser_get_token()->type == _TokenType::CLOSE_BRACKET)
            break;

        parser_assert_end_of_statement();
        parser_move_next_token_skip_eols();
    }

    if (parser_get_token()->type != _TokenType::CLOSE_BRACKET)
        CODE_ERROR("Missing closing bracket on line.", parser_get_token()->loc.fileName, errorLine + 1);

    parser_move_next_token();

    return _struct;
}

/**
 * else {
 * 
 * }
 */
StatementElse* parser_parse_else()
{
    parser_move_next_token_skip_eols();
    return new StatementElse(parser_parse_block());
}

/**
 * if-`condition` {
 * 
 * } ? 
 * else-if-`condition` {
 * 
 * } ? 
 * else {
 * 
 * }
 */
StatementIf* parser_parse_if()
{
    parser_move_next_token();

    auto condition = parser_parse_exp();

    if (condition->root == nullptr)
        CODE_ERROR("Expected a condition.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);

    if (parser_get_token()->type == _TokenType::END_OF_LINE)
        parser_move_next_token_skip_eols();

    auto block = parser_parse_block();

    // if this if statement turns out to not have an 'else if' or
    // an 'else' statement chained to it, we want to make sure that
    // we can return to the end of this if's statement rather than
    // moving ahead to the next token. Statements should always
    // end with an END_OF_LINE token.
    unsigned int endOfIfPosition = parserPosition;

    if (parser_get_token()->type == _TokenType::END_OF_LINE)
        parser_move_next_token_skip_eols();

    auto _if = new StatementIf(condition, block, nullptr, nullptr);

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
        // if nothing is chained return to the end of the if's statement
        parserPosition = endOfIfPosition;
    }

    return _if;
}

/**
 * while-`condition` {
 * 
 * }
 */
Statement* parser_parse_while()
{
    parser_move_next_token();

    auto condition = parser_parse_exp();

    if (parser_get_token()->type == _TokenType::END_OF_LINE)
        parser_move_next_token_skip_eols();

    if (parser_get_token()->type != _TokenType::OPEN_BRACKET)
        CODE_ERROR("Missing an open bracket.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);

    return new StatementWhile(condition, parser_parse_block());
}

StatementBlockNamespace* parser_parse_namespace(const bool& topLevel)
{
    if (!topLevel)
        CODE_ERROR("Namespaces cannot be nested.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);
    
    parser_move_next_token();

    if (parser_get_token()->type != _TokenType::SYMBOL)
        CODE_ERROR("Expected a name for the structure.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);

    auto _namespace = new StatementBlockNamespace(parser_get_token()->value);

    parser_move_next_token_skip_eols();

    StatementBlock* block;

    if (parser_get_token()->type == _TokenType::OPEN_BRACKET)
        block = parser_parse_block();
    else
        // parse the block without brackets - basically means parse the entire file at this token
        block = parser_parse_block(false);

    for (auto& stmt : block->statements)
        _namespace->statements.push_back(stmt);

    delete block;

    return _namespace;
}

/**
 * Parses the next statement.
 */
Statement* parser_parse_a_single_statement(const bool& topLevel)
{
    // IF
    if (parser_get_token()->type == _TokenType::IF)
    {
        return parser_parse_if();
    }
    // WHILE
    else if (parser_get_token()->type == _TokenType::WHILE)
    {
        return parser_parse_while();
    }
    // FUNCTION CALL
    else if (parser_get_token()->type == _TokenType::SYMBOL && parser_peek_token()->type == _TokenType::OPEN_PARAN)
    {
        return parser_parse_function_call();
    }
    // EXPRESSIONS
    else if (parser_get_token()->type == _TokenType::NUMBER ||
        parser_get_token()->type == _TokenType::STRING ||
        parser_get_token()->type == _TokenType::SYMBOL ||
        parser_get_token()->type == _TokenType::MINUS ||
        parser_get_token()->type == _TokenType::OPEN_PARAN)
    {
        return parser_parse_exp();
    }
    // FUNCTIONS
    else if (parser_get_token()->type == _TokenType::FUNCITON)
    {
        return parser_parse_function();
    }
    // STRUCT
    else if (parser_get_token()->type == _TokenType::STRUCT)
    {
        return parser_parse_struct();
    }
    // RETURNS
    else if (parser_get_token()->type == _TokenType::RETURN)
    {
        parser_move_next_token();
        return new StatementReturn(parser_parse_exp());
    }
    // NAMESPACE
    else if (parser_get_token()->type == _TokenType::NAMESPACE)
    {
        return parser_parse_namespace(topLevel);
    }

    return nullptr;
}

#pragma endregion

/**
 * Entry point for parsing a list of parserTokens.
 */
void parser_get_syntax_tree(SyntaxTree& tree, const std::vector<Token*>& tokens)
{
    parserPosition = 0;
    parserTokens = tokens;
    statementLocationMetadata = parser_get_token()->loc;

    // read until the end of the file
    while (parser_get_token()->type != _TokenType::END_OF_FILE)
    {
        if (auto stmt = parser_parse_a_single_statement(true))
            tree.statements.push_back(stmt);

        parser_assert_end_of_statement();
        parser_move_next_token();
    }
}

#pragma endregion

#endif // COMPILER