#ifndef RUNTIME
#define RUNTIME

#include <memory>

#include "compiler.hpp"

struct Object;

vector<std::shared_ptr<Object>> heap;
stack<std::shared_ptr<Object>> _stack;
bool isReturned = false;
bool isBroke = false;

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
        }

        return "Object?";
    }

    #pragma region Operators

    /// @brief Takes two objects off the stack and adds them
    static void add()
    {
        auto right = _stack.top();
        _stack.pop();
        auto left = _stack.top();
        _stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new int(*(int*)left->value + *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    _stack.push(std::make_shared<Object>(new float(*(float*)left->value + *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(*(bool*)left->value + *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    _stack.push(std::make_shared<Object>(new string(*(string*)left->value + *(string*)right->value)));
                    break;
            }
        }
        else
        {
            ERROR("Cannot add objects of different types.");
        }
    }

    static void sub()
    {
        auto right = _stack.top();
        _stack.pop();
        auto left = _stack.top();
        _stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new int(*(int*)left->value - *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    _stack.push(std::make_shared<Object>(new float(*(float*)left->value - *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(*(bool*)left->value - *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    ERROR("Cannot subtract strings frome each other.");
                    break;
            }
        }
        else
        {
            ERROR("Cannot add objects of different types.");
        }
    }

    static void mul()
    {
        auto right = _stack.top();
        _stack.pop();
        auto left = _stack.top();
        _stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new int(*(int*)left->value * *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    _stack.push(std::make_shared<Object>(new float(*(float*)left->value * *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(*(bool*)left->value * *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    ERROR("Cannot multiply strings to each other.");
                    break;
            }
        }
        else
        {
            ERROR("Cannot add objects of different types.");
        }
    }

    static void div()
    {
        auto right = _stack.top();
        _stack.pop();
        auto left = _stack.top();
        _stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new int(*(int*)left->value / *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    _stack.push(std::make_shared<Object>(new float(*(float*)left->value / *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(*(bool*)left->value / *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    ERROR("Cannot divide strings from each other.");
                    break;
            }
        }
        else
        {
            ERROR("Cannot add objects of different types.");
        }
    }

    static void mod()
    {
        auto right = _stack.top();
        _stack.pop();
        auto left = _stack.top();
        _stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new int(*(int*)left->value % *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    ERROR("Cannot modulus floating point numbers.");
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(*(bool*)left->value % *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    ERROR("Cannot modulus strings with each other.");
                    break;
            }
        }
        else
        {
            ERROR("Cannot add objects of different types.");
        }
    }

    static void neg()
    {
        auto value = _stack.top();
        _stack.pop();

        switch (value->type)
        {
            case DataType::INT:
                _stack.push(std::make_shared<Object>(new int(-*(int*)value->value)));
                break;
            case DataType::FLOAT:
                _stack.push(std::make_shared<Object>(new float(-*(float*)value->value)));
                break;
            case DataType::BOOL:
                ERROR("Cannot negate a boolean value.");
                break;
            case DataType::STRING:
                ERROR("Cannot negate a string");
                break;
        }
    }

    static void eql()
    {
        auto right = _stack.top();
        _stack.pop();
        auto left = _stack.top();
        _stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new bool(*(int*)left->value == *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    _stack.push(std::make_shared<Object>(new bool(*(float*)left->value == *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(*(bool*)left->value == *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    _stack.push(std::make_shared<Object>(new bool(*(string*)left->value == *(string*)right->value)));
                    break;
            }
        }
        else
        {
            ERROR("Cannot add objects of different types.");
        }
    }

    static void neq()
    {
        auto right = _stack.top();
        _stack.pop();
        auto left = _stack.top();
        _stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new bool(*(int*)left->value != *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    _stack.push(std::make_shared<Object>(new bool(*(float*)left->value != *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(*(bool*)left->value != *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    _stack.push(std::make_shared<Object>(new bool(*(string*)left->value != *(string*)right->value)));
                    break;
            }
        }
        else
        {
            ERROR("Cannot add objects of different types.");
        }
    }

    static void gte()
    {
        auto right = _stack.top();
        _stack.pop();
        auto left = _stack.top();
        _stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new bool(*(int*)left->value >= *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    _stack.push(std::make_shared<Object>(new bool(*(float*)left->value >= *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(*(bool*)left->value >= *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    _stack.push(std::make_shared<Object>(new bool(*(string*)left->value >= *(string*)right->value)));
                    break;
            }
        }
        else
        {
            ERROR("Cannot add objects of different types.");
        }
    }

    static void lte()
    {
        auto right = _stack.top();
        _stack.pop();
        auto left = _stack.top();
        _stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new bool(*(int*)left->value <= *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    _stack.push(std::make_shared<Object>(new bool(*(float*)left->value <= *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(*(bool*)left->value <= *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    _stack.push(std::make_shared<Object>(new bool(*(string*)left->value <= *(string*)right->value)));
                    break;
            }
        }
        else
        {
            ERROR("Cannot add objects of different types.");
        }
    }

    static void gth()
    {
        auto right = _stack.top();
        _stack.pop();
        auto left = _stack.top();
        _stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new bool(*(int*)left->value > *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    _stack.push(std::make_shared<Object>(new bool(*(float*)left->value > *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(*(bool*)left->value > *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    _stack.push(std::make_shared<Object>(new bool(*(string*)left->value > *(string*)right->value)));
                    break;
            }
        }
        else
        {
            ERROR("Cannot add objects of different types.");
        }
    }

    static void lth()
    {
        auto right = _stack.top();
        _stack.pop();
        auto left = _stack.top();
        _stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new bool(*(int*)left->value < *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    _stack.push(std::make_shared<Object>(new bool(*(float*)left->value < *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(*(bool*)left->value < *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    _stack.push(std::make_shared<Object>(new bool(*(string*)left->value < *(string*)right->value)));
                    break;
            }
        }
        else
        {
            ERROR("Cannot add objects of different types.");
        }
    }

    static void _an()
    {
        auto right = _stack.top();
        _stack.pop();
        auto left = _stack.top();
        _stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new bool(*(int*)left->value && *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    _stack.push(std::make_shared<Object>(new bool(*(float*)left->value && *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(*(bool*)left->value && *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    ERROR("Cannot and strings.");
                    break;
            }
        }
        else
        {
            ERROR("Cannot add objects of different types.");
        }
    }

    static void _or()
    {
        auto right = _stack.top();
        _stack.pop();
        auto left = _stack.top();
        _stack.pop();

        if (left->type == right->type)
        {
            switch (left->type)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new int(*(int*)left->value || *(int*)right->value)));
                    break;
                case DataType::FLOAT:
                    _stack.push(std::make_shared<Object>(new float(*(float*)left->value || *(float*)right->value)));
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(*(bool*)left->value || *(bool*)right->value)));
                    break;
                case DataType::STRING:
                    ERROR("Cannot or strings.");
                    break;
            }
        }
        else
        {
            ERROR("Cannot add objects of different types.");
        }
    }

    #pragma endregion
};

struct Scope
{
    Scope* parent;
    map<string, StatementFunction*> functions;
    map<string, size_t> variables;
    size_t baseAddressInHeap;

    Scope(Scope* parent, StatementBlock* block)
    {
        this->baseAddressInHeap = heap.size() - 1;
        this->parent = parent;

        for (auto& stmt : block->statements)
        {
            if (stmt->type == StatementType::FUNCTION)
            {
                auto func = CAST(stmt, StatementFunction);
                functions[func->name->symbol] = func;
            }
        }
    }

    ~Scope()
    {
        // clean up variables on heap
        // for (auto it = heap.end(); it != heap.begin() + baseAddressInHeap; --it)
        // {
        //     heap.erase(it);
        // }
    }

    bool has_var(const string& name)
    {
        return variables.find(name) != variables.end();
    }

    bool has_func(const string& name)
    {
        return functions.find(name) != functions.end();
    }

    size_t get_var_pointer(const string& name)
    {
        if (has_var(name))
            return variables[name];
        
        return -1;
    }

    void set_var(const string& name, std::shared_ptr<Object> value)
    {
        if (has_var(name))
        {
            heap[variables[name]] = value;
        }
        else
        {
            heap.push_back(value); // add the value to the heap
            variables[name] = heap.size() - 1; // map the address to the variable name
        }
    }

    StatementFunction* get_func(const string& name)
    {
        if (has_func(name))
            return functions[name];

        return nullptr;
    }
};

void rt_run_exp(Statement* stmt, Scope& scope)
{
    switch (stmt->type)
    {
        case StatementType::EXPRESSION:
            rt_run_exp(CAST(stmt, StatementExpression)->root, scope);
            break;
        case StatementType::BINARY_OPERATOR:
        {
            auto binaryOperator = CAST(stmt, StatementBinaryOperator);

            if (binaryOperator->operatorType == OperatorType::ASSIGNMENT)
            {
                if (binaryOperator->left->type != StatementType::SYMBOL)
                    ERROR("Not a valid assignment.");

                auto symbol = CAST(binaryOperator->left, StatementSymbol);

                rt_run_exp(binaryOperator->right, scope);

                auto value = _stack.top();

                scope.set_var(symbol->symbol, value);

                break;
            }

            rt_run_exp(binaryOperator->left, scope);
            rt_run_exp(binaryOperator->right, scope);

            switch (binaryOperator->operatorType)
            {
                case OperatorType::ADDITION: Object::add(); break;
                case OperatorType::SUBTRACTION: Object::sub(); break;
                case OperatorType::MULTIPLICATION: Object::mul(); break;
                case OperatorType::DIVISION: Object::div(); break;
                case OperatorType::MODULUS: Object::mod(); break;
                case OperatorType::EQUALS: Object::eql(); break;
                case OperatorType::NOT_EQUALS: Object::neq(); break;
                case OperatorType::GREATER_THAN_E: Object::gte(); break;
                case OperatorType::LESS_THAN_E: Object::lte(); break;
                case OperatorType::GREATER_THAN: Object::gth(); break;
                case OperatorType::LESS_THAN: Object::lth(); break;
                case OperatorType::AND: Object::_an(); break;
                case OperatorType::OR: Object::_or(); break;
            }
        }
            break;
        case StatementType::UNARY_OPERATOR:
        {
            auto unaryOperator = CAST(stmt, StatementUnaryOperator);

            rt_run_exp(unaryOperator->root, scope);

            switch (unaryOperator->operatorType)
            {
                case OperatorType::NEGATE: Object::neg(); break;
            }
        }
            break;
        case StatementType::VALUE:
        {
            auto value = CAST(stmt, StatementValue);

            switch (value->dataType)
            {
                case DataType::INT:
                    _stack.push(std::make_shared<Object>(new int(std::stoi(value->value))));
                    break;
                case DataType::FLOAT:
                    _stack.push(std::make_shared<Object>(new float(std::stof(value->value))));
                    break;
                case DataType::BOOL:
                    _stack.push(std::make_shared<Object>(new bool(std::stoi(value->value))));
                    break;
                case DataType::STRING:
                    _stack.push(std::make_shared<Object>(new string(value->value)));
                    break;
                case DataType::_NULL:
                    _stack.push(std::make_shared<Object>());
                    break;
            }
        }
            break;
        case StatementType::SYMBOL:
        {
            auto symbol = CAST(stmt, StatementSymbol);

            if (scope.has_var(symbol->symbol))
            {
                _stack.push(heap[scope.get_var_pointer(symbol->symbol)]);
            }
            else
            {
                ERROR("Variable " + symbol->symbol + " has not been declared.");
            }
        }
            break;
    }
}
 
void rt_run_statement(Statement* stmt, Scope& scope)
{
    switch (stmt->type)
    {
        case StatementType::BLOCK:
        {
            auto block = CAST(stmt, StatementBlock);
            
            Scope blockScope(&scope, block);
            
            for (auto& s : block->statements)
                rt_run_statement(s, blockScope);
        }
            break;
        case StatementType::EXPRESSION:
        {
            auto exp = CAST(stmt, StatementExpression);

            rt_run_exp(stmt, scope);

            auto result = _stack.top();
            _stack.pop();

            if (exp->root->type == StatementType::BINARY_OPERATOR)
            {
                auto binaryOperator = CAST(exp->root, StatementBinaryOperator);

                if (binaryOperator->operatorType == OperatorType::ASSIGNMENT)
                    break;
            }
            
            std::cout << result->to_string() << std::endl;
        }
            break;
        case StatementType::FUNCTION_CALL:
            break;
    }
}

void rt_run(StatementBlock* ast = parserAstRoot)
{
    Scope scope(nullptr, ast);
    rt_run_statement(ast, scope);
}

#endif // RUNTIME