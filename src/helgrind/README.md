# Format Helgrind Output

## Why Nobody Writes a Helgrind Output Formatter

Valgrind, Helgrind, and most other tools produce free-format text output. This is arguably great for humans, but it makes it hard to parse the output programmatically. The problem is that the output is not structured, and it can vary depending on the version of the tool, the platform, and even the specific code being analyzed. This makes it difficult to write a reliable parser that can handle all possible cases. This program will not attempt to solve this problem, but will provide a simple formatter that works with the now current version of Helgrind. It is not guaranteed to work with future versions, and it may not handle all edge cases. However, it should be sufficient for most use cases. For those cases where it does not work, the user is encouraged to modify the code herein to suit their needs, and provide feedback and patches to the author so that other users can benefit from the improvements. As always, your contributions to the code are welcome.

The author is not responsible for any damage caused by the use of this program.
It is up to the user to verify that the output is correct and to use it at their own risk.
