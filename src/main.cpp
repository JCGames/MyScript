
#include <iostream>

#include "helpers/path.hpp"
#include "runtime.hpp"

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        // Path libPath = Path::get_executable_path().back() / "stdlib";

        // auto libPathStr = libPath.to_string();
        // auto fileNames = libPath.get_files();

        SyntaxTree tree;

        // // include standard libraries
        // for (auto& fileName : fileNames)
        // {
        //     auto tokens = lexer_get_tokens(libPathStr + "\\" + fileName, fileName);
        //     parser_get_syntax_tree(tree, tokens);
        //     lexer_delete_tokens(tokens);
        // }

        auto tokens = lexer_get_tokens(argv[1], argv[1]);
        parser_get_syntax_tree(tree, tokens);
        lexer_delete_tokens(tokens);

        // analyze_tree(tree);
        tree.print();

        // rt_run_tree(tree);
    }
}