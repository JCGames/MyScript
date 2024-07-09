#ifndef LEXER
#define LEXER

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "enums/token-type.hpp"
#include "helpers/logging.hpp"
#include "helpers/utils.hpp"

#define EMPTY_STRING ""

/// @brief Holds the names of all the files that have been lexified and parsed.
std::vector<std::string> lexerFiles;

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

void lexer_delete_tokens(std::vector<Token*>& tokens)
{
    for (auto token : tokens)
    {
        if (token != nullptr)
            delete token;
    }
}

void lexer_print_tokens(std::vector<Token*>& tokens)
{
    for (const auto token : tokens)
        std::cout << "Type: " << token_type_name(token->type) << " Value: |" << token->value << "| Line: " << token->line + 1 << std::endl;
}

std::vector<Token*> lexer_lexify(std::string fileName)
{
    std::ifstream ifs(fileName);

    if (!ifs.is_open())
        LOG_ERROR("Was unable to open file with name " + fileName + ".");

    lexerFiles.push_back(fileName);
    lexerFIndex = lexerFiles.size() - 1;

    std::vector<Token*> tokens;

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
            tokens.push_back(token);
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

            /**
             * Keywords
             */
            if (symbol == "fn")
            {
                tokens.push_back(new Token(_TokenType::FUNCITON, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "return")
            {
                tokens.push_back(new Token(_TokenType::RETURN, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "struct")
            {
                tokens.push_back(new Token(_TokenType::STRUCT, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "if")
            {
                tokens.push_back(new Token(_TokenType::IF, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "else")
            {
                tokens.push_back(new Token(_TokenType::ELSE, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "and")
            {
                tokens.push_back(new Token(_TokenType::AND, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "or")
            {
                tokens.push_back(new Token(_TokenType::OR, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "null")
            {
                tokens.push_back(new Token(_TokenType::_NULL, symbol, lexerFIndex, line));
                continue;
            }
            else if (symbol == "namespace")
            {
                tokens.push_back(new Token(_TokenType::NAMESPACE, symbol, lexerFIndex, line));
                continue;
            }

            tokens.push_back(new Token(_TokenType::SYMBOL, symbol, lexerFIndex, line));
            continue;
        }
        // STRING
        else if (c == '"')
        {
            std::string str = EMPTY_STRING;
            unsigned int beginLine = line;

            while (ifs.peek() != '"' && !ifs.eof())
                str += ifs.get();

            c = ifs.get();

            if (c != '"')
                CODE_ERROR("Missing a closing quotes.", lexerFiles[lexerFIndex], beginLine + 1);

            /**
             * Escaping characters in strings.
             */
            string_replace(str, "\\n", "\n");
            string_replace(str, "\\'", "\'");
            string_replace(str, "\\\"", "\"");
            string_replace(str, "\\?", "\?");
            string_replace(str, "\\\\", "\\");
            string_replace(str, "\\a", "\a");
            string_replace(str, "\\b", "\b");
            string_replace(str, "\\f", "\f");
            string_replace(str, "\\r", "\r");
            string_replace(str, "\\t", "\t");
            string_replace(str, "\\v", "\v");

            tokens.push_back(new Token(_TokenType::STRING, str, lexerFIndex, line));
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
                    CODE_ERROR("Number has too many decimal points.", lexerFiles[lexerFIndex], line + 1);
                }

                number += c;
            }

            tokens.push_back(new Token(_TokenType::NUMBER, number, lexerFIndex, line));
            continue;
        }
        // DOUBLE OPERATORS
        else if (c == '-' && ifs.peek() == '>')
        {
            char next = ifs.get();
            tokens.push_back(new Token(_TokenType::ASSIGN, std::string(1, c) + std::string(1, next), lexerFIndex, line));
        }
        else if (c == '!' && ifs.peek() == '=')
        {
            char next = ifs.get();
            tokens.push_back(new Token(_TokenType::NOT_EQUALS, std::string(1, c) + std::string(1, next), lexerFIndex, line));
        }
        else if (c == '>' && ifs.peek() == '=')
        {
            char next = ifs.get();
            tokens.push_back(new Token(_TokenType::GREATER_THAN_E, std::string(1, c) + std::string(1, next), lexerFIndex, line));
        }
        else if (c == '<' && ifs.peek() == '=')
        {
            char next = ifs.get();
            tokens.push_back(new Token(_TokenType::LESS_THAN_E, std::string(1, c) + std::string(1, next), lexerFIndex, line));
        }
        // SINGLE CHARACTER OPERATORS
        else if (c == '=')
        {
            tokens.push_back(new Token(_TokenType::EQUALS, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '>')
        {
            tokens.push_back(new Token(_TokenType::GREATER_THAN, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '<')
        {
            tokens.push_back(new Token(_TokenType::LESS_THAN, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '+')
        {
            tokens.push_back(new Token(_TokenType::PLUS, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '-')
        {
            tokens.push_back(new Token(_TokenType::MINUS, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '*')
        {
            tokens.push_back(new Token(_TokenType::MULTIPLY, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '/')
        {
            tokens.push_back(new Token(_TokenType::DIVIDE, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '%')
        {
            tokens.push_back(new Token(_TokenType::MODULUS, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '(')
        {
            tokens.push_back(new Token(_TokenType::OPEN_PARAN, std::string(1, c), lexerFIndex, line));
        }
        else if (c == ')')
        {
            tokens.push_back(new Token(_TokenType::CLOSE_PARAN, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '{')
        {
            tokens.push_back(new Token(_TokenType::OPEN_BRACKET, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '}')
        {
            tokens.push_back(new Token(_TokenType::CLOSE_BRACKET, std::string(1, c), lexerFIndex, line));
        }
        else if (c == ',')
        {
            tokens.push_back(new Token(_TokenType::COMMA, std::string(1, c), lexerFIndex, line));
        }
        else if (c == '.')
        {
            tokens.push_back(new Token(_TokenType::DOT, std::string(1, c), lexerFIndex, line));
        }
    }

    tokens.push_back(new Token(_TokenType::END_OF_FILE, EMPTY_STRING, lexerFIndex, line));

    ifs.close();

    return tokens;
}

#endif