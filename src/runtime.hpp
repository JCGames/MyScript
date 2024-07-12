#ifndef RUNTIME
#define RUNTIME

#include "static-analysis.hpp"
#include "environment.hpp"

Environment globalEnvironment;
std::stack<Object*> stack;
Statement* currentStmtBeingExecuted = nullptr;

void rt_error(const std::string& msg)
{
    CODE_ERROR(msg, currentStmtBeingExecuted->loc.fileName, currentStmtBeingExecuted->loc.line + 1);
}

Object* stack_pop()
{
    if (stack.size() <= 0)
        rt_error("There is no answer for you to pop off the stack.");

    auto value = stack.top();
    stack.pop();

    return value;
}

void rt_add_objs()
{
    auto right = stack_pop();
    auto left = stack_pop();

    if (left->get_type() != right->get_type())
        rt_error("Cannot add objects of different types.");

    if (left->get_type() == ObjectType::INT)
    {
        stack.push(new Object(new int(*(int*)left->get_value() + *(int*)right->get_value())));
    }
    else if (left->get_type() == ObjectType::FLOAT)
    {
        stack.push(new Object(new float(*(float*)left->get_value() + *(float*)right->get_value())));
    }
    else if (left->get_type() == ObjectType::BOOL)
    {
        stack.push(new Object(new bool(*(bool*)left->get_value() + *(bool*)right->get_value())));
    }
    else if (left->get_type() == ObjectType::STRING)
    {
        stack.push(new Object(new std::string(*(std::string*)left->get_value() + *(std::string*)right->get_value())));
    }

    delete right;
    delete left;
}

void rt_sub_objs()
{
    auto right = stack_pop();
    auto left = stack_pop();

    if (left->get_type() != right->get_type())
        rt_error("Cannot subtract objects of different types.");

    if (left->get_type() == ObjectType::INT)
    {
        stack.push(new Object(new int(*(int*)left->get_value() - *(int*)right->get_value())));
    }
    else if (left->get_type() == ObjectType::FLOAT)
    {
        stack.push(new Object(new float(*(float*)left->get_value() - *(float*)right->get_value())));
    }
    else if (left->get_type() == ObjectType::BOOL)
    {
        stack.push(new Object(new bool(*(bool*)left->get_value() - *(bool*)right->get_value())));
    }
    else if (left->get_type() == ObjectType::STRING)
    {
        rt_error("Cannot subtract strings.");
    }

    delete right;
    delete left;
}

void rt_run_exp(Statement* exp, Environment& env)
{
    switch (exp->type)
    {   
        case StatementType::EXPRESSION:
            rt_run_exp(((StatementExpression*)exp)->root, env);
            break;
        case StatementType::BINARY_OPERATOR:
        {
            auto binaryOperator = (StatementBinaryOperator*)exp;

            switch (binaryOperator->operatorType)
            {
                case OperatorType::ADDITION:
                {
                    rt_run_exp(binaryOperator->left, env);
                    rt_run_exp(binaryOperator->right, env);
                    rt_add_objs();
                }
                    break;
                case OperatorType::SUBTRACTION:
                {
                    rt_run_exp(binaryOperator->left, env);
                    rt_run_exp(binaryOperator->right, env);
                    rt_sub_objs();
                }
                    break;
            }
        }
            break;
        case StatementType::UNARY_OPERATOR:
        {

        }
            break;
        case StatementType::CONSTANT:
        {
            auto constant = (StatementConstant*)exp;

            if (constant->dataType == DataType::INT)
            {
                stack.push(new Object(new int(std::stoi(constant->value))));
            }
            else if (constant->dataType == DataType::FLOAT)
            {
                stack.push(new Object(new float(std::stof(constant->value))));
            }
            else if (constant->dataType == DataType::BOOL)
            {
                stack.push(new Object(new bool(std::stoi(constant->value))));
            }
            else if (constant->dataType == DataType::STRING)
            {
                stack.push(new Object(new std::string(constant->value)));
            }
        }
            break;
        case StatementType::SYMBOL:
        {

        }
            break;
    }
}

void rt_run_stmt(Statement* stmt, Environment& env)
{
    currentStmtBeingExecuted = stmt;

    switch (stmt->type)
    {
        case StatementType::EXPRESSION:
        {
            rt_run_exp(stmt, env);

            auto result = stack_pop();
            std::cout << result->to_string() << std::endl;
            delete result;
        }
            break;
    }
}

void rt_run_tree(SyntaxTree& tree)
{
    globalEnvironment.map_block(tree.statements);

    for (auto& stmt : tree.statements)
    {
        rt_run_stmt(stmt, globalEnvironment);
    }
}

#endif