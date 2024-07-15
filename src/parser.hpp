#ifndef COMPILER
#define COMPILER

#include <stack>
#include <memory>

#include "enums/token-type.hpp"
#include "enums/statement-type.hpp"
#include "enums/operator-type.hpp"
#include "enums/data-type.hpp"
#include "statements.hpp"
#include "lexer.hpp"

unsigned int parserPosition = 0;
unsigned int parserUniqueInt = 0;
std::vector<Token*> parserTokens;

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

/**
 * Throws an error if the current token is not of the given type.
 */
void parser_assert_token_is(_TokenType type, const std::string& expectedValue)
{
    if (parser_get_token()->type != type)
    {
        CODE_ERROR("Expected a " + expectedValue + " but got |" + parser_get_token()->value + "|", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);
    }
}

/**
 * Throws an error if the current token is not of the given type on the given line.
 */
void parser_assert_token_is(_TokenType type, const std::string& expectedValue, const unsigned int& line)
{
    if (parser_get_token()->type != type)
    {
        CODE_ERROR("Expected a " + expectedValue + " but got " + parser_get_token()->value, parser_get_token()->loc.fileName, line);
    }
}

/**
 * Statements should end with an END_OF_LINE or END_OF_FILE token.
 */
void parser_assert_end_of_statement()
{
    if (parser_get_token()->type != _TokenType::END_OF_LINE && parser_get_token()->type != _TokenType::END_OF_FILE)
        CODE_ERROR("Expected the end of a statment but got |" + parser_get_token()->value + "|.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);
}

/**
 * Throws an error if the current token is not of the given type.
 */
void parser_assert_stmt_is_not_null(const Statement* stmt, const std::string& expectedValue)
{
    if (stmt == nullptr)
    {
        CODE_ERROR("Expected a " + expectedValue + " but got |" + parser_get_token()->value + "|", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);
    }
}

/**
 * Raise an error if a statement in a block is invalid.
 */
void parser_error_invalid_stmt(const std::string& inWhat)
{
    CODE_ERROR("Invalid statement in " + inWhat + ".", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);
}

/**
 * Raise an error if the statement could not be parsed because it was unknown.
 */
void parser_raise_error_unknown_stmt()
{
    CODE_ERROR("Unknown statment |" + parser_get_token()->value + "|", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);
}

#pragma region Expressions

std::unique_ptr<StatementExpression> parser_parse_exp();
std::unique_ptr<StatementFunctionCall> parser_parse_function_call();
std::unique_ptr<StatementFunction> parser_parse_function(const bool& isLambda = false, const bool& isContainedWithinStruct = false);

std::unique_ptr<Statement> parser_parse_exp_term()
{
    // FUNCTION CALL
    if (parser_get_token()->type == _TokenType::SYMBOL && parser_peek_token()->type == _TokenType::OPEN_PARAN)
    {
        auto function = parser_parse_function_call();
        return function;
    }
    // NUMBER
    else if (parser_get_token()->type == _TokenType::NUMBER)
    {
        bool isDecimal = false;
        for (char c : parser_get_token()->value)
        {
            // is decimal?
            if (c == '.')
                isDecimal = true;
        }

        auto number = isDecimal ?
            new StatementConstant(DataType::FLOAT, parser_get_token()->value) :
            new StatementConstant(DataType::INT, parser_get_token()->value);

        parser_move_next_token();
        return std::unique_ptr<Statement>(number);
    }
    // STRING
    else if (parser_get_token()->type == _TokenType::STRING)
    {
        auto result = new StatementConstant(DataType::STRING, parser_get_token()->value);
        parser_move_next_token();
        return std::unique_ptr<Statement>(result);
    }
    // SYMBOL
    else if (parser_get_token()->type == _TokenType::SYMBOL)
    {
        auto result = new StatementSymbol(parser_get_token()->value);
        parser_move_next_token();
        return std::unique_ptr<Statement>(result);
    }
    // NEGATE
    else if (parser_get_token()->type == _TokenType::MINUS)
    {
        parser_move_next_token();
        return std::unique_ptr<Statement>(new StatementUnaryOperator(OperatorType::NEGATE, parser_parse_exp_term().release()));
    }
    // INNER EXPRESSIONS
    else if (parser_get_token()->type == _TokenType::OPEN_PARAN)
    {
        unsigned int errorLine = parser_get_token()->loc.line;

        parser_move_next_token();

        auto result = parser_parse_exp();

        parser_assert_token_is(_TokenType::CLOSE_PARAN, ")", errorLine + 1);
        parser_move_next_token();
        return result;
    }
    // NULL
    else if (parser_get_token()->type == _TokenType::_NULL)
    {
        auto result = new StatementConstant(DataType::_NULL, EMPTY_STRING);
        parser_move_next_token();
        return std::unique_ptr<Statement>(result);
    }
    // TRUE
    else if (parser_get_token()->type == _TokenType::_TRUE)
    {
        auto result = new StatementConstant(DataType::BOOL, "1");
        parser_move_next_token();
        return std::unique_ptr<Statement>(result);
    }
    // FALSE
    else if (parser_get_token()->type == _TokenType::_FALSE)
    {
        auto result = new StatementConstant(DataType::BOOL, "0");
        parser_move_next_token();
        return std::unique_ptr<Statement>(result);
    }
    // LAMBDA FUNCTION
    else if (parser_get_token()->type == _TokenType::FUNCTION)
    {
        return parser_parse_function(/*isLambda*/ true);
    }

    parser_error_invalid_stmt("Expression Term");
    return nullptr;
}

std::unique_ptr<Statement> parser_parse_exp_dot()
{
    std::stack<Statement*> stmtStack;
    stmtStack.push(parser_parse_exp_term().release());

    while (parser_get_token()->type == _TokenType::DOT)
    {
        parser_move_next_token();
        stmtStack.push(parser_parse_exp_term().release());
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

        return std::unique_ptr<Statement>(binaryOperator);
    }
    else if (stmtStack.size() == 1)
    {
        return std::unique_ptr<Statement>(stmtStack.top());
    }

    parser_error_invalid_stmt("Member Accessor");
    return nullptr;
}

std::unique_ptr<Statement> parser_parse_exp_muldivmod()
{
    Statement* left = parser_parse_exp_dot().release();

    while (parser_get_token()->type == _TokenType::MULTIPLY ||
        parser_get_token()->type == _TokenType::DIVIDE ||
        parser_get_token()->type == _TokenType::MODULUS)
    {
        if (parser_get_token()->type == _TokenType::MULTIPLY)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::MULTIPLICATION, left, parser_parse_exp_dot().release());
        }
        else if (parser_get_token()->type == _TokenType::DIVIDE)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::DIVISION, left, parser_parse_exp_dot().release());
        }
        else if (parser_get_token()->type == _TokenType::MODULUS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::MODULUS, left, parser_parse_exp_dot().release());
        }
    }

    return std::unique_ptr<Statement>(left);
}

std::unique_ptr<Statement> parser_parse_exp_addsub()
{
    Statement* left = parser_parse_exp_muldivmod().release();

    while (parser_get_token()->type == _TokenType::PLUS ||
        parser_get_token()->type == _TokenType::MINUS)
    {
        if (parser_get_token()->type == _TokenType::PLUS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::ADDITION, left, parser_parse_exp_muldivmod().release());
        }
        else if (parser_get_token()->type == _TokenType::MINUS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::SUBTRACTION, left, parser_parse_exp_muldivmod().release());
        }
    }

    return std::unique_ptr<Statement>(left);
}

std::unique_ptr<Statement> parser_parse_equality()
{
    Statement* left = parser_parse_exp_addsub().release();

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
            left = new StatementBinaryOperator(OperatorType::EQUALS, left, parser_parse_exp_addsub().release());
        }
        else if (parser_get_token()->type == _TokenType::NOT_EQUALS)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::NOT_EQUALS, left, parser_parse_exp_addsub().release());
        }
        else if (parser_get_token()->type == _TokenType::GREATER_THAN)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::GREATER_THAN, left, parser_parse_exp_addsub().release());
        }
        else if (parser_get_token()->type == _TokenType::LESS_THAN)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::LESS_THAN, left, parser_parse_exp_addsub().release());
        }
        else if (parser_get_token()->type == _TokenType::GREATER_THAN_E)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::GREATER_THAN_E, left, parser_parse_exp_addsub().release());
        }
        else if (parser_get_token()->type == _TokenType::LESS_THAN_E)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::LESS_THAN_E, left, parser_parse_exp_addsub().release());
        }
    }

    return std::unique_ptr<Statement>(left);
}

std::unique_ptr<Statement> parser_parse_andor()
{
    Statement* left = parser_parse_equality().release();

    while (parser_get_token()->type == _TokenType::AND ||
        parser_get_token()->type == _TokenType::OR)
    {
        if (parser_get_token()->type == _TokenType::AND)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::AND, left, parser_parse_equality().release());
        }
        else if (parser_get_token()->type == _TokenType::OR)
        {
            parser_move_next_token();
            left = new StatementBinaryOperator(OperatorType::OR, left, parser_parse_equality().release());
        }
    }

    return std::unique_ptr<Statement>(left);
}

std::unique_ptr<Statement> parser_parse_exp_assign()
{
    std::stack<Statement*> stmtStack;
    stmtStack.push(parser_parse_andor().release());

    while (parser_get_token()->type == _TokenType::ASSIGN)
    {
        parser_move_next_token();
        stmtStack.push(parser_parse_andor().release());
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

        return std::unique_ptr<StatementBinaryOperator>(binaryOperator);
    }
    else if (stmtStack.size() == 1)
    {
        return std::unique_ptr<Statement>(stmtStack.top());
    }

    parser_error_invalid_stmt("Assignment");
    return nullptr;
}

/**
 * Always ends on the next token.
 */
std::unique_ptr<StatementExpression> parser_parse_exp()
{
    return std::unique_ptr<StatementExpression>(new StatementExpression(parser_parse_exp_assign().release()));
}

#pragma endregion

#pragma region Statements

std::unique_ptr<Statement> parser_parse_a_single_statement(const bool& topLevel = false);

/**
 * Parses a block.
 * @param withBrackets if true, will read until a closed bracket, and if false, will read until an END_OF_FILE token.
 */
std::unique_ptr<StatementBlock> parser_parse_block(const bool& withBrackets = true)
{
    auto block = new StatementBlock();

    // for errors
    auto errorLine = parser_get_token()->loc.line;

    if (withBrackets)
    {
        parser_assert_token_is(_TokenType::OPEN_BRACKET, "{");
        parser_move_next_token();
    }

    // read statements consecutively until reaching a close bracket
    while (parser_get_token()->type != _TokenType::END_OF_FILE)
    {
        if (auto stmt = parser_parse_a_single_statement())
            block->statements.push_back(std::unique_ptr<Statement>(stmt.release()));

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
        parser_assert_token_is(_TokenType::CLOSE_BRACKET, "}");
        parser_move_next_token();
    }

    return std::unique_ptr<StatementBlock>(block);
}

/**
 * fn-`Name`-(? `symbol`... , ) {
 * 
 * }-EOL
 */
std::unique_ptr<StatementFunction> parser_parse_function(const bool& isLambda, const bool& isContainedWithinStruct)
{
    auto function = new StatementFunction(EMPTY_STRING);

    if (!isLambda)
    {
        parser_move_next_token();

        if (isContainedWithinStruct &&
            (parser_get_token()->type == _TokenType::PLUS ||
            parser_get_token()->type == _TokenType::MINUS ||
            parser_get_token()->type == _TokenType::MULTIPLY ||
            parser_get_token()->type == _TokenType::DIVIDE ||
            parser_get_token()->type == _TokenType::EQUALS ||
            parser_get_token()->type == _TokenType::NOT_EQUALS ||
            parser_get_token()->type == _TokenType::GREATER_THAN_E ||
            parser_get_token()->type == _TokenType::LESS_THAN_E ||
            parser_get_token()->type == _TokenType::GREATER_THAN ||
            parser_get_token()->type == _TokenType::LESS_THAN))
        {
            // function is an operator function
        }
        else
        {
            parser_assert_token_is(_TokenType::SYMBOL, "Function Name");
        }

        function->name = parser_get_token()->value;
    }
    else
    {
        function->name = std::to_string(parser_get_next_unique_int()) + "_";
    }

    parser_move_next_token();
    parser_assert_token_is(_TokenType::OPEN_PARAN, "(");
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
            parser_assert_token_is(_TokenType::SYMBOL, "Function Parameter");

            function->params.push_back(std::unique_ptr<StatementSymbol>(new StatementSymbol(parser_get_token()->value)));
            ++numberOfParams;
            parser_move_next_token();

            if (parser_get_token()->type == _TokenType::CLOSE_PARAN)
                break;

            parser_assert_token_is(_TokenType::COMMA, "Comma");
            parser_move_next_token();
        }

        parser_assert_token_is(_TokenType::CLOSE_PARAN, ")", errorLine + 1);
    }

    parser_move_next_token_skip_eols();

    // function names consist of the name of the function concatinated
    // with the number of parameters the function has
    function->name += std::to_string(numberOfParams);

    function->body = std::unique_ptr<StatementBlock>(parser_parse_block());

    return std::unique_ptr<StatementFunction>(function);
}

/**
 * `FunctionName`-(? `expression`... , )-EOL
 */
std::unique_ptr<StatementFunctionCall> parser_parse_function_call()
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

            parser_assert_stmt_is_not_null(exp.get(), "Function Call Argument");

            functionCall->args.push_back(std::unique_ptr<StatementExpression>(exp.release()));
            ++numberOfExpressions;

            if (parser_get_token()->type == _TokenType::CLOSE_PARAN)
                break;

            parser_assert_token_is(_TokenType::COMMA, "Comma");
            parser_move_next_token();
        }

        parser_assert_token_is(_TokenType::CLOSE_PARAN, ")", errorLine + 1);
    }

    parser_move_next_token();

    // function call names consist of the name of the function concatinated
    // with the number of arguments the function has been passed
    functionCall->name += std::to_string(numberOfExpressions);

    return std::unique_ptr<StatementFunctionCall>(functionCall);
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
std::unique_ptr<StatementStruct> parser_parse_struct()
{
    parser_move_next_token();
    parser_assert_token_is(_TokenType::SYMBOL, "Structure Name");

    auto _struct = new StatementStruct(parser_get_token()->value);

    // move to the open bracket
    parser_move_next_token_skip_eols();
    parser_assert_token_is(_TokenType::OPEN_BRACKET, "{");

    // move to the first statement
    parser_move_next_token_skip_eols();

    unsigned int errorLine = parser_get_token()->loc.line;

    // parse variables and functions in structure
    while (parser_get_token()->type != _TokenType::CLOSE_BRACKET &&
        parser_get_token()->type != _TokenType::END_OF_FILE)
    {
        // VARIABLE ASSIGNMENT
        if (parser_get_token()->type == _TokenType::SYMBOL && parser_peek_token()->type == _TokenType::ASSIGN)
        {
            _struct->variables.push_back(std::unique_ptr<StatementExpression>(parser_parse_exp()));
        }
        // FUNCTION
        else if (parser_get_token()->type == _TokenType::FUNCTION)
        {
            _struct->functions.push_back(
                std::unique_ptr<StatementFunction>(
                    parser_parse_function(/*isLambda*/ false, /*isContainedWithinStruct*/ true)
                    )
                );
        }
        else 
        {
            parser_error_invalid_stmt("Structure");
        }

        if (parser_get_token()->type == _TokenType::CLOSE_BRACKET)
            break;

        parser_assert_end_of_statement();
        parser_move_next_token_skip_eols();
    }

    parser_assert_token_is(_TokenType::CLOSE_BRACKET, "}", errorLine + 1);
    parser_move_next_token();

    return std::unique_ptr<StatementStruct>(_struct);
}

/**
 * else {
 * 
 * }
 */
std::unique_ptr<StatementElse> parser_parse_else()
{
    parser_move_next_token_skip_eols();
    return std::unique_ptr<StatementElse>(new StatementElse(parser_parse_block().release()));
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
std::unique_ptr<StatementIf> parser_parse_if()
{
    parser_move_next_token();

    auto condition = parser_parse_exp();

    parser_assert_stmt_is_not_null(condition.get(), "Condition");

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

    auto _if = new StatementIf(condition.release(), block.release(), nullptr, nullptr);

    if (parser_get_token()->type == _TokenType::ELSE && parser_peek_token()->type == _TokenType::IF)
    {
        parser_move_next_token();
        _if->_elseIf = std::unique_ptr<StatementIf>(parser_parse_if());
    }
    else if (parser_get_token()->type == _TokenType::ELSE)
    {
        _if->_else = std::unique_ptr<StatementElse>(parser_parse_else());
    }
    else
    {
        // if nothing is chained return to the end of the if's statement
        parserPosition = endOfIfPosition;
    }

    return std::unique_ptr<StatementIf>(_if);
}

/**
 * while-`condition` {
 * 
 * }
 */
std::unique_ptr<StatementWhile> parser_parse_while()
{
    parser_move_next_token();

    auto condition = parser_parse_exp();

    parser_assert_stmt_is_not_null(condition.get(), "Condition");

    if (parser_get_token()->type == _TokenType::END_OF_LINE)
        parser_move_next_token_skip_eols();

    return std::unique_ptr<StatementWhile>(new StatementWhile(condition.release(), parser_parse_block().release()));
}

std::unique_ptr<StatementBlockNamespace> parser_parse_namespace(const bool& topLevel)
{
    if (!topLevel)
        CODE_ERROR("Namespaces cannot be nested.", parser_get_token()->loc.fileName, parser_get_token()->loc.line + 1);
    
    parser_move_next_token();
    parser_assert_token_is(_TokenType::SYMBOL, "Structure Name");

    auto _namespace = new StatementBlockNamespace(parser_get_token()->value);

    parser_move_next_token_skip_eols();

    StatementBlock* block;

    if (parser_get_token()->type == _TokenType::OPEN_BRACKET)
        block = parser_parse_block().release();
    else
        // parse the block without brackets - basically means parse the entire file at this token
        block = parser_parse_block(/*withBrackets*/ false).release();

    for (auto& stmt : block->statements)
        _namespace->statements.push_back(std::unique_ptr<Statement>(stmt.release()));

    delete block;

    return std::unique_ptr<StatementBlockNamespace>(_namespace);
}

/**
 * Parses the next statement.
 */
std::unique_ptr<Statement> parser_parse_a_single_statement(const bool& topLevel)
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
        parser_get_token()->type == _TokenType::OPEN_PARAN ||
        parser_get_token()->type == _TokenType::_TRUE ||
        parser_get_token()->type == _TokenType::_FALSE)
    {
        return parser_parse_exp();
    }
    // FUNCTIONS
    else if (parser_get_token()->type == _TokenType::FUNCTION)
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
        return std::unique_ptr<StatementReturn>(new StatementReturn(parser_parse_exp().release()));
    }
    // NAMESPACE
    else if (parser_get_token()->type == _TokenType::NAMESPACE)
    {
        return parser_parse_namespace(topLevel);
    }
    // WHITESPACE
    else if (parser_get_token()->type == _TokenType::END_OF_LINE || 
        parser_get_token()->type == _TokenType::END_OF_FILE)
    {
        return nullptr;
    }
    // TODO: figure out a better way to not throw if in block and close bracket is encountered
    else if (parser_get_token()->type == _TokenType::CLOSE_BRACKET)
    {
        return nullptr;
    }

    parser_raise_error_unknown_stmt();
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
            tree.statements.push_back(std::unique_ptr<Statement>(stmt.release()));

        parser_assert_end_of_statement();
        parser_move_next_token();
    }
}

#endif // COMPILER