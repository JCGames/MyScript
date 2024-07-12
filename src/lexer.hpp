#ifndef LEXER
#define LEXER

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "enums/token-type.hpp"
#include "helpers/logging.hpp"
#include "helpers/utils.hpp"

#include "location-metadata.hpp"

#define EMPTY_STRING ""

std::string lexerCurrentFileName;

struct Token
{
    _TokenType type;
    std::string value;
    LocationMetadata loc;

    Token(const _TokenType& type, const std::string& value, const unsigned int& line)
    {
        this->type = type;
        this->value = value;
        loc.fileName = lexerCurrentFileName;
        loc.line = line;
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
        std::cout << "Type: " << token_type_name(token->type) << " Value: |" << token->value << "| Line: " << token->loc.line + 1 << std::endl;
}

std::vector<Token*> lexer_get_tokens(const std::string& filePath, const std::string& fileName)
{
    std::ifstream ifs(filePath);

    if (!ifs.is_open())
        LOG_ERROR("Was unable to open file with name " + fileName + ".");

    lexerCurrentFileName = fileName;

    std::vector<Token*> tokens;

    unsigned int line = 0;
    char c;

    while (!ifs.eof())
    {
        c = ifs.get();

        // COMMENT - will be completely ignored
        if (c == '/' && ifs.peek() == '/')
        {
            while (!ifs.eof() && ifs.peek() != '\n')
                ifs.get();
            continue;
        }
        // END_OF_LINE
        else if (c == '\n')
        {
            auto token = new Token(_TokenType::END_OF_LINE, EMPTY_STRING, line);
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
                tokens.push_back(new Token(_TokenType::FUNCITON, symbol, line));
                continue;
            }
            else if (symbol == "return")
            {
                tokens.push_back(new Token(_TokenType::RETURN, symbol, line));
                continue;
            }
            else if (symbol == "struct")
            {
                tokens.push_back(new Token(_TokenType::STRUCT, symbol, line));
                continue;
            }
            else if (symbol == "if")
            {
                tokens.push_back(new Token(_TokenType::IF, symbol, line));
                continue;
            }
            else if (symbol == "else")
            {
                tokens.push_back(new Token(_TokenType::ELSE, symbol, line));
                continue;
            }
            else if (symbol == "and")
            {
                tokens.push_back(new Token(_TokenType::AND, symbol, line));
                continue;
            }
            else if (symbol == "or")
            {
                tokens.push_back(new Token(_TokenType::OR, symbol, line));
                continue;
            }
            else if (symbol == "null")
            {
                tokens.push_back(new Token(_TokenType::_NULL, symbol, line));
                continue;
            }
            else if (symbol == "namespace")
            {
                tokens.push_back(new Token(_TokenType::NAMESPACE, symbol, line));
                continue;
            }
            else if (symbol == "while")
            {
                tokens.push_back(new Token(_TokenType::WHILE, symbol, line));
                continue;
            }
            else if (symbol == "true")
            {
                tokens.push_back(new Token(_TokenType::_TRUE, symbol, line));
                continue;
            }
            else if (symbol == "false")
            {
                tokens.push_back(new Token(_TokenType::_FALSE, symbol, line));
                continue;
            }

            tokens.push_back(new Token(_TokenType::SYMBOL, symbol, line));
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
                CODE_ERROR("Missing a closing quotes.", lexerCurrentFileName, beginLine + 1);

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

            tokens.push_back(new Token(_TokenType::STRING, str, line));
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
                    CODE_ERROR("Number has too many decimal points.", lexerCurrentFileName, line + 1);
                }

                number += c;
            }

            tokens.push_back(new Token(_TokenType::NUMBER, number, line));
            continue;
        }
        // DOUBLE OPERATORS
        else if (c == '-' && ifs.peek() == '>')
        {
            char next = ifs.get();
            tokens.push_back(new Token(_TokenType::ASSIGN, std::string(1, c) + std::string(1, next), line));
        }
        else if (c == '!' && ifs.peek() == '=')
        {
            char next = ifs.get();
            tokens.push_back(new Token(_TokenType::NOT_EQUALS, std::string(1, c) + std::string(1, next), line));
        }
        else if (c == '>' && ifs.peek() == '=')
        {
            char next = ifs.get();
            tokens.push_back(new Token(_TokenType::GREATER_THAN_E, std::string(1, c) + std::string(1, next), line));
        }
        else if (c == '<' && ifs.peek() == '=')
        {
            char next = ifs.get();
            tokens.push_back(new Token(_TokenType::LESS_THAN_E, std::string(1, c) + std::string(1, next), line));
        }
        // SINGLE CHARACTER OPERATORS
        else if (c == '=')
        {
            tokens.push_back(new Token(_TokenType::EQUALS, std::string(1, c), line));
        }
        else if (c == '>')
        {
            tokens.push_back(new Token(_TokenType::GREATER_THAN, std::string(1, c), line));
        }
        else if (c == '<')
        {
            tokens.push_back(new Token(_TokenType::LESS_THAN, std::string(1, c), line));
        }
        else if (c == '+')
        {
            tokens.push_back(new Token(_TokenType::PLUS, std::string(1, c), line));
        }
        else if (c == '-')
        {
            tokens.push_back(new Token(_TokenType::MINUS, std::string(1, c), line));
        }
        else if (c == '*')
        {
            tokens.push_back(new Token(_TokenType::MULTIPLY, std::string(1, c), line));
        }
        else if (c == '/')
        {
            tokens.push_back(new Token(_TokenType::DIVIDE, std::string(1, c), line));
        }
        else if (c == '%')
        {
            tokens.push_back(new Token(_TokenType::MODULUS, std::string(1, c), line));
        }
        else if (c == '(')
        {
            tokens.push_back(new Token(_TokenType::OPEN_PARAN, std::string(1, c), line));
        }
        else if (c == ')')
        {
            tokens.push_back(new Token(_TokenType::CLOSE_PARAN, std::string(1, c), line));
        }
        else if (c == '{')
        {
            tokens.push_back(new Token(_TokenType::OPEN_BRACKET, std::string(1, c), line));
        }
        else if (c == '}')
        {
            tokens.push_back(new Token(_TokenType::CLOSE_BRACKET, std::string(1, c), line));
        }
        else if (c == ',')
        {
            tokens.push_back(new Token(_TokenType::COMMA, std::string(1, c), line));
        }
        else if (c == '.')
        {
            tokens.push_back(new Token(_TokenType::DOT, std::string(1, c), line));
        }
    }

    tokens.push_back(new Token(_TokenType::END_OF_FILE, EMPTY_STRING, line));

    ifs.close();

    return tokens;
}

#endif