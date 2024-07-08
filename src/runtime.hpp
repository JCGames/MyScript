#ifndef RUNTIME
#define RUNTIME

#include <memory>

#include "compiler.hpp"

struct Object;

vector<std::shared_ptr<Object>> heap;
stack<std::shared_ptr<Object>> _stack;

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
    bool isReturned = false;
    string functionName = EMPTY_STRING;

    Scope(Scope* parent, StatementBlock* block)
    {
        this->parent = parent;

        if (parent != nullptr)
            functionName = parent->functionName;

        for (auto& stmt : block->statements)
        {
            if (stmt->type == StatementType::FUNCTION)
            {
                auto func = CAST(stmt, StatementFunction);
                functions[func->name->symbol + std::to_string(func->params.size())] = func;
            }
        }
    }

    void do_return(string functionName)
    {
        if (this->functionName == functionName)
        {
            isReturned = true;

            if (parent != nullptr)
                parent->do_return(functionName);
        }
    }

    bool has_var_in_this_scope(const string& name)
    {
        return variables.find(name) != variables.end();
    }

    bool has_fun_in_this_scope(const string& name)
    {
        return functions.find(name) != functions.end();
    }

    size_t* get_var_pointer(const string& name)
    {
        if (has_var_in_this_scope(name))
        {
            return &variables[name];
        }
        else if (parent != nullptr)
        {
            return parent->get_var_pointer(name);
        }
        
        return nullptr;
    }

    StatementFunction* get_func(const string& name)
    {
        if (has_fun_in_this_scope(name))
        {
            return functions[name];
        }
        else if (parent != nullptr)
        {
            return parent->get_func(name);
        }

        return nullptr;
    }

    void set_var(const string& name, std::shared_ptr<Object> value)
    {
        if (auto pointer = get_var_pointer(name))
        {
            heap[*pointer] = value;
            return;
        }

        heap.push_back(value); // add the value to the heap
        variables[name] = heap.size() - 1; // map the address to the variable name
    }

    void print_variables()
    {
        if (parent != nullptr)
            parent->print_variables();

        for (auto& variable : variables)
            std::cout << variable.first << " " << heap[variable.second]->to_string() << std::endl;
    }

    void print_functions()
    {
        if (parent != nullptr)
            parent->print_functions();

        for (auto& function : functions)
            std::cout << function.first << std::endl;
    }

    void print_scopes()
    {
        std::cout << "=======================" << std::endl;
        std::cout << "Scope Parent: " << parent << std::endl;
        std::cout << "Function Name: " << functionName << std::endl;
        std::cout << "Is Returned: " << isReturned << std::endl;
        std::cout << "=======================" << std::endl;

        if (parent != nullptr)
            parent->print_scopes();
    }
};

void rt_run_statement(Statement* stmt, Scope& scope);
void rt_run_function(StatementFunctionCall* functionCall, Scope& scope);

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

                if (symbol->symbol == "__OUT__")
                    std::cout << value->to_string();

                scope.set_var(symbol->symbol, value);

                break;
            }

            rt_run_exp(binaryOperator->left, scope);
            rt_run_exp(binaryOperator->right, scope);

            switch (binaryOperator->operatorType)
            {
                case OperatorType::ADDITION:        Object::add(); break;
                case OperatorType::SUBTRACTION:     Object::sub(); break;
                case OperatorType::MULTIPLICATION:  Object::mul(); break;
                case OperatorType::DIVISION:        Object::div(); break;
                case OperatorType::MODULUS:         Object::mod(); break;
                case OperatorType::EQUALS:          Object::eql(); break;
                case OperatorType::NOT_EQUALS:      Object::neq(); break;
                case OperatorType::GREATER_THAN_E:  Object::gte(); break;
                case OperatorType::LESS_THAN_E:     Object::lte(); break;
                case OperatorType::GREATER_THAN:    Object::gth(); break;
                case OperatorType::LESS_THAN:       Object::lth(); break;
                case OperatorType::AND:             Object::_an(); break;
                case OperatorType::OR:              Object::_or(); break;
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

            if (auto pointer = scope.get_var_pointer(symbol->symbol))
            {
                _stack.push(heap[*pointer]);
            }
            else
            {
                ERROR("Variable " + symbol->symbol + " has not been declared.");
            }
        }
            break;
        case StatementType::FUNCTION_CALL:
        {
            auto funcCall = CAST(stmt, StatementFunctionCall);

            if (funcCall->name->symbol == "__IN__")
            {
                string line;
                std::getline(std::cin, line);
                _stack.push(std::make_shared<Object>(new string(line)));
                scope.do_return(scope.functionName);
                break;
            }

            rt_run_function(funcCall, scope);
        }
            break;
    }
}

void rt_run_function(StatementFunctionCall* functionCall, Scope& scope)
{
    if (auto func = scope.get_func(functionCall->name->symbol + std::to_string(functionCall->argExpressions.size())))
    {
        Scope funcScope(&scope, func->body);

        std::ostringstream oss;
        oss << &funcScope;
        string address = oss.str();

        funcScope.functionName = func->name->symbol + std::to_string(functionCall->argExpressions.size()) + ":" + address;

        for (size_t i = 0; i < func->params.size(); ++i) 
        {
            rt_run_exp(functionCall->argExpressions[i], scope);

            auto result = _stack.top();
            _stack.pop();

            funcScope.set_var(func->params[i]->symbol, result);
        }

        rt_run_statement(func->body, funcScope);
    }
    else
    {
        ERROR("Wrong number of arguemnts for function " + functionCall->name->symbol + std::to_string(functionCall->argExpressions.size()));
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
            {
                rt_run_statement(s, blockScope);

                if (blockScope.isReturned == true)
                    break;
            }
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
        {
            auto funcCall = CAST(stmt, StatementFunctionCall);

            if (funcCall->name->symbol == "__VARS__")
            {
                scope.print_variables();
                break;
            }
            else if (funcCall->name->symbol == "__FUNCS__")
            {
                scope.print_functions();
                break;
            }
            else if (funcCall->name->symbol == "__SCOPES__")
            {
                scope.print_scopes();
                break;
            }

            rt_run_function(funcCall, scope);
        }
            break;
        case StatementType::IF:
        {
            auto _if = CAST(stmt, StatementIf);
            
            Scope ifScope(&scope, _if->block);

            rt_run_exp(_if->condition, scope);

            auto result = _stack.top();
            _stack.pop();

            if ((result->type == DataType::BOOL && *(bool*)result->value == true) ||
                (result->type == DataType::INT && *(int*)result->value > 0))
            {
                rt_run_statement(_if->block, ifScope);
            }
            else if (_if->_elseIf != nullptr)
            {
                rt_run_statement(_if->_elseIf, scope);
            }
            else if (_if->_else != nullptr)
            {
                rt_run_statement(_if->_else, scope);
            }
        }
            break;
        case StatementType::ELSE:
        {
            auto _else = CAST(stmt, StatementElse);

            Scope elseScope(&scope, _else->block);

            rt_run_statement(_else->block, elseScope);
        }
            break;
        case StatementType::RETURN:
        {
            auto _return = CAST(stmt, StatementReturn);

            rt_run_exp(_return->expression, scope);
            
            scope.do_return(scope.functionName);
        }
            break;
    }
}

void rt_run(StatementBlock* ast = parserAstRoot)
{
    Scope scope(nullptr, ast);

    scope.set_var("__OUT__", std::make_shared<Object>());

    rt_run_statement(ast, scope);
}

#endif // RUNTIME