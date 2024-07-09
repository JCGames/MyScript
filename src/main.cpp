
#include <iostream>

#include "path.hpp"
#include "runtime.hpp"

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
            lexer_lexify(libPathStr + "/" + fileName);
            auto block = parser_parse_statements();
            ast->statements.push_back(block);
        }

        lexer_lexify(argv[1]);
        auto block = parser_parse_statements();
        ast->statements.push_back(block);

        lexer_print_tokens();
        ast->print(EMPTY_STRING);

        rt_run(ast);

        // do deletes
        lexer_delete_tokens();
    }
}