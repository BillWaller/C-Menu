#!/bin/bash

lf |
    awk -F'/' '
{
    for (i=1; i<NF; i++) {
        printf (i == NF-1) ? "├── " : "│   "
    }
    print $NF
}'
