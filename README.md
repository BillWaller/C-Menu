![C-Menu Announcement](screenshots/Announcement.png)

*2026-06-19T23:40:50-05:00* - REFACTOR: The goal of this refactor is to decouple the core logic from the UI library, and to create a uniform abstraction layer user interface (UALUI) to support multiple backends. The steps will be as follows:

1. Create a UALUI that defines UI operations, such as drawing, input handling, and event processing.
2. Isolate library dependent UI code behind the UALUI.
3. Refactor the core logic, replacing NCurses specific code with UALUI code.
4. Test the refactor to ensure that there are no regressions or breakages in existing functionality.
5. Add support for new backends, such as Notcurses and a modern graphics library.

The refactoring will proceed incrementally, maintaining both the existing NCurses interface and the UALUI simultaneously. This will ensure that the refactor does not introduce any regressions or break any existing functionality, while also allowing us to test the new UALUI code and the new backends as they are developed.

This is an exciting time for the project, and I look forward to seeing the improvements that will come from this refactor. Thank you to everyone who has contributed to the project so far, and I encourage everyone to continue contributing and providing feedback as we move forward.

Note: A substantial portion of the UALUI framework is already in place under
C-Menu/src/ui.  

[C-Menu Doxygen](https://decision-inc.com) | [C-Menu CHANGELOG](docs/CHANGELOG.md) | [C-Menu Installation](docs/INSTALL.md)

![C-Menu Introduction](screenshots/C-Menu-Introduction.png)


C-Menu is a fast, modular toolkit for building terminal-based user interfaces on Linux. Its components can be combined to create responsive menus, forms, pick lists, viewers, file-finding workflows, and administrative tools without the overhead of a heavyweight GUI stack.

Written in C and designed for speed, C-Menu works well for developer tools, system administration workflows, kiosk-style interfaces, and resource-constrained environments.

## Why C-Menu?

C-Menu is built for users who want terminal applications that are:

- **Fast** - optimized C programs with a minimal footprint
- **Modular** - combine small purpose-built components into larger workflows
- **Script-friendly** - easy to integrate with shell scripts and external commands
- **Interactive** - supports keyboard and mouse driven terminal interfaces
- **Practical** - useful for both end-user tools and developer/admin utilities

Instead of treating menus, forms, selection lists, and viewers as separate one-off programs, C-Menu treats them as reusable building blocks that can be assembled into complete applications.

## Components

| Component | Purpose                                            | Typical Use                                            |
| --------- | -------------------------------------------------- | ------------------------------------------------------ |
| `menu`    | Display menus and launch actions                   | Application navigation, submenus, command dispatch     |
| `form`    | Enter, edit, validate, and process structured data | Data entry, calculations, query/update workflows       |
| `pick`    | Select one or more items from a list               | File selection, multi-select actions, command dispatch |
| `view`    | Display text and command output efficiently        | Logs, reports, source code, highlighted output         |
| `lf`      | Regular-expression-based file finder               | File discovery, filtering, pipelines                   |
| `rsh`     | Privileged shell helper                            | Controlled administrative workflows                    |
| `C-Keys`  | Keyboard and mouse diagnostic utility              | Input testing and terminal diagnostics                 |

## Quick Start

See [BUILD](docs/BUILD.md) for full build and install instructions.

A typical workflow with C-Menu looks like this:

1. Write a small menu description file
2. Launch `menu`
3. Dispatch actions to `form`, `pick`, `view`, shell commands, or other C-Menu components

Minimal example:

```text
: APPLICATIONS
:     Edit Current Project C Files
!pick -S "lf -d 5 '.*\.c$'" -T "Project Source" -c nvim %%
:     Open Root Shell
!exec rsh
```

This example shows one of C-Menu’s core ideas: the menu itself is simple, and the power comes from composing components and external commands.

## Core Concepts

C-Menu applications are usually built from plain text description files and command lines.

- **Menu** description files define choices and the commands that run when a choice is selected.
- **Form** description files define labels, input fields, and directives for processing.
- **Pick** reads items from a file or command and lets the user select one or more of them.
- **View** displays files or command output in a fast read-only interface.
- **lf** can generate file lists that feed directly into `pick`, shell pipelines, or custom scripts.

This makes C-Menu especially effective when you want to combine:

- shell commands
- external utilities
- custom scripts
- terminal UI components
- low-latency workflows

## Examples

### Menu

`menu` is the top-level dispatcher. It displays choices and runs the corresponding commands.

Example:

```bash
: APPLICATIONS
:     Full Screen Shell
!exec rsh

:     Workstation Configuration
!menu workstation_config.m

:     Diagnostic Tools
!menu diag.m
```

Use `menu` when you want:

- a text-based launcher
- a hierarchy of submenus
- a consistent front end for scripts and utilities
- a terminal-native application shell

### Form

`form` is used for entering, editing, validating, and processing structured data.

A form description file contains text lines and field definitions.

Text example:

```bash
T:5:14:Principal Amount
```

Field example:

```bash
F:5:33:14:Currency
```

Common workflow:

```bash
:     Installment Loan Calculations
!form iloan.f -i iloan.dat -S iloan -R "view -S \"amort %%\"" -o iloan.dat
```

This demonstrates a powerful pattern:

- load initial values
- process them with an external executable
- redisplay updated values
- optionally hand off results to `view`

Use `form` when you need:

- data entry
- calculations
- validation workflows
- query/update cycles
- integration with external programs

### Pick

`pick` displays a list of items and lets the user select one or more of them.

General form:

```bash
!pick [ -n maximum_number_of_selections ][ -m ] \
    [ -i input_file ][ -S executable_provider ] \
    [ -o output_file ][ -c executable %% ]
```

Example:

```bash
:     Rustlings Source
!pick -S rust_src -n 1 -T "Rustlings Source - Edit" -c nvim %%
```

You can also avoid helper scripts when a direct command is sufficient:

```bash
: Edit .c Files in Current Directory
!pick -S "lf -d 5 '.*\.c$'" -T "Project Tree - Select File to Edit" -c nvim %%
```

Use `pick` when you need:

- interactive selection from generated lists
- real-time filtering
- single- or multi-select workflows
- dispatching selected items to external commands

### View

`view` is a fast read-only viewer for files and command output.

It supports:

- Unicode
- line numbering
- regular-expression searching
- horizontal scrolling with a large virtual pad
- highlighted output from tools such as Tree-sitter, `bat`, `pygments`, and `source-highlight`

Example:

```bash
: View C-Menu Source with Tree-Sitter
!pick -S project_src -n 1 -T "Select Project Source to Highlight" -c "view -L 60 -C 85 -S \"tree-sitter highlight %%\""
```

`view` is especially useful for:

- source browsing
- log inspection
- highlighted output
- large text files
- command output that should remain read-only

### lf

`lf` is C-Menu’s regular expression file finder. The name can be read as either **list files** or **lightweight find**.

Example:

```bash
lf -d 5 -t f -a 2024-01-01 -b 2024-06-01 | xargs ls -l
```

This makes it easy to generate file sets for:

- `pick`
- shell pipelines
- batch processing
- date-based searches
- source filtering

Example integration with `pick`:

```bash
!pick -S "lf -d 5 '.*\.c$'" -T "Project Tree - Select File to Edit" -c nvim %%
```

For performance notes and additional details, see [PERFORMANCE](docs/Performance.md).

### rsh

`rsh` provides a root-shell-oriented administrative workflow intended as an alternative approach to repeatedly invoking `su` or `sudo` for certain tasks.

Example use case:

- elevate for an administrative operation
- run the required command
- exit immediately when finished

This can be useful in tightly controlled environments where short-lived privileged sessions are part of an established workflow.

## Documentation

Additional documentation is available in the `docs` directory:

- [API](docs/API.md) - developer-facing API documentation
- [CHANGELOG](docs/CHANGELOG.md) - project history and notable changes
- [USER GUIDE](docs/C-Menu-UG.md) - end-user documentation
- [AUGMENTATION](docs/extras.md) - additional examples and supporting material
- [FAQ](docs/FAQ.md) - frequently asked questions
- [INSTALLATION](docs/INSTALL.md) - build and installation instructions
- [PERFORMANCE](docs/Performance.md) - benchmarks and performance notes
- [VALGRIND](docs/valgrind.md) - memory-checking notes
- [HTML Documentation](https://decision-inc.com) - published documentation site

## Configuration

Many C-Menu options can be set either:

- on the command line, or
- in the configuration file `~/.minitrc`

Command-line options override configuration-file settings.

Examples:

```bash
# .minitrc
fill_character=_
brackets=[]
```

Configuration can be used to control display behavior and tailor the interface to your workflow.

## Platform and Requirements

C-Menu is designed for Linux and terminal-based operation.

Typical requirements:

- Linux
- a standard C library
- a terminal with the capabilities needed for the desired interface features

Optional integrations may depend on additional external tools such as:

- `nvim`
- `bat`
- Tree-sitter tools
- `source-highlight`
- `pygments`

Refer to [INSTALLATION](docs/INSTALL.md) for the exact build and runtime details.

## Security Notes

Some C-Menu components can be used in privileged or security-sensitive workflows, especially `rsh`.

Before using `rsh` or integrating C-Menu into elevated workflows, make sure you clearly define:

- who is allowed to authenticate
- how authentication is performed
- what level of privilege is granted
- what auditing or logging is required
- whether the environment is appropriate for passwordless or key-based elevation

Administrative tooling should be reviewed carefully before use in production or multi-user environments.

## Why the Design Works

C-Menu’s design is effective because it keeps the UI layer simple and composable.

Rather than forcing a single monolithic framework, it lets you connect:

- plain text configuration
- terminal-native interaction
- Unix pipelines
- external executables
- lightweight focused tools

That makes it a strong fit for:

- custom internal tools
- developer utilities
- interactive scripts
- operations workflows
- systems with limited resources

## Contributing

If you are exploring C-Menu for the first time, a good path is:

1. Read [BUILD](docs/BUILD.md)
2. Read the [USER GUIDE](docs/C-Menu-UG.md)
3. Try a small `menu` + `pick` workflow
4. Expand into `form`, `view`, and `lf` integrations

If you contribute examples, documentation improvements, or fixes, keeping examples small and practical will help new users learn the system quickly.

## See also

- [Build](docs/BUILD.md) - build instructions and requirements
- [API](docs/API.md) - developer-facing API documentation
- [CHANGELOG](docs/CHANGELOG.md) - project history and notable changes
- [User Guide](docs/C-Menu-UG.md) - end-user documentation
- [Augmentation](docs/extras.md) - additional examples and supporting material
- [FAQ](docs/FAQ.md) - frequently asked questions
- [Performance](docs/Performance.md) - benchmarks and performance notes
- [Menu](docs/menu.md) - detailed documentation for the `menu` component
- [Form](docs/form.md) - detailed documentation for the `form` component
- [Pick](docs/pick.md) - detailed documentation for the `pick` component
- [View](docs/view.md) - detailed documentation for the `view` component
- [Exercises](docs/exercises.md) - exercises to practice using C-Menu components
- [Valgrind](docs/valgrind.md) - memory-checking notes and best practices
- [Roadmap](docs/ROADMAP.md) - planned features and future directions'
- [Contributing](docs/CONTRIBUTING.md) - guidelines for contributing to the project
