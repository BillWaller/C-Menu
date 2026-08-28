When you have two separate Linux C programs utilizing , and one behaves normally (blocking until a key is pressed) while the other blocks sporadically, the issue almost always stems from how the underlying  file descriptor is handled, concurrent system interactions, or signal delivery. [1]  
Under the hood,  loops on an internal  or  mechanism against . If it returns instantly without an input event, it retries—causing "sporadic" blocking or spinning. [1, 2]  
The most likely causes for this behavior include: 
1. Multiple Processes Sharing the Same TTY () 
If both programs (or one Notcurses program and a background shell/process) are running in the same terminal window simultaneously, they are competing for . 

• The Cause: Linux distributes TTY input to whichever process reads it first. If the other program or a background shell "steals" the characters or EOF from the TTY stream, Notcurses' internal read loop gets interrupted or receives a 0-byte read, causing  to cycle rapidly or behave unexpectedly. 
• The Fix: Ensure they run in separate TTYs (separate terminal windows/tabs) or that one is not accidentally running in the background () of the same session. 

2. Differing Signal Handling () 
 blocks until an input event occurs or until it is interrupted by a signal. 

• The Cause: If the sporadically-blocking program has active timers (, ), handles  (window resizes), or receives custom inter-process signals (like ), the underlying / system call will fail with . 
• The Fix: Notcurses internally handles  to return . If your sporadic program is receiving other frequent signals, you must either mask those signals during the read or ensure your own signal handlers are registered with the  flag via . [4]  

3. Mixing Standard I/O Functions 
The sporadically-blocking program might be mixing traditional C standard library I/O functions with Notcurses. 

• The Cause: If the problematic program calls functions like , , or  anywhere else in the code, those functions invoke user-space buffering on . They will aggressively read blocks of data from the OS into the  buffer. When Notcurses subsequently tries to read from the raw file descriptor (), the data has already been eaten by the standard library buffer, throwing off Notcurses' internal state machine. 
• The Fix: Never mix  input functions with Notcurses input tracking. Use only Notcurses input API calls. [6]  

4. Spawning Subprocesses (Forking) 
If the sporadic program uses , , or , the child process inherits the  file descriptor. 

• The Cause: If a spawned child process does not immediately close  or replace it via , it can manipulate the TTY properties or read from the shared stream. Furthermore, when the child terminates, it sends a  signal to the parent, which breaks the parent's blocking  call via . 
• The Fix: Block  around input loops or handle it cleanly, and ensure child processes decouple their I/O streams. 

5. Multithreading Violations 
If the sporadic program is multithreaded and more than one thread is trying to read input, it will break. 

• The Cause: According to the official Notcurses documentation, only one thread may call  or any input-related function at a time. Concurrent calls will cause data races over the input buffer and sporadic blocking behavior. 
• The Fix: Isolate all input reading to a single dedicated thread. [7]  

How to Diagnose 
To pinpoint the exact culprit, run the malfunctioning program through  to observe what happens when it unblocks sporadically: 

• If you see  or  continuously without typing, an external configuration or thread is touching . 
• If you see  followed by , an unhandled signal is cutting the block short. 

To help narrow this down further, let me know: 

• Are these two programs running simultaneously in the same terminal? 
• Is the sporadic program multithreaded, or does it use timers/signals? 

AI responses may include mistakes.

[1] https://notcurses.com/notcurses_input.3.html
[2] https://github.com/dankamongmen/notcurses/issues/2873
[3] https://github.com/dankamongmen/notcurses/blob/master/USAGE.md
[4] https://notcurses.com/notcurses_init.3.html
[5] https://notcurses.com/notcurses_refresh.3.html
[6] https://github.com/dankamongmen/notcurses/blob/master/doc/CURSES.md
[7] https://notcurses.com/notcurses.3.html

----------------------------------------------------
     pipe_fd[P_READ]  11

327  dup2(pipe_fd[P_READ], STDIN_FILENO);
     view->in_fd = dup(STDIN_FILENO);

     view->in_fd    11

----------------------------------------------------

view->in_fd = memfd_create("view_input", MFD_CLOEXEC);  11

401 close(view->in_fd);


