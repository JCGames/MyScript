#ifndef OBJECT
#define OBJECT

#include <memory>

#include "compiler.hpp"

struct Object
{
    DataType type;
    void* value;

    Object()
    {
        this->type = DataType::_NULL;
        this->value = nullptr;
    }

    template<typename T>
    Object(T* value)
    {
        if (typeid(T) == typeid(int))
        {
            this->type = DataType::INT;
        }
        else if (typeid(T) == typeid(float))
        {
            this->type = DataType::FLOAT;
        }
        else if (typeid(T) == typeid(bool))
        {
            this->type = DataType::BOOL;
        }
        else if (typeid(T) == typeid(string))
        {
            this->type = DataType::STRING;
        }
        else
        {
            this->type = DataType::_NULL;
        }

        this->value = value;
    }

    Object(Object&& other) = delete;
    Object& operator=(Object& other) = delete;

    ~Object()
    {
        if (this->value == nullptr)
            return;

        switch (this->type)
        {
            case DataType::INT: 
                delete (int*)value;
                break;
            case DataType::FLOAT: 
                delete (float*)value;
                break;
            case DataType::BOOL: 
                delete (bool*)value;
                break;
            case DataType::STRING: 
                delete (string*)value;
                break;
        }
    }

    string to_string()
    {
        switch (this->type)
        {
            case DataType::INT: return std::to_string(*(int*)value);
            case DataType::FLOAT: return std::to_string(*(float*)value);
            case DataType::BOOL: return std::to_string(*(bool*)value);
            case DataType::STRING: return *(string*)value;
            case DataType::_NULL: return "Null";
        }

        return "???Object";
    }

    #pragma region Operators

    /// @brief Takes two objects off the stack and adds them
    static void add(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    stack.push(std::make_shared<Object>(new int(*(int*)left->value + *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    stack.push(std::make_shared<Object>(new float(*(float*)left->value + *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    stack.push(std::make_shared<Object>(new bool(*(bool*)left->value + *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    stack.push(std::make_shared<Object>(new string(*(string*)left->value + *(string*)right->value)));
                    break;
            }
        }
        else
        {
            LOG_ERROR("Cannot add objects of different types.");
        }
    }

    static void sub(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    stack.push(std::make_shared<Object>(new int(*(int*)left->value - *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    stack.push(std::make_shared<Object>(new float(*(float*)left->value - *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    stack.push(std::make_shared<Object>(new bool(*(bool*)left->value - *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    LOG_ERROR("Cannot subtract strings frome each other.");
                    break;
            }
        }
        else
        {
            LOG_ERROR("Cannot add objects of different types.");
        }
    }

    static void mul(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    stack.push(std::make_shared<Object>(new int(*(int*)left->value * *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    stack.push(std::make_shared<Object>(new float(*(float*)left->value * *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    stack.push(std::make_shared<Object>(new bool(*(bool*)left->value * *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    LOG_ERROR("Cannot multiply strings to each other.");
                    break;
            }
        }
        else
        {
            LOG_ERROR("Cannot add objects of different types.");
        }
    }

    static void div(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    stack.push(std::make_shared<Object>(new int(*(int*)left->value / *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    stack.push(std::make_shared<Object>(new float(*(float*)left->value / *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    stack.push(std::make_shared<Object>(new bool(*(bool*)left->value / *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    LOG_ERROR("Cannot divide strings from each other.");
                    break;
            }
        }
        else
        {
            LOG_ERROR("Cannot add objects of different types.");
        }
    }

    static void mod(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    stack.push(std::make_shared<Object>(new int(*(int*)left->value % *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    LOG_ERROR("Cannot modulus floating point numbers.");
                    break;
                case DataType::BOOL:
                    stack.push(std::make_shared<Object>(new bool(*(bool*)left->value % *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    LOG_ERROR("Cannot modulus strings with each other.");
                    break;
            }
        }
        else
        {
            LOG_ERROR("Cannot add objects of different types.");
        }
    }

    static void neg(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto value = stack.top();
        stack.pop();

        switch (value->type)
        {
            case DataType::INT:
                stack.push(std::make_shared<Object>(new int(-*(int*)value->value)));
                break;
            case DataType::FLOAT:
                stack.push(std::make_shared<Object>(new float(-*(float*)value->value)));
                break;
            case DataType::BOOL:
                LOG_ERROR("Cannot negate a boolean value.");
                break;
            case DataType::STRING:
                LOG_ERROR("Cannot negate a string");
                break;
        }
    }

    static void eql(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    stack.push(std::make_shared<Object>(new bool(*(int*)left->value == *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    stack.push(std::make_shared<Object>(new bool(*(float*)left->value == *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    stack.push(std::make_shared<Object>(new bool(*(bool*)left->value == *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    stack.push(std::make_shared<Object>(new bool(*(string*)left->value == *(string*)right->value)));
                    break;
            }
        }
        else
        {
            LOG_ERROR("Cannot add objects of different types.");
        }
    }

    static void neq(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    stack.push(std::make_shared<Object>(new bool(*(int*)left->value != *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    stack.push(std::make_shared<Object>(new bool(*(float*)left->value != *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    stack.push(std::make_shared<Object>(new bool(*(bool*)left->value != *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    stack.push(std::make_shared<Object>(new bool(*(string*)left->value != *(string*)right->value)));
                    break;
            }
        }
        else
        {
            LOG_ERROR("Cannot add objects of different types.");
        }
    }

    static void gte(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    stack.push(std::make_shared<Object>(new bool(*(int*)left->value >= *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    stack.push(std::make_shared<Object>(new bool(*(float*)left->value >= *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    stack.push(std::make_shared<Object>(new bool(*(bool*)left->value >= *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    stack.push(std::make_shared<Object>(new bool(*(string*)left->value >= *(string*)right->value)));
                    break;
            }
        }
        else
        {
            LOG_ERROR("Cannot add objects of different types.");
        }
    }

    static void lte(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    stack.push(std::make_shared<Object>(new bool(*(int*)left->value <= *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    stack.push(std::make_shared<Object>(new bool(*(float*)left->value <= *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    stack.push(std::make_shared<Object>(new bool(*(bool*)left->value <= *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    stack.push(std::make_shared<Object>(new bool(*(string*)left->value <= *(string*)right->value)));
                    break;
            }
        }
        else
        {
            LOG_ERROR("Cannot add objects of different types.");
        }
    }

    static void gth(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    stack.push(std::make_shared<Object>(new bool(*(int*)left->value > *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    stack.push(std::make_shared<Object>(new bool(*(float*)left->value > *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    stack.push(std::make_shared<Object>(new bool(*(bool*)left->value > *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    stack.push(std::make_shared<Object>(new bool(*(string*)left->value > *(string*)right->value)));
                    break;
            }
        }
        else
        {
            LOG_ERROR("Cannot add objects of different types.");
        }
    }

    static void lth(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    stack.push(std::make_shared<Object>(new bool(*(int*)left->value < *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    stack.push(std::make_shared<Object>(new bool(*(float*)left->value < *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    stack.push(std::make_shared<Object>(new bool(*(bool*)left->value < *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    stack.push(std::make_shared<Object>(new bool(*(string*)left->value < *(string*)right->value)));
                    break;
            }
        }
        else
        {
            LOG_ERROR("Cannot add objects of different types.");
        }
    }

    static void _an(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    stack.push(std::make_shared<Object>(new bool(*(int*)left->value && *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    stack.push(std::make_shared<Object>(new bool(*(float*)left->value && *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    stack.push(std::make_shared<Object>(new bool(*(bool*)left->value && *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    LOG_ERROR("Cannot and strings.");
                    break;
            }
        }
        else
        {
            LOG_ERROR("Cannot add objects of different types.");
        }
    }

    static void _or(std::stack<std::shared_ptr<Object>>& stack)
    {
        auto right = stack.top();
        stack.pop();
        auto left = stack.top();
        stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    stack.push(std::make_shared<Object>(new int(*(int*)left->value || *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    stack.push(std::make_shared<Object>(new float(*(float*)left->value || *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    stack.push(std::make_shared<Object>(new bool(*(bool*)left->value || *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    LOG_ERROR("Cannot or strings.");
                    break;
            }
        }
        else
        {
            LOG_ERROR("Cannot add objects of different types.");
        }
    }

    #pragma endregion
};

#endif