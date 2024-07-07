#include "runtime.hpp"

int main(int argc, char** argv)
{
    if (argc > 1)
    {
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