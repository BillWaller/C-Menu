# @name dirs.sh
# @brief List directories in the current directory, excluding hidden directories
# and common build/output directories
# @details This script uses the `find` command to list directories in the
# current directory while excluding hidden directories (those starting with a
# dot) and common build/output directories such as `node_modules`, `dist`,
# `build`, `out`, `coverage`, `public`, `assets`, and `src`. The output is
# sorted alphabetically and formatted to remove the leading './' from the
# directory names.
find . -maxdepth 1 -type d -not -name '.*' -not -name 'node_modules' -not -name 'dist' -not -name 'build' -not -name 'out' -not -name 'coverage' -not -name 'public' -not -name 'assets' -not -name 'src' | sort | sed 's/^\.\///'
