add-auto-load-safe-path /usr/bin
set pagination off
set directories /usr/bin
set auto-load python-scripts on
set disassembly intel
set startup-quietly on
set debuginfod enabled on
tui enable
define dbg_lchild
  "List child processes of the current process."
  let current_pid = gdb.execute("print getpid()", to_string=True).split('=')[1].strip()
  let child_pids = gdb.execute("info proc children", to_string=True)
  processes of PID " . current_pid . ":"
  echo child_pids
end
define dbg_child
  set follow-fork-mode child
end
define dbg_parent
  set follow-fork-mode parent
end
define dbg_bt
  let backtrace = gdb.execute("backtrace full", to_string=True)
  echo "Backtrace with function arguments:"
  echo backtrace
end
define dbg_lt
  let threads = gdb.execute("info threads", to_string=True)
  echo "Threads in the current process:"
  echo threads
end
set inferior-tty /dev/pts/0
