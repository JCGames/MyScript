
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

        // include standard libraries
        for (auto& fileName : fileNames)
            lexer_lexify(libPathStr + "/" + fileName);

        lexer_lexify(argv[1]);
        // lexer_print_tokens();

        parser_parse_statements();
        // parserAstRoot->print(EMPTY_STRING);

        rt_run();

        // do deletes
        lexer_delete_tokens();
        delete parserAstRoot;
    }
}