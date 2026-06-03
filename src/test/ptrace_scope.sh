#!/bin/bash
# @name ptrace_scope.sh
# @desc Enable or disable ptrace_scope kernel parameter.
# @usage ptrace_scope.sh [0123]
# The ptrace_scope kernel parameter controls the behavior of the ptrace system call,
# which is used for debugging and process tracing. It is designed to enhance
# security by restricting the ability of processes to trace or debug other
# processes. The parameter can be set to different levels of restriction,
# allowing system administrators to balance security and functionality based on
# their needs.
#
# 0 - (Classic Mode): Any process can trace any other process running under the
#     same UID, provided it is "dumpable".
# 1 - (Restricted Mode - Default): A process can only trace its direct
#     descendants (child processes). To trace an unrelated process, the target
#     process must explicitly declare the debugger as an allowed tracer via
#     prctl(PR_SET_PTRACER, ...).
# 2 - (Admin-Only Attach): Only processes holding the CAP_SYS_PTRACE capability
#     (typically run as root) can use ptrace to attach to other processes.
# 3 - (No Attach): No process can trace any other process. Once set to \(3\),
#     this value cannot be changed without a system reboot.
getptrace_scope() {
    case "$1" in
    0) echo "Classic Mode (0)" ;;
    1) echo "Restricted Mode (1)" ;;
    2) echo "Admin-Only Attach Mode (2)" ;;
    3) echo "No Attach Mode (3)" ;;
    *) echo "Unknown: $1" ;;
    esac
}
if [ "$(id -un)" != "root" ]; then
    echo Please run as root
    exit 1
fi
level=$(cat /proc/sys/kernel/ptrace_scope)
desc=$(getptrace_scope "$level")
echo read ptrace_scope level: "$level" "$desc"

if [ "$#" != "1" ]; then
    echo usage: "$0" "[0123]"
    exit 1
else
    if [ "$1" -ge "0" ] && [ "$1" -le "4" ]; then
        echo "$level" >/proc/sys/kernel/ptrace_scope
        desc=$(getptrace_scope "$level")
        echo set ptrace_scope level: "$level" "$desc"
    else
        echo usage: "$0" "[0123]"
        exit 1
    fi
fi
