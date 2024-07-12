#ifndef OBJECT
#define OBJECT

#include <map>
#include <typeinfo>

#include "statements.hpp"
#include "enums/data-type.hpp"

enum class ObjectType
{
    _NULL,
    INT,
    FLOAT,
    BOOL,
    STRING,
    STRUCT
};

class Object
{
    ObjectType type;
    std::string nameOfType;

    std::map<std::string, Object*> variables;
    std::map<std::string, StatementFunction*> functions;
    
    void* primitiveValue;

public:
    Object()
    {
        this->type = ObjectType::_NULL;
        this->primitiveValue = nullptr;
    }

    template<typename T>
    Object(T* primitiveValue)
    {
        if (typeid(T) == typeid(int))
        {
            this->type = ObjectType::INT;
        }
        else if (typeid(T) == typeid(float))
        {
            this->type = ObjectType::FLOAT;
        }
        else if (typeid(T) == typeid(bool))
        {
            this->type = ObjectType::BOOL;
        }
        else if (typeid(T) == typeid(std::string))
        {
            this->type = ObjectType::STRING;
        }
        else
        {
            // error occured
        }
        
        this->primitiveValue = primitiveValue;
    }

    ~Object()
    {
        for (auto& v : variables)
        {
            if (v.second != nullptr)
                delete v.second;
        }

        if (primitiveValue == nullptr) return;

        if (this->type == ObjectType::INT)
        {
            delete (int*)primitiveValue;
        }
        else if (this->type == ObjectType::FLOAT)
        {
            delete (float*)primitiveValue;
        }
        else if (this->type == ObjectType::BOOL)
        {
            delete (bool*)primitiveValue;
        }
        else if (this->type == ObjectType::STRING)
        {
            delete (std::string*)primitiveValue;
        }
    }

    std::string to_string()
    {
        switch (this->type)
        {
            case ObjectType::INT: return std::to_string(*(int*)primitiveValue);
            case ObjectType::FLOAT: return std::to_string(*(float*)primitiveValue);
            case ObjectType::BOOL: return *(bool*)primitiveValue ? "True" : "False";
            case ObjectType::STRING: return *(std::string*)primitiveValue;
        }

        return nameOfType;
    }

    bool has_variable(std::string name) const { return variables.find(name) != variables.end(); }
    bool has_function(std::string name) const { return functions.find(name) != functions.end(); }

    void* get_value() { return primitiveValue; }
    ObjectType get_type() { return type; }
    std::string get_type_name() { return nameOfType; }

    void set_type(ObjectType type) { this->type = type; }
    void set_type_name(std::string typeName) { this->nameOfType = typeName; }
    
    Object* get_variable(std::string name)
    {
        if (has_variable(name))
            return variables[name];

        return nullptr;
    }

    StatementFunction* get_function(std::string name)
    {
        if (has_function(name))
            return functions[name];

        return nullptr;
    } 
};

#endif