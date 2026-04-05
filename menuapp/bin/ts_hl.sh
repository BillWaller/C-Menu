#!/bin/bash
# ts_hl.sh
if [ $(which tree-sitter) ]; then
    tree-sitter highlight "$@" 2>/dev/null
else
    echo "tree-sitter not found."
    echo "Please install tree-sitter to use this script."
    enterchr "Press any key to exit..."
    exit 1
fi
