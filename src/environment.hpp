#ifndef ENVIRONMENT
#define ENVIRONMENT

#include <map>
#include <string>

#include "object.hpp"

class Environment
{
    Environment* parent;
    std::map<std::string, Object*> variables;
    std::map<std::string, StatementFunction*> functions;
    std::map<std::string, StatementStruct*> structures;

public:
    Environment()
    {
        this->parent = nullptr;
    }

    Environment(Environment* parent, StatementBlock* block)
    {
        this->parent = parent;
        map_block(block->statements);
    }

    Environment(Environment* parent, StatementBlockNamespace* _namespace)
    {
        this->parent = parent;

        for (auto& definedSymbol : _namespace->definedSymbols)
        {
            if (definedSymbol.stmt->type == StatementType::FUNCTION)
            {
                set_function(definedSymbol.symbol, (StatementFunction*)definedSymbol.stmt);
            }
            else if (definedSymbol.stmt->type == StatementType::STRUCT)
            {
                set_structure(definedSymbol.symbol, (StatementStruct*)definedSymbol.stmt);
            }
        }
    }

    ~Environment()
    {
        for (auto& v : variables)
        {
            if (v.second != nullptr)
                delete v.second;
        }
    }

    bool has_variable(std::string name) { return variables.find(name) != variables.end(); }
    bool has_structure(std::string name) { return structures.find(name) != structures.end(); }
    bool has_function(std::string name) { return functions.find(name) != functions.end(); }

    void map_block(const std::vector<Statement*>& statements)
    {
        for (auto& stmt : statements)
        {
            if (stmt->type == StatementType::FUNCTION)
            {
                auto func = (StatementFunction*)stmt;
                set_function(func->name, func);
            }
            else if (stmt->type == StatementType::STRUCT)
            {
                auto _struct = (StatementStruct*)stmt;
                set_structure(_struct->name, _struct);
            }
        }
    }

    Object* get_variable(std::string name)
    {
        if (has_variable(name))
        {
            variables[name];
        }
        else if (parent != nullptr)
        {
            return parent->get_variable(name);
        }

        return nullptr;
    }

    void set_variable(std::string name, Object* value)
    {
        if (auto v = get_variable(name))
        {
            delete v;
            v = value;
        }

        variables[name] = value;
    }

    Object* get_function(std::string name)
    {
        if (has_variable(name))
        {
            functions[name];
        }
        else if (parent != nullptr)
        {
            return parent->get_function(name);
        }

        return nullptr;
    }

    void set_function(std::string name, StatementFunction* function)
    {
        functions[name] = function;
    }

    Object* get_structure(std::string name)
    {
        if (has_structure(name))
        {
            functions[name];
        }
        else if (parent != nullptr)
        {
            return parent->get_function(name);
        }

        return nullptr;
    }

    void set_structure(std::string name, StatementStruct* structure)
    {
        structures[name] = structure;
    }
};

#endif