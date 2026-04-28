#!/bin/bash
# @name doxy_chrome.sh
# @description Open Doxygen example in Google Chrome
DOXY_EXAMPLE_PATH=/usr/local/src/doxygen/build/html/examples
/usr/bin/google-chrome-stable $DOXY_EXAMPLE_PATH/$1/html/index.html >/dev/null 2>&1
