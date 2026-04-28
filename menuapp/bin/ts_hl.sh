#!/bin/bash
# @name ts_hl.sh
# @brief Uses tree-sitter CLI to highlight code
# @details requires tree-sitter CLI and appropriate parser installed for the language
# @usage ts_hl.sh <file>
if [ "$(which tree-sitter)" ]; then
    tree-sitter highlight "$@" 2>/dev/null
else
    echo "tree-sitter not found."
    echo "Please install tree-sitter to use this script."
    enterchr "Press any key to exit..."
    exit 1
fi
