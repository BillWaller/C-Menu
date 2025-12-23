#!/bin/bash
#
# A simple wrapper around lf to pick all .c files in the current directory
# and its subdirectories
#
# lf, unlike ls, uses regular expressions for filtering. That's the excuse for
# the strange styntax.
# We use '.*\.c$' to match all files ending with ".c".  Optionally, you can pipe
# the output to 'pick' for further selection or to open the files in 'vi' editor
# by adding -c vi to the pick command
#
lf -r ./ '.*\.c$'
#| pick -c vi
