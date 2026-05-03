![C-Menu Performance](screenshots/Performance.png)

# C-Menu Performance

## Design Philosophy and Optimizations

To build truly outstanding applications with C-Menu, it is essential to understand the design philosophy and optimizations that C-Menu provides to achieve superior performance. By understanding and applying these principles, the developer can leverage C-Menu's features effectively and create highly responsive applications.

### C-Menu Launcher

When you use C-Menu's Example Application Menu, you will notice that most menu selections respond instantaneously with no perceptible delay. It just snaps. That level of optimization is achieved in part by avoiding the overhead and unpredictability of using a shell to execute command lines. C-Menu provides the capability to execute command lines directly, which results in start-up times an order of magnitude faster than traditional shell-based menu systems. But, C-Menu has something even faster. You can take C-Menu performance to the next level by letting C-Menu call it's internal functions directly, bypassing the start-up overhead of external executables. In the Example Applications Menu, all except the first command line are internal function calls, which execute in nanoseconds compared to the milliseconds it takes to launch an external executable.

Here's a breakdown of the performance levels you can expect when using C-Menu:

Relative performance:

| Performance level | Description             | Elapsed Time |
| ----------------- | ----------------------- | ------------ |
| Somewhat Slugish  | Shell based execution   | 10-100 ms    |
| Fast              | Direct execution        | 1-10 ms      |
| Instantaneous     | Internal function calls | 0.001 ms     |

At 200 ms response, a user will perceive an application as sluggish. At 20 ms, the user will perceive a smooth and responsive experience. At 1 ms, the user will perceive an instantaneous response. Admittedly, no one really cares whether a program loads in 0.001 ms or 1,000 times slower at 1 ms, but in practical applications iterative processes often take thousands of cycles to complete.

Sometimes it is expedient to use a shell to execute a command line, such as when you want to use shell features like globbing, variable expansion, or complex command pipelines. In those cases, you can explicitly invoke a shell by including "sh -c" or a shell script on the command line.

When designing with C-Menu, you should use in order:

- C-Menu internals, such as Menu, Form, Pick, and View
- direct execution
- shell

C-Menu was designed to support internal function calls and direct execution, so it provides the most commonly needed conveniences such as tilde expansion, file location based on the environment, and piped input and output.

For piped input and output with internal function calls and direct execution, C-Menu allows the specification of providers and receivers for pipe I-O. Instead of using I/O redirection on the command line with pipe symbols, C-Menu provides more controllable alternatives such as "-S" for specifying a command to execute as a provider (source) of input to a form, pick, or view, "-R" for specifying a command to receive standard output from a form, pick, or view, and "-c" for specifying a command to execute with the selected item as an argument. These features allow you to create powerful and flexible menu items that can interact with other applications and scripts in a more controlled and efficient manner.

If an internal function call will do, use an internal function call. If not, consider direct execution. If you need features provided by a shell, use a shell.

---

### C-Menu Lightweight Find (lf)

C-Menu's lf (lightweight find) is an alternative to Unix find. Although Unix find is an extremely powerful tool, and it is not slow, it can be unwieldy at times (see the 40 page manual). It does everything you could want, but with unnecessary overhead and complexity. C-Menu's lf is smaller, faster, and easier to use than Unix find, yet covers most of the day-to-day use cases.

One of find's most often used features is the built-in -exec option, which executes a specified command on each file found. Conspicuously, lf does not have a built-in -exec option, and that is one of the first things people notice. However, lf achieves the same result, by piping the output of lf into xargs. Intuitively, it makes sense that find with its built-in exec option would be faster than lf using an external xargs command. We compared C-Menu's lf with xargs and find with its built-in -exec option. Both methods produced identical and accurate results, but the performance benchmarks were not what we expected.

time find . -maxdepth 5 -type f -exec ls -l {} \; >find.out

time lf -d 4 -t f | xargs ls -l >lf.out

| Command | real     | user     | sys      | files found |
| ------- | -------- | -------- | -------- | ----------- |
| find    | 0m0.469s | 0m0.160s | 0m0.288s | 142         |
| lf      | 0m0.008s | 0m0.004s | 0m0.006s | 142         |

time find . -maxdepth 4 -type f -exec ls -l {} \; >find.out

time lf -d 4 -t f | xargs ls -l >lf.out

| Command | real     | user     | sys      | files found |
| ------- | -------- | -------- | -------- | ----------- |
| find    | 0m2.123s | 0m0.788s | 0m0.281s | 598         |
| lf      | 0m0.014s | 0m0.007s | 0m0.009s | 598         |

lf with xargs is significantly faster than find with its built-in -exec option. find, with its built-in exec option executes the specified command for each file found, which can be very inefficient when dealing with a large number of files. In contrast, using xargs allows you to execute the command on multiple files at once, which can significantly reduce the overhead and improve performance. In fairness, you can improve the performance of find dramatically by piping its output into xargs instead of using its built-in -exec option. In other benchmarks, lf performed about 35% faster than find.

Even if lf wasn't faster than find, it would still be a compelling alternative due to its simplicity and ease of use. With fewer options and a more intuitive syntax, lf can be easier to learn and use for common file searching tasks. Additionally, lf's performance advantages make it an attractive choice for users who need to search through large directories or perform complex searches.

With a variety of tools in your toolbox, use a tool that is fit for purpose. Don't drive tacks with pile driver or piles with a tack hammer. If the task calls for the full power of find, use find. If not, use lf.

---

### C-Menu View

Throughout C-Menu, and especially View, you will find many optimizations that
contribute to it's efficiency and speed. Traditionally, large file I-O has relied on user-space buffering schemes in which chunks of data are copied from mass storage into local buffers using seek and read operations. The application must keep track of buffer contents, manage buffer lifecycles, and handle edge cases such as partial reads, end-of-file conditions, and error handling. This approach can be complex, error-prone, and inefficient, especially when dealing with large files or high-throughput applications. C-Menu's view takes a different approach to large file I-O by leveraging the operating system's virtual memory management capabilities to provide direct access to file data through memory mapping. Instead of relying on user-space buffering, C-Menu's view provides a direct-to-kernel, demand paged, memory mapped virtual address space for file access. This eliminates the overhead and complexity associated with user-space buffering, and allows for more efficient and reliable access to large files. With C-Menu's view, applications can access any part of a multi-gigabyte file instantly without the need for copying data into user-space buffers or managing buffer lifecycles. This results in unmatched reliability and performance when working with large files, making C-Menu's view an ideal choice for applications that require high-throughput file access or need to work with large datasets.

---
