#!/bin/bash

export DOXY_EXAMPLE_PATH=/usr/local/src/doxygen/build/html/examples
cd "$DOXY_EXAMPLE_PATH"
for file in $(lf 'index\.html$'); do
  dirname "$file" | cut -d '/' -f 1
done | pick -n 1 -c "doxy_chrome.sh %%"
