---
title: "lf - lite find"
section: 1
header: User Manuals
footer: C-Menu Version 0.2.9
author: Bill Waller
date: June 2026
---

# NAME

lf - lite find

# SYNOPSIS

lf [-HiLRS?V] [-a time] [-b time] [-d number] [-D 12345678] [-e regex]
[-p sgrwx] [-r regex] [-s size] [-t pcdbflsu] [-T threads]
[-u user name] [--after=time] [--before=time] [--max_depth=number]
[--debug=12345678] [--ere=regex] [--include_hidden] [--ignore_case]
[--follow_links] [--include_perms=sgrwx] [--re=regex]
[--sort_reverse] [--file_size_min=size] [--sort]
[--include_types=pcdbflsu] [--nthreads=threads] [--user=user name]
[--help] [--usage] [--version] [DIRECTORY] [REGULAR_EXPRESSION]

# DESCRIPTION

lf recursively reads directories, listing files that match the specified criteria.

Processing is concurrent on systems with multi-threading support, resulting in
significant performance improvements when searching large directory trees.

lf is easy to use, with a simple and intuitive command-line interface,
providing a variety of options for customizing the search criteria.

# OPTIONS

-a, --after=Modified after YYYY-MM-DDTHH:MM:SS

-b, --before=Modified before YYYY-MM-DDTHH:MM:SS

    Dates use the ISO 8601 format. The T is a separator between the date and
    time. If the time is not specified, it defaults to 00:00:00. If the date
    is not specified, it defaults to the current date.

-d, --max_depth=Depth into directory tree

    Default depth is 0, which means no limit. A depth of 1 means only the
    specified directory, 2 means the specified directory and its immediate
    subdirectories, and so on.

-D, --debug=12345678

    The debug option can be used to print various levels of debugging
    information. The levels are as follows:

    1-config        Print the configuration settings.
    2-info          Print informational messages about the program execution.
    3-warnings      Print warning messages about potential issues.
    4-errors        Print error messages about problems encountered during
                    execution.
    5-badlinks      Print messages about broken symbolic links.
    6-trace         Trace ancestor scans for cyclic links.
    7-all           Print all debugging information (config, info, warnings,
                    errors and badlinks).
    8-only_errors:  Print only error messages. This is useful for
                    examining a directory tree for errors.

    Debug option arguments may be combined in any order. For example, if
    you want to examine a directory tree for bad and problem links, you

-e, --ere=Exclude regular expression

    The regular expression should be a properly formatted regular expression
    for which matching files will be excluded from the results.

-H, --include_hidden Include hidden files

    Use -H to include hidden files and directories, which lf excludes
    by default.

-i, --ignore_case ignore case in search

    Use -i to ignore case when searching for files that match the regular
    expression. By default, lf is case-sensitive.

-L, --follow_links Follow symbolic links

    Use -L to follow symbolic links. By default, lf does not follow symbolic
    links.

-p, --include_perms=sgrwx

    Use -p to include only files with the specified permissions.

    x-execute
    w-write
    r-read
    s-setuid
    g-setgid

    For example, if you want to include only files that have read and write
    permissions you would use -prw.

    To list all files with the setuid bit, you would use -ps.

-r, --re=Regular expression to search for

    The regular expression should be a properly formatted regular expression
    for which matching files will be listed in the results. By default, lf
    lists all files that match the other criteria specified by the options.
    If you specify a regular expression, only files that match the regular
    expression will be listed. A regular expression may be specified
    as the second non-option positional argument on the command line or
    as an argument to the -r option.

-R, --sort_reverse Sort in Reverse order

    Use -R to sort the results in reverse order. By default, results
    are not sorted.

-s, --file_size_min=minimum size

    No Suffix-bytes
    K-kilobytes
    M-Megabytes, or
    G-Gigabytes

    Use -s to include only files that are at least the specified size. A
    suffix may be used to specify the size in bytes, kilobytes, megabytes or
    gigabytes. For example, -s10M would include only files that are at least
    10 megabytes in size.

-S, --sort Sort in Ascending order

    Use -R to sort the results in ascending order. By default, results
    are not sorted.

-t, --include_types=pcdbflsu

    p-pipe
    c-character_dev
    d-directory
    b-block_dev
    f-regular_file
    l-link
    s-socket
    u-unknown

    Use -t to include only files of the specified types. To include only
    regular files and directories, you would use -tdf.

-T, --nthreads=Number of threads to use for searching

    Use -T to specify the number of threads to use for searching. By default,
    lf queries the operating system for the number of CPU cores and uses
    about 40% of that value as the number of threads. The optimal number of
    threads to use depends on the number of files in the directory tree and
    the speed of the storage device. If you are searching a directory tree
    with a large number of files, you may want to increase the number of
    threads. If you are searching a directory tree with a small number of
    files, you may want to decrease the number of threads.

-u, --user=User Name of file owner

    Use -u to include only files owned by the specified user. The user name
    should be the login name of the user, not the user ID number.

-?, --help

    Give this help list

--usage

    Give a short usage message

-V, --version

    Print program version

A space after short options is optional. For example, -s10M and -s 10M are both valid.

Option arguments may be ganged. For example, to list all files, directories, and
links, you can use -t f -t d -t l or -tfdl.

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

If specified, DIRECTORY is the top-level directory to search.
REGULAR_EXPRESSION is a properly formatted regular expression for which
matching files will be listed.

# EXAMPLES

List all files in the current directory and its subdirectories that have a .txt extension:

    lf -r '.*\.txt$'

List all files in the /var/log directory that are larger than 100 megabytes:

    lf -s 100M /var/log

List all files in the /home directory that were modified after January 1, 2025:

    lf -a 2025-01-01T00:00:00 /home

List all files in the /usr directory that are owned by the user "bill":

    lf -u bill /usr

List all files in the /tmp directory that are symbolic links:

    lf -t l /tmp

List all files in the /var directory that have read and write permissions for the owner:

    lf -p rw /var

List all files in the /home directory that have a .log extension and were modified before June 1, 2025:

    lf -r '.*\.log$' -b 2025-06-01T00:00:00 /home

List broken or bad symbolic links:

    lf -D458 /path/to/directory

Count the number of files in a directory tree using 7 threads:

    lf -L -H -T7 /path/to/directory | wc -l

# REPORTING BUGS

Report bugs to <billxwaller@gmail.com>.

# COPYRIGHT

Copyright © 2026 Bill Waller.

# LICENSE

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# SEE ALSO

C-Menu Menu, Form, Pick, View, RSH, C-Keys
