#!/usr/bin/env bash

# NAME:        lfin.sh
# DESCRIPTION: regular expression file search from colon-separated paths
# AUTHOR:      Bill Waller <billxwaller@gmail.com>
# DATE:        2026-10-02
# VERSION:     1.0.0

# Exit immediately if a command exits with a non-zero status
# set -Eeuo pipefail
# Second thought, we will not use the above line because it will cause the script
# to exit on any error, which may not be desirable in this case.

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS] [ARGUMENT]

regular expression file search from colon-separated paths

Options:
  -h, --help     Display this help message and exit
  -v, --version  Output version information and exit
  -n, --name     Specify a target name

Arguments:
  paths          Colon-separated list of paths to search
  re             Regular expression to match files

Example:
  $(basename "$0") \$MANPATH '\/view.*$'
EOF
    exit 0
}

version() {
    echo "$(basename "$0") version 1.0.0"
    exit 0
}

parse_params() {
    paths=""
    re=""
    while [[ $# -gt 0 ]]; do
        case "$1" in
        -h | --help)
            usage
            ;;
        -v | --version)
            version
            ;;
        -*)
            echo "Error: Invalid option $1" >&2
            usage
            ;;
        *)
            if [ "$paths" == "" ]; then
                paths="$1"
            else
                if [ "$re" == "" ]; then
                    re="$1"
                else
                    echo "Error: Unexpected argument $1" >&2
                    usage
                fi
            fi
            shift
            ;;
        esac
    done
}

main() {
    parse_params "$@"
    local old_ifs="$IFS"
    IFS=":"
    for dir in $paths; do
        [ -z "$dir" ] && continue
        lf -r "$re" "$dir"
    done
    IFS="$old_ifs"
}

main "$@"
