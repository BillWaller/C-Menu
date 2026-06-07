#!/usr/bin/env bash
# Function: iso8601_timestamp
# -l for local time
if [ "$1" = "-l" ]; then
    date +"%Y-%m-%dT%H:%M:%S%z"
else
    date -u +"%Y-%m-%dT%H:%M:%SZ"
fi
