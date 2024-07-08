#ifndef RUNTIME
#define RUNTIME

#include <memory>

#include "compiler.hpp"
#include "object.hpp"

vector<std::shared_ptr<Object>> rtHeap;
stack<std::shared_ptr<Object>> rtStack;

/// @brief A structure for contextual data.
class Context
{
    std::string blockName;
    map<string, StatementFunction*> functions;
    map<string, size_t> variables;

public:
    bool has_var(const string& name) { return variables.find(name) != variables.end(); }
    bool has_fun(const string& name) { return functions.find(name) != functions.end(); }

    void set_var(const string& name, const size_t& pointer) { variables[name] = pointer; }
    void set_func(const string& name, StatementFunction* stmtFunc) { functions[name] = stmtFunc; }

    size_t* get_var(const string& name) { if (has_var(name)) return &variables[name]; return nullptr; }
    StatementFunction* get_func(const string& name) { if (has_fun(name)) return functions[name]; return nullptr; }

    void print_variables()
    {
        for (auto& variable : variables)
            std::cout << variable.first << " " << rtHeap[variable.second]->to_string() << std::endl;
    }

    void print_functions()
    {
        for (auto& function : functions)
            std::cout << function.first << std::endl;
    }
};

/// @brief Used for all types of scoping problems.
class Scope
{
    Scope* parent;
    Context ctx;

    /// @brief has this scope been returned from.
    bool isReturned = false;

    /// @brief Example: A function can create a scope. Hince the need for tracking what scope we are in.
    string id = EMPTY_STRING;

    void internal_propagate_return_upwards(std::string id)
    {
        if (this->id == id)
        {
            isReturned = true;

            if (parent != nullptr)
                parent->internal_propagate_return_upwards(id);
        }
    }

public:
    Scope(Scope* parent, StatementBlock* block)
    {
        this->parent = parent;

        if (parent != nullptr)
            id = parent->id;

        for (auto& stmt : block->statements)
        {
            if (stmt->type == StatementType::FUNCTION)
            {
                auto func = CAST(stmt, StatementFunction);
                ctx.set_func(func->name->symbol, func);
            }
        }
    }

    Scope(Scope* parent, StatementBlock* block, std::string id)
    {
        std::ostringstream oss;
        oss << parent;
        auto callersAddress = oss.str();

        this->id = id + ":" + callersAddress;

        for (auto& stmt : block->statements)
        {
            if (stmt->type == StatementType::FUNCTION)
            {
                auto func = CAST(stmt, StatementFunction);
                ctx.set_func(func->name->symbol, func);
            }
        }
    }

    const std::string& get_id() { return this->id; }
    const bool& get_is_returned() { return this->isReturned; }
    /// @brief Will propagate upwards until the id of the upwards scope changes.
    void propagate_return_upwards() { internal_propagate_return_upwards(this->id); }

    size_t* get_var_pointer(const string& name)
    {
        if (ctx.has_var(name))
            return ctx.get_var(name);
        else if (parent != nullptr)
            return parent->get_var_pointer(name);
        
        return nullptr;
    }

    StatementFunction* get_func(const string& name)
    {
        if (ctx.has_fun(name))
            return ctx.get_func(name);
        else if (parent != nullptr)
            return parent->get_func(name);

        return nullptr;
    }

    void set_var(const string& name, std::shared_ptr<Object> value)
    {
        if (auto pointer = get_var_pointer(name))
        {
            rtHeap[*pointer] = value;
            return;
        }

        rtHeap.push_back(value); // add the value to the rtHeap
        ctx.set_var(name, rtHeap.size() - 1); // map the address to the variable name
    }

    void print_variables()
    {
        if (parent != nullptr)
            parent->print_variables();

        ctx.print_variables();
    }

    void print_functions()
    {
        if (parent != nullptr)
            parent->print_functions();

        ctx.print_functions();
    }

    void print_scopes()
    {
        std::cout << "=======================" << std::endl;
        std::cout << "Scope Parent: " << parent << std::endl;
        std::cout << "Scope Id: " << id << std::endl;
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
                    LOG_ERROR("Not a valid assignment.");

                auto symbol = CAST(binaryOperator->left, StatementSymbol);

                rt_run_exp(binaryOperator->right, scope);

                auto value = rtStack.top();

                if (symbol->symbol == "__OUT__")
                    std::cout << value->to_string();

                scope.set_var(symbol->symbol, value);

                break;
            }

            rt_run_exp(binaryOperator->left, scope);
            rt_run_exp(binaryOperator->right, scope);

            switch (binaryOperator->operatorType)
            {
                case OperatorType::ADDITION:        Object::add(rtStack); break;
                case OperatorType::SUBTRACTION:     Object::sub(rtStack); break;
                case OperatorType::MULTIPLICATION:  Object::mul(rtStack); break;
                case OperatorType::DIVISION:        Object::div(rtStack); break;
                case OperatorType::MODULUS:         Object::mod(rtStack); break;
                case OperatorType::EQUALS:          Object::eql(rtStack); break;
                case OperatorType::NOT_EQUALS:      Object::neq(rtStack); break;
                case OperatorType::GREATER_THAN_E:  Object::gte(rtStack); break;
                case OperatorType::LESS_THAN_E:     Object::lte(rtStack); break;
                case OperatorType::GREATER_THAN:    Object::gth(rtStack); break;
                case OperatorType::LESS_THAN:       Object::lth(rtStack); break;
                case OperatorType::AND:             Object::_an(rtStack); break;
                case OperatorType::OR:              Object::_or(rtStack); break;
            }
        }
            break;
        case StatementType::UNARY_OPERATOR:
        {
            auto unaryOperator = CAST(stmt, StatementUnaryOperator);

            rt_run_exp(unaryOperator->root, scope);

            switch (unaryOperator->operatorType)
            {
                case OperatorType::NEGATE: Object::neg(rtStack); break;
            }
        }
            break;
        case StatementType::VALUE:
        {
            auto value = CAST(stmt, StatementValue);

            switch (value->dataType)
            {
                case DataType::INT:
                    rtStack.push(std::make_shared<Object>(new int(std::stoi(value->value))));
                    break;
                case DataType::FLOAT:
                    rtStack.push(std::make_shared<Object>(new float(std::stof(value->value))));
                    break;
                case DataType::BOOL:
                    rtStack.push(std::make_shared<Object>(new bool(std::stoi(value->value))));
                    break;
                case DataType::STRING:
                    rtStack.push(std::make_shared<Object>(new string(value->value)));
                    break;
                case DataType::_NULL:
                    rtStack.push(std::make_shared<Object>());
                    break;
            }
        }
            break;
        case StatementType::SYMBOL:
        {
            auto symbol = CAST(stmt, StatementSymbol);

            if (auto pointer = scope.get_var_pointer(symbol->symbol))
            {
                rtStack.push(rtHeap[*pointer]);
            }
            else
            {
                LOG_ERROR("Variable " + symbol->symbol + " has not been declared.");
            }
        }
            break;
        case StatementType::FUNCTION_CALL:
        {
            auto funcCall = CAST(stmt, StatementFunctionCall);

            if (funcCall->name->symbol == "__IN__0")
            {
                string line;
                std::getline(std::cin, line);
                rtStack.push(std::make_shared<Object>(new string(line)));
                break;
            }

            rt_run_function(funcCall, scope);
        }
            break;
    }
}

void rt_run_function(StatementFunctionCall* functionCall, Scope& scope)
{
    if (auto func = scope.get_func(functionCall->name->symbol))
    {
        Scope funcScope(&scope, func->body, func->name->symbol);

        for (size_t i = 0; i < func->params.size(); ++i) 
        {
            rt_run_exp(functionCall->argExpressions[i], scope);

            auto result = rtStack.top();
            rtStack.pop();

            funcScope.set_var(func->params[i]->symbol, result);
        }

        rt_run_statement(func->body, funcScope);
    }
    else
    {
        LOG_ERROR("Wrong number of arguemnts for function " + functionCall->name->symbol);
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

                if (blockScope.get_is_returned())
                    break;
            }
        }
            break;
        case StatementType::EXPRESSION:
        {
            auto exp = CAST(stmt, StatementExpression);

            rt_run_exp(stmt, scope);

            auto result = rtStack.top();
            rtStack.pop();

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

            if (funcCall->name->symbol == "__VARS__0")
            {
                scope.print_variables();
                break;
            }
            else if (funcCall->name->symbol == "__FUNCS__0")
            {
                scope.print_functions();
                break;
            }
            else if (funcCall->name->symbol == "__SCOPES__0")
            {
                scope.print_scopes();
                break;
            }
            else if (funcCall->name->symbol == "__IN__0")
            {
                string line;
                std::getline(std::cin, line);
                rtStack.push(std::make_shared<Object>(new string(line)));
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

            auto result = rtStack.top();
            rtStack.pop();

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
            
            scope.propagate_return_upwards();
        }
            break;
    }
}

/**
 * Runs the given Abstract Syntax Tree.
 */
void rt_run(StatementBlock* ast)
{
    Scope scope(nullptr, ast);

    scope.set_var("__OUT__", std::make_shared<Object>());

    rt_run_statement(ast, scope);
}

#endif // RUNTIME