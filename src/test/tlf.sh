#!/bin/bash
# Example usage of lf and pick commands
# List all .c files in the current directory and pipe the output to pick command
# The picked files will be saved to pick.out

lf ./ '.*\.c$' | pick >pick.out
