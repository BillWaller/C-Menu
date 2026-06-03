---
title: "rsh - Root Shell Helper"
section: 1
header: User Manuals
footer: C-Menu Version 0.2.9
author: Bill Waller
date: June 2026
---

# NAME

rsh

# SYNOPSIS

rsh [-i] [-D] [COMMAND_ARGUMENTS...]

# DESCRIPTION

rsh is a helper utility for C-Menu that allows users to execute commands with root privileges. It provides a secure and convenient way to run commands that require elevated permissions.

# NOTE

Throughout this manual, you will see references to "user_name". This is a placeholder for the actual username of the user who has installed C-Menu. When you see "user_name", replace it with the actual username to ensure that the instructions and commands work correctly on your system.

# ROOT AUTHORITY

C-Menu build installs rsh with root ownership and the setuid bit turned on. This
means that when a user executes rsh, it runs with the privileges of the root user, allowing it to perform actions with elevated permissions. Although useful, this feature must be used with caution, as it can potentially be exploited if not properly secured.

The first line of protection is inherent to the OS. If rsh is not owned by root
with its setuid bit turned on, it is no more dangerous than any other binary.

We will assume the person who installed C-Menu is a trusted user, who will understand the implications and take necessary precautions to secure the rsh binary and limit its usage to authorized users only. While there are compelling reasons for top-level administrators and developers to have and use rsh, there is no case for allowing unrestricted access.

# SECURE PRACTICES

To mitigate potential vulnerabilities associated with rsh, it is crucial to follow secure practices when using and managing the rsh binary. This includes restricting access to the binary, monitoring its usage, and configuring it to use additional security measures such as PAM (Pluggable Authentication Modules).

# HOW DEVELOPERS AND ADMINISTRATORS USE RSH

Developers can use rsh to execute commands with root privileges, which can be particularly useful for tasks that require elevated permissions, such as system administration, debugging, or testing. For example, a developer might use rsh to edit system configuration files, manage services, or perform other administrative tasks without needing to switch to a root shell.

Occasionally, there is a need for users to access privileged resources, such as system logs or configuration files. In such cases, rsh can be used to grant temporary access to these resources without giving users full root access. There are any number of ways this can be accomplished.

A time-based certificate system that will grant temporary access is currently under development.

## Restrict Access

Distribute fully enabled rsh binaries only to trusted users who need administrative access to accomplish their work. Set appropriate file permissions and use access control lists (ACLs) if necessary. For example, you can set the permissions to allow only specific users or groups to execute rsh:

    chmod 4750 /home/user_name/menuapp/bin/rsh
    chown root:admin_group /home/user_name/menuapp/bin/rsh

## Monitor Usage

    Regularly monitor the usage of rsh to detect any unauthorized access or suspicious activity. This can be done by reviewing system logs and using intrusion detection systems (IDS) to identify potential threats. If you have systemd, use journalctl to audit rsh usage as follows:

    journalctl -r /home/user_name/menuapp/bin/rsh

    Requires rsh to be built with -DRSH_LOG

## PAM

    rsh can be configured to use Pluggable Authentication Modules (PAM) for additional security. This allows for more flexible authentication methods and can help prevent unauthorized access.

    Requires rsh to be built with -DRSH_PAM

## USAGE

# OPTIONS

    -i  interactive shell

        rsh starts a login shell by default.

    -D2  display result of authentication

        By default, rsh does not display the result of the authentication process. This option enables the display of authentication results, which can be useful for debugging or verifying that the authentication process is working correctly.

# EXAMPLES

To start a login shell:

    rsh

To start an interactive root shell:

    rsh -i

To execute a command with root privileges:

    rsh vi /etc/passwd

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
