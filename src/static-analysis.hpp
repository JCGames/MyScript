#ifndef STATIC_ANALYSIS
#define STATIC_ANALYSIS

#include "parser.hpp"

void analyze_define_symbols(StatementBlock* block)
{
    for (auto* stmt : block->statements)
    {
        switch (stmt->type)
        {
            case StatementType::FUNCTION:
            {
                auto func = (StatementFunction*)stmt;
                block->definedSymbols.push_back(DefinedSymbol(func->name, func));
            }
                break;
            case StatementType::STRUCT:
            {
                auto _struct = (StatementStruct*)stmt;
                block->definedSymbols.push_back(DefinedSymbol(_struct->name, _struct));
            }
                break;
        }
    }
}

void analyze_stmt(Statement* stmt, SyntaxTree& tree)
{
    switch (stmt->type)
    {
        case StatementType::BLOCK:
        {
            auto block = (StatementBlock*)stmt;
            analyze_define_symbols(block);

            for (auto& bStmt : block->statements)
                analyze_stmt(bStmt, tree);
        }
            break;
        case StatementType::NAMESPACE:
        {
            auto _namespace = (StatementBlockNamespace*)stmt;
            analyze_define_symbols(_namespace);

            for (auto& nsStmt : _namespace->statements)
                analyze_stmt(nsStmt, tree);
        }
            break;
    }
}

void analyze_tree(SyntaxTree& tree)
{
    for (auto& stmt : tree.statements)
        analyze_stmt(stmt, tree);
}

#endif