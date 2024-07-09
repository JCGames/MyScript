
#include <iostream>

#include "helpers/path.hpp"
#include "parser.hpp"

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        Path libPath = Path::get_executable_path().back() / "stdlib";

        auto libPathStr = libPath.to_string();
        auto fileNames = libPath.get_files();

        auto tree = new SyntaxTree();

        // include standard libraries
        for (auto& fileName : fileNames)
        {
            auto tokens = lexer_lexify(libPathStr + "/" + fileName);
            parser_get_syntax_tree_from_tokens(tree, tokens);
            lexer_delete_tokens(tokens);
        }

        auto tokens = lexer_lexify(argv[1]);
        parser_get_syntax_tree_from_tokens(tree, tokens);
        lexer_delete_tokens(tokens);

        tree->print();

        delete tree;
    }
}