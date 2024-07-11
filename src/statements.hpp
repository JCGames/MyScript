#ifndef STATEMENTS
#define STATEMENTS

#include <iostream>
#include <string>
#include <vector>

#include "enums/statement-type.hpp"
#include "enums/operator-type.hpp"
#include "enums/data-type.hpp"

struct Statement
{
    StatementType type;

    Statement(const StatementType& type)
    {
        this->type = type;
    }

    virtual void print(std::string padding)
    {
        print_type(padding);
    }

    virtual void _delete() { }

protected:
    void print_type(std::string padding)
    {
        std::cout << padding << statement_type_name(type) << std::endl;
    }
};

struct SyntaxTree
{
    std::vector<std::string> files;
    std::vector<Statement*> statements;

    ~SyntaxTree()
    {
        for (auto& stmt : statements)
        {
            if (stmt != nullptr)
            {
                stmt->_delete();
                delete stmt;
            }
        }
    }

    void print()
    {
        std::cout << "Syntax Tree: [" << std::endl;

        for (auto& stmt : statements)
            stmt->print("\t");

        std::cout << "]" << std::endl;
    }
};

struct StatementExpression : Statement
{
    Statement* root;

    StatementExpression(Statement* root) : Statement(StatementType::EXPRESSION)
    {
        this->root = root;
    };

    void _delete() override
    {
        delete root;
    }

    void print(std::string padding) override
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

    void _delete() override
    {
        delete left;
        delete right;
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
    Statement* root;

    StatementUnaryOperator(const OperatorType& operatorType, Statement* root) : Statement(StatementType::UNARY_OPERATOR)
    {
        this->operatorType = operatorType;
        this->root = root;
    }

    void _delete() override
    {
        delete root;
    }

    void print(std::string padding) override
    {
        print_type(padding);

        std::cout << padding << operator_type_name(operatorType) << std::endl;

        root->print(padding + '\t');
    }
};

struct StatementValue : Statement
{
    DataType dataType;
    std::string value;

    StatementValue(const DataType& dataType, const std::string& value) : Statement(StatementType::VALUE)
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
    std::string symbol;

    StatementSymbol(const std::string& symbol) : Statement(StatementType::SYMBOL)
    {
        this->symbol = symbol;
    }

    void print(std::string padding) override
    {
        print_type(padding);
        std::cout << padding << symbol << std::endl;
    }
};

struct StatementBlock : Statement
{
    std::vector<Statement*> statements;

    StatementBlock() : Statement(StatementType::BLOCK) { }

    StatementBlock(bool isNamespace) : Statement(StatementType::NAMESPACE) { }

    void _delete() override
    {
        for (auto& stmt : statements)
            delete stmt;
    }

    void print(std::string padding) override
    {
        print_type(padding);
        std::cout << padding << "[" << std::endl;

        for (auto stmt : statements)
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

        std::cout << padding << name << " ";
        std::cout << "[" << std::endl;

        for (auto stmt : statements)
            stmt->print(padding + '\t');

        std::cout << padding << "]" << std::endl;
    }
};

struct StatementFunction : Statement
{
    std::string name;
    std::vector<StatementSymbol*> params;
    StatementBlock* body;

    StatementFunction(std::string name) : Statement(StatementType::FUNCTION) 
    {
        this->name = name;
    }

    void _delete() override
    {        
        for (auto& stmt : params)
            delete stmt;
        
        delete body;
    }

    void print(std::string padding) override
    {
        print_type(padding);

        std::cout << padding << "Name: " << this->name << std::endl;
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

    void _delete() override
    {
        delete expression;
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
    std::vector<StatementExpression*> argExpressions;

    StatementFunctionCall(std::string name) : Statement(StatementType::FUNCTION_CALL)
    {
        this->name = name;
    }

    void _delete() override
    {
        for (auto& stmt : argExpressions)
            delete stmt;
    }

    void print(std::string padding) override
    {
        print_type(padding);

        std::cout << padding << "Name: " << this->name << std::endl;
        std::cout << padding << "Arg Expressions: [" << std::endl;

        for (auto& stmt : argExpressions)
            stmt->print(padding + '\t');

        std::cout << padding << "]" << std::endl; 
    }
};

struct StatementStruct : Statement
{
    std::string name;
    std::vector<StatementExpression*> variables;
    std::vector<StatementFunction*> functions;

    StatementStruct(std::string name) : Statement(StatementType::STRUCT)
    {
        this->name = name;
    }

    void _delete() override
    {
        for (auto& stmt : variables)
            delete stmt;

        for (auto& stmt : functions)
            delete stmt;
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
    StatementBlock* block;

    StatementElse(StatementBlock* block) : Statement(StatementType::ELSE)
    {
        this->block = block;
    }

    void _delete() override
    {
        delete this->block;
    }

    void print(std::string padding) override
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

    void _delete() override
    {
        delete this->condition;
        delete this->block;

        if (this->_elseIf != nullptr)
            delete this->_elseIf;

        if (this->_else != nullptr)
            delete this->_else;
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
    StatementExpression* condition;
    StatementBlock* block;

    StatementWhile(StatementExpression* condition, StatementBlock* block) : Statement(StatementType::WHILE)
    {
        this->condition = condition;
        this->block = block;
    }

    void _delete() override
    {
        delete this->condition;
        delete this->block;
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

#endif