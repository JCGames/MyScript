#ifndef LEXER
#define LEXER

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "enums/token-type.hpp"

#define LOG_ERROR(msg) logger_error(msg, __FILE__, __LINE__)
#define CODE_ERROR(msg, file, line) logger_code_error(msg, file, line, __FILE__, __LINE__)

void logger_error(std::string msg, const char* file, int line)
{
    std::cout << msg << " <<COMPILER:[" << file << ":" << line << "]>>" << std::endl;
    exit(1);
}

void logger_code_error(std::string msg, std::string codeFile, unsigned int codeLine, const char* file, int line)
{
    std::cout << msg << " [" << codeFile << ":" << codeLine << "] <<COMPILER:[" << file << ":" << line << "]>>" << std::endl;
    exit(1);
}

bool string_replace(std::string& str, const std::string& from, const std::string& to) 
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

const std::string EMPTY_STRING = "";

/// @brief Holds the names of all the files that have been lexified and parsed.
std::vector<std::string> lexeFiles;

/// @brief Gets the current file that is being lexified
unsigned int lexerFIndex;

struct Token
{
    _TokenType type;
    std::string value;
    unsigned int fIndex;
    unsigned int line;

    Token(_TokenType type, std::string value, unsigned int fIndex, unsigned int line)
    {
        this->type = type;
        this->value = value;
        this->fIndex = fIndex;
        this->line = line;
    }
};

std::vector<Token*> lexerTokens;

void lexer_delete_tokens()
{
    for (auto token : lexerTokens)
    {
        if (token != nullptr)
            delete token;
    }
}

void lexer_print_tokens()
{
    for (const auto token : lexerTokens)
        std::cout << "Type: " << token_type_name(token->type) << " Value: |" << token->value << "| Line: " << token->line + 1 << std::endl;
}

void lexer_lexify(std::string fileName)
{
    std::ifstream ifs(fileName);

    if (!ifs.is_open())
        LOG_ERROR("Was unable to open file with name " + fileName + ".");

    lexeFiles.push_back(fileName);
    lexerFIndex = lexeFiles.size() - 1;

    // reset the tokens list
    if (lexerTokens.size() > 0)
        lexer_delete_tokens();
    lexerTokens.clear();

    unsigned int line = 0;
    char c;

    // tokenize the entire file
    while (!ifs.eof())
    {
        c = ifs.get();

        // COMMENT
        if (c == '/' && ifs.peek() == '/')
        {
            while (!ifs.eof() && ifs.peek() != '\n')
                ifs.get();

            continue;
        }

        // END_OF_LINE
        if (c == '\n')
        {
            auto token = new Token(_TokenType::END_OF_LINE, EMPTY_STRING, lexerFIndex, line);
            ++line;
            lexerTokens.push_back(token);
            continue;
        }
        // SYMBOL
        else if (isalpha(c) || c == '_')
        {
            std::string symbol = std::string(1, c);
            char peek;

            while (true)
            {
                // only want to have to call ifs.peek once rather than twice
                peek = ifs.peek(); 

                if (!isalnum(peek) && peek != '_')
                    break;

                symbol += ifs.get();
            }

            if (symbol == "fn")
            {
                lexerTokens.push_back(new Token(_TokenType::FUNCITON, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "return")
            {
                lexerTokens.push_back(new Token(_TokenType::RETURN, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "struct")
            {
                lexerTokens.push_back(new Token(_TokenType::STRUCT, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "if")
            {
                lexerTokens.push_back(new Token(_TokenType::IF, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "else")
            {
                lexerTokens.push_back(new Token(_TokenType::ELSE, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "and")
            {
                lexerTokens.push_back(new Token(_TokenType::AND, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "or")
            {
                lexerTokens.push_back(new Token(_TokenType::OR, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "null")
            {
                lexerTokens.push_back(new Token(_TokenType::_NULL, symbol, lexerFIndex, line));
                continue;
            }

            lexerTokens.push_back(new Token(_TokenType::SYMBOL, symbol, lexerFIndex, line));
            continue;
        }
        // std::string
        else if (c == '"')
        {
            std::string str = EMPTY_STRING;
            unsigned int beginLine = line;

            while (ifs.peek() != '"' && !ifs.eof())
                str += ifs.get();

            c = ifs.get();

            if (c != '"')
                CODE_ERROR("Missing a closing quotes.", lexeFiles[lexerFIndex], beginLine + 1);

            string_replace(str, "\\n", "\n");

            lexerTokens.push_back(new Token(_TokenType::STRING, str, lexerFIndex, line));
            continue;
        }
        // NUMBER
        else if (isdigit(c) || (c == '.' && isdigit(ifs.peek())))
        {
            std::string number = std::string(1, c);
            char peek;
            bool hasDecimalPoint = false;

            while (true)
            {
                peek = ifs.peek();

                if (!isdigit(peek) && peek != '.')
                    break;

                c = ifs.get(); 

                if (c == '.' && !hasDecimalPoint)
                {
                    hasDecimalPoint = true;
                } 
                else if (c == '.' && hasDecimalPoint)
                {
                    CODE_ERROR("Number has too many decimal points.", lexeFiles[lexerFIndex], line + 1);
                }

                number += c;
            }

            lexerTokens.push_back(new Token(_TokenType::NUMBER, number, lexerFIndex, line));
            continue;
        }
        // DOUBLE OPERATORS
        else if (c == '-' && ifs.peek() == '>')
        {
            char next = ifs.get();
            lexerTokens.push_back(new Token(_TokenType::ASSIGN, std::string(1, c) + std::string(1, next), lexerFIndex, line));
        }
        else if (c == '!' && ifs.peek() == '=')
        {
            char next = ifs.get();
            lexerTokens.push_back(new Token(_TokenType::NOT_EQUALS, std::string(1, c) + std::string(1, next), lexerFIndex, line));
        }
        else if (c == '>' && ifs.peek() == '=')
        {
            char next = ifs.get();
            lexerTokens.push_back(new Token(_TokenType::GREATER_THAN_E, std::string(1, c) + std::string(1, next), lexerFIndex, line));
        }
        else if (c == '<' && ifs.peek() == '=')
        {
            char next = ifs.get();
            lexerTokens.push_back(new Token(_TokenType::LESS_THAN_E, std::string(1, c) + std::string(1, next), lexerFIndex, line));
        }
        // SINGLE CHARACTER OPERATORS
        else if (c == '=')
        {
            lexerTokens.push_back(new Token(_TokenType::EQUALS, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '>')
        {
            lexerTokens.push_back(new Token(_TokenType::GREATER_THAN, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '<')
        {
            lexerTokens.push_back(new Token(_TokenType::LESS_THAN, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '+')
        {
            lexerTokens.push_back(new Token(_TokenType::PLUS, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '-')
        {
            lexerTokens.push_back(new Token(_TokenType::MINUS, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '*')
        {
            lexerTokens.push_back(new Token(_TokenType::MULTIPLY, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '/')
        {
            lexerTokens.push_back(new Token(_TokenType::DIVIDE, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '%')
        {
            lexerTokens.push_back(new Token(_TokenType::MODULUS, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '(')
        {
            lexerTokens.push_back(new Token(_TokenType::OPEN_PARAN, std::string(1, c), lexerFIndex, line));
        }
        else if (c == ')')
        {
            lexerTokens.push_back(new Token(_TokenType::CLOSE_PARAN, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '{')
        {
            lexerTokens.push_back(new Token(_TokenType::OPEN_BRACKET, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '}')
        {
            lexerTokens.push_back(new Token(_TokenType::CLOSE_BRACKET, std::string(1, c), lexerFIndex, line));
        }
        else if (c == ',')
        {
            lexerTokens.push_back(new Token(_TokenType::COMMA, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '.')
        {
            lexerTokens.push_back(new Token(_TokenType::DOT, std::string(1, c), lexerFIndex, line));
        }
    }

    lexerTokens.push_back(new Token(_TokenType::END_OF_FILE, EMPTY_STRING, lexerFIndex, line));

    ifs.close();
}

#endif