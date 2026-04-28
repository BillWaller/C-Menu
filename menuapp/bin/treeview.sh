#!/bin/bash
# @name treeview.sh
# @brief Display a tree view of the current directory
# @author ChatGPT
lf |
    awk -F'/' '
{
    for (i=1; i<NF; i++) {
        printf (i == NF-1) ? "├── " : "│   "
    }
    print $NF
}'
