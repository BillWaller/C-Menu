#!/bin/bash

DOXY_EXAMPLE_PATH=/usr/local/src/doxygen/build/html/examples
/usr/bin/google-chrome-stable $DOXY_EXAMPLE_PATH/$1/html/index.html >/dev/null 2>&1
