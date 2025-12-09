#include <string.h>
#include <tree_sitter/api.h>

extern const TSLanguage *tree_sitter_c();

int main() {
    TSParser *parser = ts_parser_new();

    ts_parser_set_language(parser, tree_sitter_c());

    const char *source_code = "int main() { return 0; }";
    TSTree *tree =
        ts_parser_parse_string(parser, NULL, source_code, strlen(source_code));

    TSNode root_node = ts_tree_root_node(tree);

    // (Load and execute highlighting queries here)
    // use tree-sitter's query API to match patterns
    // and extract highlight ranges.

    // (Apply highlighting based on query results)

    ts_tree_delete(tree);
    ts_parser_delete(parser);

    return 0;
}
