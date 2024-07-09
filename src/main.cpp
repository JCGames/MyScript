
#include <iostream>

#include "path.hpp"
#include "parser.hpp"

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        Path libPath = Path::get_executable_path().back() / "stdlib";

        auto libPathStr = libPath.to_string();
        auto fileNames = libPath.get_files();

        auto ast = new StatementBlock();

        // include standard libraries
        for (auto& fileName : fileNames)
        {
            auto tokens = lexer_lexify(libPathStr + "/" + fileName);
            auto block = parser_parse_statements_from_tokens(tokens);
            lexer_delete_tokens(tokens);

            ast->statements.push_back(block);
        }

        auto tokens = lexer_lexify(argv[1]);
        auto block = parser_parse_statements_from_tokens(tokens);
        lexer_delete_tokens(tokens);

        ast->statements.push_back(block);

        ast->print(EMPTY_STRING);
    }
}