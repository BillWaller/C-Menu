#!/usr/bin/env bash

# Function: iso8601_timestamp
# Usage: iso8601_timestamp [-l]
#  -l : Output local time (default is UTC)

iso8601_timestamp() {
    local use_local=false
    while getopts ":l" opt; do
        case "$opt" in
        l) use_local=true ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            return 1
            ;;
        esac
    done
    shift $((OPTIND - 1))
    if $use_local; then
        date +"%Y-%m-%dT%H:%M:%S%z"
    else
        date -u +"%Y-%m-%dT%H:%M:%SZ"
    fi
}

echo "UTC:   $(iso8601_timestamp)"
echo "Local: $(iso8601_timestamp -l)"
