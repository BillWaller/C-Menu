#!/bin/bash
# inst.sh install files
# Bill Waller
# billxwaller@gmail.com
#
#   echo usage: inst.sh FileName Directory DestName Mode Owner Group
#   echo "    -l      list installed files"
#   echo "    --      clear installed files list"
#

DATE=$(rfc3339)
FILENAME=$1
DSTDIR=$2
DSTNAME=$3
FILEMOD=$4
FILEOWN=$5
FILEGRP=$6
EUSER=$(whoami)
if [ "$EUSER" != "root" ]; then
    echo Root privileges recommended for install
    key=$(enterchr "Press 'C' to continue or any other key to abort.")
    key=$(toupper "$key")
    if [ "$key" != "C" ]; then
        echo Aborting install
        exit 1
    fi
fi
if [ "$#" -eq 1 ]; then
    if [ "$1" = "-l" ]; then
        if [ -f installed ]; then
            echo Installed files:
            which lsd >/dev/null 2>&1
            if which lsd >/dev/null 2>&1; then
                awk '{printf("%s\n", $2)}' installed | while read -r line; do
                    lsd -l "$line"
                done
            else
                cat installed | while read -r line; do
                    ls --color=always -l "$line"
                done
            fi
            exit 0
        else
            echo "No files have been installed yet."
            exit 0
        fi
    fi
    if [ "$1" = "--" ]; then
        rm -f installed
        exit 0
    fi
fi
if [ "$#" != 6 ]; then
    echo usage: inst.sh FileName Directory DestName Mode Owner Group
    echo "    -l      list installed files"
    echo "    --      clear installed files list"
    exit 1
fi
if [ ! -f "$FILENAME" ]; then
    echo file "$FILENAME" not found
    echo usage: inst.sh FileName Directory DestName Mode Owner Group
    exit 1
fi
if [ ! -d "$DSTDIR" ]; then
    if mkdir -p "$DSTDIR"; then
        echo created directory "$DSTDIR"
    else
        echo unable to create directory "$DSTDIR"
        echo usage: inst.sh FileName Directory DestName Mode Owner Group
        exit 1
    fi
fi
if [ -f /tmp/"$FILENAME".old ]; then
    rm -f /tmp/"$FILENAME".old
fi
if [ -f "$DSTDIR/$DSTNAME" ]; then
    mv "$DSTDIR/$DSTNAME" /tmp/"$FILENAME".old
fi
echo "$DATE $DSTDIR/$DSTNAME" >>installed
cp "$FILENAME" "$DSTDIR/$DSTNAME"
chown "$FILEOWN":"$FILEGRP" "$DSTDIR/$DSTNAME"
chmod "$FILEMOD" "$DSTDIR/$DSTNAME"
