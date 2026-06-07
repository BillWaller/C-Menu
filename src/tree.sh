#!/bin/bash

# Function to recursively map directories and files
traverse_dir() {
    local dir="$1"
    local prefix="$2"

    # Fetch all items, including hidden ones if preferred
    local entries=("$dir"/*)

    # Handle empty directories gracefully
    if [ ! -e "${entries[0]}" ]; then
        return
    fi

    local count=${#entries[@]}
    local index=0

    for entry in "${entries[@]}"; do
        ((index++))
        local filename=$(basename "$entry")

        # Determine formatting graphics based on position
        if [ $index -eq $count ]; then
            echo "${prefix}└── ${filename}"
            local next_prefix="${prefix}    "
        else
            echo "${prefix}├── ${filename}"
            local next_prefix="${prefix}│   "
        fi

        # Recurse if the entry is a directory
        if [ -d "$entry" ] && [ ! -L "$entry" ]; then
            traverse_dir "$entry" "$next_prefix"
        fi
    done
}

# Target directory defaults to current directory if left blank
target_dir="${1:-.}"
echo "$target_dir"
traverse_dir "$target_dir" ""
