#ifndef STATEMENTS
#define STATEMENTS

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "enums/statement-type.hpp"
#include "enums/operator-type.hpp"
#include "enums/data-type.hpp"

#include "location-metadata.hpp"

#define U(type) std::unique_ptr<type>
#define U_INIT(type, value) std::unique_ptr<type>(value)

static LocationMetadata statementLocationMetadata;

struct Statement
{
    StatementType type;
    LocationMetadata loc;

    Statement(const StatementType& type)
    {
        this->type = type;
        this->loc = statementLocationMetadata;
    }

    virtual void print(std::string padding)
    {
        print_type(padding);
    }

protected:
    void print_type(std::string padding)
    {
        std::cout << padding << "\x1B[34m" << statement_type_name(type) << "\033[0m [";
        std::cout << loc.fileName << ":";
        std::cout << loc.line + 1 << "]" << std::endl;
    }
};

struct StatementExpression : Statement
{
    U(Statement) root;

    StatementExpression(Statement* root) : Statement(StatementType::EXPRESSION)
    {
        this->root = U_INIT(Statement, root);
    };

    void print(std::string padding) override
    {
        print_type(padding);
        root->print(padding + '\t');
    }
};

struct StatementBinaryOperator : Statement
{
    OperatorType operatorType;
    U(Statement) left;
    U(Statement) right;

    StatementBinaryOperator(const OperatorType& operatorType, Statement* left, Statement* right) : Statement(StatementType::BINARY_OPERATOR)
    {
        this->operatorType = operatorType;
        this->left = U_INIT(Statement, left);
        this->right = U_INIT(Statement, right);
    }

    void print(std::string padding) override
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
    U(Statement) root;

    StatementUnaryOperator(const OperatorType& operatorType, Statement* root) : Statement(StatementType::UNARY_OPERATOR)
    {
        this->operatorType = operatorType;
        this->root = U_INIT(Statement, root);
    }

    void print(std::string padding) override
    {
        print_type(padding);

        std::cout << padding << operator_type_name(operatorType) << std::endl;

        root->print(padding + '\t');
    }
};

struct StatementConstant : Statement
{
    DataType dataType;
    std::string value;

    StatementConstant(const DataType& dataType, const std::string& value) : Statement(StatementType::CONSTANT)
    {
        this->dataType = dataType;
        this->value = value;
    }

    void print(std::string padding) override
    {
        print_type(padding);
        std::cout << padding << data_type_name(dataType) << " " << value << std::endl;
    }
};

struct StatementSymbol : Statement
{
    std::string value;

    StatementSymbol(const std::string& value) : Statement(StatementType::SYMBOL)
    {
        this->value = value;
    }

    void print(std::string padding) override
    {
        print_type(padding);
        std::cout << padding << value << std::endl;
    }
};

struct StatementBlock : Statement
{
    std::vector<U(Statement)> statements;

    StatementBlock() : Statement(StatementType::BLOCK) { }

    StatementBlock(bool isNamespace) : Statement(StatementType::NAMESPACE) { }

    void print(std::string padding) override
    {
        print_type(padding);

        std::cout << padding << "[" << std::endl;

        for (auto& stmt : statements)
            stmt->print(padding + '\t');

        std::cout << padding << "]" << std::endl;
    }
};

struct StatementBlockNamespace : StatementBlock
{
    std::string name;

    StatementBlockNamespace(std::string name) : StatementBlock(true)
    {
        this->name = name;
    }

    void print(std::string padding) override
    {
        print_type(padding);

        std::cout << padding << "Name: " << name << " ";
        std::cout << "[" << std::endl;

        for (auto& stmt : statements)
            stmt->print(padding + '\t');

        std::cout << padding << "]" << std::endl;
    }
};

struct StatementFunction : Statement
{
    std::string name;
    std::vector<U(StatementSymbol)> params;
    U(StatementBlock) body;

    StatementFunction(std::string name) : Statement(StatementType::FUNCTION) 
    {
        this->name = name;
    }

    void print(std::string padding) override
    {
        print_type(padding);

        std::cout << padding << "Name: " << this->name << std::endl;
        std::cout << padding << "Params: [" << std::endl;

        for (auto& stmtSymbol : params)
            std::cout << padding << '\t' << stmtSymbol->value << std::endl;

        std::cout << padding << "]" << std::endl;
        std::cout << padding << "Function Block:" << std::endl;

        if (body != nullptr)
            body->print(padding + '\t');
    }
};

struct StatementReturn : Statement
{
    U(StatementExpression) expression;

    StatementReturn(StatementExpression* expression) : Statement(StatementType::RETURN)
    {
        this->expression = U_INIT(StatementExpression, expression);
    }

    void print(std::string padding) override
    {
        print_type(padding);
        expression->print(padding + '\t');
    }
};

struct StatementFunctionCall : Statement
{
    std::string name;
    std::vector<U(StatementExpression)> args;

    StatementFunctionCall(std::string name) : Statement(StatementType::FUNCTION_CALL)
    {
        this->name = name;
    }

    void print(std::string padding) override
    {
        print_type(padding);

        std::cout << padding << "Name: " << this->name << std::endl;
        std::cout << padding << "Arg Expressions: [" << std::endl;

        for (auto& stmt : args)
            stmt->print(padding + '\t');

        std::cout << padding << "]" << std::endl; 
    }
};

struct StatementStruct : Statement
{
    std::string name;
    std::vector<U(StatementExpression)> variables;
    std::vector<U(StatementFunction)> functions;

    StatementStruct(std::string name) : Statement(StatementType::STRUCT)
    {
        this->name = name;
    }

    void print(std::string padding) override
    {
        print_type(padding);

        std::cout << padding << "Name: " << this->name << std::endl;
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
    U(StatementBlock) block;

    StatementElse(StatementBlock* block) : Statement(StatementType::ELSE)
    {
        this->block = U_INIT(StatementBlock, block);
    }

    void print(std::string padding) override
    {
        print_type(padding);
        block->print(padding + '\t');
    }
};

struct StatementIf : Statement
{
    U(StatementExpression) condition;
    U(StatementBlock) block;
    U(StatementIf) _elseIf;
    U(StatementElse) _else;

    StatementIf(StatementExpression* condition, StatementBlock* block, StatementIf* _elseIf, StatementElse* _else) : Statement(StatementType::IF)
    {
        this->condition = U_INIT(StatementExpression, condition);
        this->block = U_INIT(StatementBlock, block);
        this->_elseIf = U_INIT(StatementIf, _elseIf);
        this->_else = U_INIT(StatementElse, _else);
    }

    void print(std::string padding) override
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

struct StatementWhile : Statement
{
    U(StatementExpression) condition;
    U(StatementBlock) block;

    StatementWhile(StatementExpression* condition, StatementBlock* block) : Statement(StatementType::WHILE)
    {
        this->condition = U_INIT(StatementExpression, condition);
        this->block = U_INIT(StatementBlock, block);
    }

    void print(std::string padding) override
    {
        print_type(padding);

        std::cout << padding << "Condition: [" << std::endl;

        condition->print(padding + '\t');

        std::cout << padding << "]" << std::endl;

        block->print(padding + '\t');
    }
};

struct SyntaxTree
{
    std::vector<U(Statement)> statements;

    void print()
    {
        std::cout << "Syntax Tree: [" << std::endl;

        for (auto& stmt : statements)
            stmt->print("\t");

        std::cout << "]" << std::endl;
    }
};

StatementBinaryOperator* statement_as_binary_operator(Statement* stmt, OperatorType type)
{
    if (stmt->type == StatementType::BINARY_OPERATOR)
    {
        auto binaryOperator = (StatementBinaryOperator*)stmt;

        if (binaryOperator->operatorType == type)
            return binaryOperator;
    }

    return nullptr;
}

StatementUnaryOperator* statement_as_unary_operator(Statement* stmt, OperatorType type)
{
    if (stmt->type == StatementType::BINARY_OPERATOR)
    {
        auto binaryOperator = (StatementUnaryOperator*)stmt;

        if (binaryOperator->operatorType == type)
            return binaryOperator;
    }

    return nullptr;
}

#undef U
#undef U_INIT

#endif