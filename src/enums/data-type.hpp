#ifndef DATA_TYPE
#define DATA_TYPE

#include <string>

enum class DataType
{
    STRING,
    FLOAT,
    INT,
    BOOL,
    _NULL
};

std::string data_type_name(DataType type)
{
    switch (type)
    {
        case DataType::STRING: return "STRING";
        case DataType::FLOAT: return "FLOAT";
        case DataType::INT: return "INT";
        case DataType::BOOL: return "BOOL";
        case DataType::_NULL: return "_NULL";
        default: return "That was not a token type";
    }
}

#endif