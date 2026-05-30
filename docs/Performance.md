![C-Menu Performance](../screenshots/Performance.png)

**_NEW_** C-Menu Installation Guide

[C-Menu User Guide](C-Menu-UG.md)

[C-Menu Installation Guide](INSTALL.md)

# Design Philosophy and Optimizations

To build truly outstanding applications with C-Menu, it is essential to understand the design philosophy and optimizations that C-Menu provides and how to leverage them.

C-Menu was not meant to replace GUI or shell-based applications, but to compliment them by providing an alternative to users who demand the performance and efficiency possible with terminal-based interfaces. C-Menu's design philosophy is centered around simplicity, efficiency, and directness.

C-Menu provides a powerful set of internal functions that allow you to create complex and responsive applications while minimizing the need for external executables or shells. By using C-Menu's internal functions and direct execution capabilities, you can achieve performance levels that are orders of magnitude beyond than traditional shell and GUI based menu systems.

Hard-core, veteran programmers tend to prefer the black screen terminal over flashy GUIs for good reason. It's not just a matter of aesthetics, but a reflection of the underlying design philosophy and optimizations that terminal-based applications can provide. Terminal applications have a more direct and efficient interface to the underlying operating system, resulting in exceptional performance and greater control over resources.

Understandably, less experienced users prefer a GUI interface because a raw terminal interface, while powerful, can be terse and unforgiving. Becoming truly proficient with the terminal environment requires a significant investment of time and effort. In contrast, GUI interfaces are generally more intuitive and user-friendly, especially for those who are not familiar with the Unix command-line. A GUI provides visual cues and interactive elements that can make it easier for users to navigate and perform tasks without needing to remember complex commands or syntax.

C-Menu's design objective is to provide the best of both worlds by combining the speed and efficiency of terminal-based applications with an attractive, intuitive, and user-friendly framework. C-Menu's internal functions and direct execution capabilities allow you to create applications that are fast and responsive, while its menu system provides a more accessible, intuitive, and visually appealing interface.

With the C-Menu applications you create, intelligent users will discover a new sense of freedom, unbridled by slugish response times that wreck continuity of thought.

## C-Menu Launcher

When you use C-Menu's Example Application Menu, you will notice that most menu selections respond instantaneously with no perceptible delay. It just snaps. That level of optimization is achieved in part by avoiding the overhead and unpredictability of using a shell to execute command lines. C-Menu provides direct execution, which results in start-up times an order of magnitude faster than traditional shell-based menu systems. But, C-Menu has something even faster. You can take C-Menu performance to the next level by letting C-Menu call it's internal functions directly, bypassing the start-up overhead of external executables. Menu, Form, Pick, View, and CKeys are internal functions. In the Example Applications Menu, all except the first command line are internal function calls, which execute in nanoseconds compared to the milliseconds it takes to launch an external executable.

Here's a breakdown of the performance levels you can expect when using C-Menu:

Relative performance:

| Performance level | Description             | Elapsed Time |
| ----------------- | ----------------------- | ------------ |
| Somewhat Slugish  | Shell based execution   | 10-100 ms    |
| Fast              | Direct execution        | 1-10 ms      |
| Instantaneous     | Internal function calls | 0.001 ms     |

At 200 ms response, a user will perceive an application as sluggish. At 20 ms, the user will perceive a smooth and responsive experience. At 1 ms, the user will perceive an instantaneous response. Admittedly, no one really cares whether a program loads in 0.001 ms or 1,000 times slower at 1 ms, but in practical applications iterative processes often take thousands of cycles to complete.

Sometimes it is expedient to use a shell to execute a command line, such as when you want to use shell features like globbing, variable expansion, or complex command pipelines. In those cases, you can explicitly invoke a shell by including "sh -c" or a shell script on the command line.

When designing with C-Menu, you should select a method in the following order:

- C-Menu internals, such as Menu, Form, Pick, and View
- direct execution
- shell

If an internal function call will work, use an internal function call. If not, consider direct execution. Finally, if you need features provided by a shell, use a shell. There is nothing wrong with using a shell when it is appropriate, but you should be aware of the performance implications and use it judiciously.

C-Menu was designed to support internal function calls and direct execution, so it provides the most commonly needed conveniences such as tilde expansion, file location based on the environment, and piped input and output.

For piped input and output with internal function calls and direct execution, C-Menu allows the specification of providers and receivers for pipe I-O. Instead of using I/O redirection on the command line with pipe symbols, C-Menu provides more controllable alternatives such as "-S" for specifying a command to execute as a provider (source) of input to a form, pick, or view, "-R" for specifying a command to receive standard output from a form, pick, or view, and "-c" for specifying a command to execute with the selected item as an argument. These features allow you to create powerful and flexible menu items that can interact with other applications and scripts in a more controlled and efficient manner.

---
