# C-Menu Extras

# Table of Contents

- [Introduction](#introduction)
- [Installing Rust](#installing-rust)
- [About C-Menu and R-Menu](#about-c-menu-and-r-menu)
- [Installing Bat](#installing-bat)
- [Installing Ripgrep](#installing-ripgrep)
- [lsd](#lsd)
- [Tree-Sitter](#tree-sitter)
- [Tree-Sitter-CLI](#tree-sitter-cli)
- [Tree-Sitter Parsers](#tree-sitter-parsers)
- [Rustlings](#rustlings)
- [Neovim](#neovim)
- [Lazyvim](#lazyvim)
- [Ghostty](#ghostty)
- [Useful Configurations](#useful-configurations)
  - [Alternate Bash Executable](#alternate-bash-executable)
  - [Shell Configuration](#shell-configuration)
  - [Customize Your Terminal Emulator](#customize-your-terminal-emulator)
    - [Fonts](#fonts)
    - [Example Terminal Configurations](#example-terminal-configurations)
      - [Ghostty Configuration](#ghostty-configuration)
      - [Kitty Configuration](#kitty-configuration)
      - [Alacritty Configuration](#alacritty-configuration)
- [Additional Resources](#additional-resources)

## Introduction

C-Menu is a powerful C-based suite of programs that you can use to create
a front-end infrastructure for applications. It includes Menu, Form, Pick,
View, lf (list files), C-Keys, and RSH. Howver, the C-Menu examples
rely on software that may not be included in all OS distributions.
You may very well already have some or all of these tools installed.

Because Rust's Cargo, is generally the easiest to use, and most
dependable, the installation methods described in this document use
Cargo wherever practical. You may prefer to use another package manager,
and most Distro Package Managers are exceptional. If so, please refer to
the documentation for that package manager for installation instructions.

This guide will walk you through the installation process for Rust and
some of the most useful tools that complement C-Menu.

Not all of these applications are written in Rust. Ghostty is written in
Zig and Neovim is written in C. I havent listed the terminal emulators,
Kitty or Alacritty, but I have used both and they are excellent terminal
emulators. I have all three installed on my desktop. I use Ghostty most
because it lives up to the ethos of Rust. It's solid, fast, and reliable.

The ability to span the two worlds of C and Rust is far more valuable than
being proficient in either language, and one computer language is never
enough.

By using C-Menu now, and better yet, contributing to the C-Menu project,
you can gain first-hand knowledge of the transition to Rust, shape the
future of the tool, and add software migration expert to your skill set.

## Installing Rust

C-Menu is written in C, but there are many amazing tools written in Rust that
can augment your experience using C-Menu. Even if you never plan to write code
in rust, the Cargo package manager that comes with Rust is worth the effort to
install Rust. You will notice immediately that Cargo is the easiest and most
reliable way to install Rust-based tools and their dependencies. Below are
instructions on how to install Rust and some of my favorite Rust-based tools.

So much for the preamble. Let's jump in.

### Installing Rust On Linux and macOS

Run the following in your terminal, then follow the onscreen instructions.

```bash
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

For more detailed instructions, visit

[the official Rust installation page](https://www.rust-lang.org/tools/install)

After installation, ensure that your environment is set up correctly by running:

```bash
    source $HOME/.cargo/env
```

You can verify the installation by checking the Rust version:

```bash
    rustc --version
```

This should display the installed Rust version, confirming that Rust is
successfully installed on your system.

You can update Rust and Cargo at any time by running:

```bash
    rustup update
```

---

## About C-Menu and R-Menu

Eventually, if there is enough interest in C-Menu, it will be ported to Rust.
The plan is to thoroughly test and debug the C version, and incrementally
incorporate Rust-like coding paradigms into the C codebase. Once the C version
is stable and feature complete, we begin the porting process, we will begin
refactoring C-Menu to use more modular code structures and rust-like paradigms
that will facilitate the eventual port to Rust.

Come along with us on this exciting journey. If you already know Rust, we
could use your help. If you don't know Rust, you can learn with us.
You can contribute to R-Menu's evolution, and get involved in the Rust
community. Add Rust to your CV or Resume.

And, then you can say, you were a part of the team that created R-Menu. 😀

---

## Installing Bat

Bat is a popular Rust-based tool that serves as a cat clone with syntax
highlighting. It's easier to install and accommodates a variety of file
types not addressed by Tree-Sitter. To install Bat, use the following
commands based on your operating system:

### Bat On Linux

After installing Rust, you can install Bat using Cargo, Rust's package manager:

```bash
    cargo install bat
```

### Bat On macOS

```bash
    brew install bat
```

### Bat On Windows

```powershell
    choco install bat
```

After installation, you can verify that Bat is installed correctly by running:

```bash
    bat --version

```

This should display the installed Bat version, confirming that Bat is successfully
installed on your system.

---

## Installing Ripgrep

Ripgrep is a modernized and powerful replacement for grep. It is amazingly fast
and incorporates many features not available in grep.

To install Ripgrep, you can use the following commands depending on your
operating system

### Ripgrep On Linux

After installing Rust, you can install Ripgrep using Cargo, Rust's package manager:

```bash
    cargo install ripgrep
```

### Ripgrep On macOS

```bash
    brew install ripgrep
```

### Ripgrep On Windows

```powershell
    choco install ripgrep
```

After installation, you can verify that Ripgrep is installed correctly by running:

```bash
    rg --version
```

This should display the installed Ripgrep version, confirming that Ripgrep
is successfully installed on your system.

---

## lsd

To install lsd, a modern replacement for 'ls' with colorful output and additional
features, you can use the following commands based on your operating system:

### lsd On Linux

After installing Rust, you can install lsd using Cargo, Rust's package manager:

```bash
    cargo install lsd
```

### lsd On macOS

```bash
    brew install lsd
```

### lsd On Windows

```powershell
    choco install lsd
```

After installation, you can verify that lsd is installed correctly by running:

```bash
    lsd --version
```

This should display the installed lsd version, confirming that lsd is successfully
installed on your system.

A word of caution here, "lsd" in a different context, is an acronym for a
powerful hallucinogenic drug. Don't tell your grandmother you use lsd without
explaining that it's "ls Deluxe", a file listing utility, and not the "LSD"
she remembers from her college days at UC Berkeley. 😎

---

## Tree-Sitter

To put it mildly, Tree-Sitter is a game-changer when it comes to parsing and
analyzing source code. It provides a robust and efficient way to build
incremental parsers for programming languages, enabling advanced features like
syntax highlighting, code folding, and more. Tree-Sitter is widely adopted in
various code editors and IDEs, making it an essential tool for developers who
work with code on a regular basis.

The syntax highlighting provided by Tree-Sitter is top-notch, offering
precise and context-aware highlighting that enhances code readability and
understanding. It can handle complex language constructs and edge cases that
traditional syntax highlighters often struggle with.

In a phrase, it'll knock your socks off! 😮

![Tree-Sitter](screenshots/Tree-sitter-small.png)

Tree-Sitter works with Neovim, C-Menu, and many other editors and pagers to
provide advanced syntax highlighting and code analysis features.

![Tree-Sitter in C-Menu View](screenshots/tree-sitter1.png)

To install Tree-Sitter, you can use the following commands depending on your
operating system.

### Tree-Sitter On Linux

After installing Rust, you can install Tree-Sitter using Cargo,
Rust's package manager:

```bash
`bash cargo install tree-sitter-cli
```

### Tree-Sitter On macOS

```bash
brew install tree-sitter
```

### Tree-Sitter On Windows

```powershell
choco install tree-sitter
```

After installation, you can verify that Tree-Sitter is installed correctly by running:

```bash
    tree-sitter --version
```

This should display the installed Tree-Sitter version, confirming that Tree-Sitter
is successfully installed on your system.

---

## Tree-Sitter-CLI

To install Tree-Sitter-CLI, a command-line interface for Tree-Sitter, you can
use the following commands based on your operating system:

### Tree-Sitter-CLI On Linux

After installing Rust, you can install Tree-Sitter-CLI using Cargo, Rust's
package manager

```bash
    cargo install tree-sitter-cli
```

### Tree-Sitter-CLI On macOS

```bash
    brew install tree-sitter-cli
```

### Tree-Sitter-CLI On Windows

```powershell
    choco install tree-sitter-cli
```

After installation, you can verify that Tree-Sitter-CLI is installed correctly by
running:

```bash
    tree-sitter --version
```

This should display the installed Tree-Sitter-CLI version, confirming that
Tree-Sitter-CLI is successfully installed on your system.

---

## Tree-Sitter Parsers

To install Tree-Sitter parsers for various programming languages, you can use the
following commands based on your operating system:

### Tree-Sitter Parsers On Linux

After installing Rust, you can install Tree-Sitter parsers using Cargo, Rust's
package manager. As always, I highly recommend installing only the parsers you
need. Here are some examples:

```bash
    cargo install tree-sitter-python
    cargo install tree-sitter-javascript
    cargo install tree-sitter-rust
    cargo install tree-sitter-go
    cargo install tree-sitter-cpp
```

I installed my tree-sitter parsers in the following directory structure:

```bash
~/.config/tree-sitter
  ├── config.json
  └── parsers
  ├── tree-sitter-awk
  ├── tree-sitter-bash
  ├── tree-sitter-cpp
  ├── tree-sitter-c
  ├── tree-sitter-go
  ├── tree-sitter-lua
  ├── tree-sitter-markdown
  ├── tree-sitter-rust
  └── tree-sitter-zig
```

The `config.json` file contains configuration settings for Tree-Sitter.
You will find a sample config.json file in the tree-sitter directory of thie
repository.

The `parsers` directory contains the installed Tree-Sitter parsers for
different programming languages.

To let Tree-Sitter know where to find your custom parsers, you need to
specify the parser directories in the `config.json` file. Tree-sitter will
automatically look in ~/.config/tree-sitter. Here's an example of how to
do this:

```bash
cd ~/.config/tree-sitter
vi config.json

"parser-directories": [
        /home/bill/.config/tree-sitter/parsers
]
```

You can also configure a custom color scheme in the config.json file if you like.
Here's mine:

![~/.config/tree-sitter/config.json](screenshots/tree-sitter-config.json.png)

The ~/tree-sitter/config.json distributed with tree-sitter uses the 256 color
xterm palette, but you can also use HTML style six digit hex numbers as shown
above. Refer to the Tree-Sitter documentation for more details on configuring
the color scheme.

Note: I find it much easier to use the six-digit hex color codes when configuring
Tree-Sitter colors. This way, I can easily match the colors used in my terminal
emulator and other applications. The 256 color xterm palette can be limiting
and may not provide the exact colors I want for syntax highlighting. Using
hex codes gives me more flexibility and allows for a more consistent color
scheme across my development environment. They are also more intuitive to
work with since they directly represent RGB values.

```bash
    cargo install tree-sitter-<language>
```

### Tree-Sitter Parsers On macOS

```bash
    brew install tree-sitter-<language>
```

### Tree-Sitter Parsers On Windows

```powershell
    choco install tree-sitter-<language>
```

After installation, you can verify that the Tree-Sitter parser for your chosen
language is installed correctly by running:

```bash
    tree-sitter <language> --version
```

This should display the installed Tree-Sitter parser version for your chosen
language, confirming that it is successfully installed on your system.
Replace `<language>` with the specific programming language you want to install
the parser for, such as `python`, `javascript`, `rust`, etc.

To put it bluntly, nothing compares with Tree-Sitter for syntax highlighting
and code analysis. Once you start using it, you'll wonder how you ever lived
without it.

![Tree-Sitter in C-Menu View](screenshots/tree-sitter4.png)

---

## Rustlings

Even if you are an experienced programmer, Rust is not an easy language
to learn. The ownership model and borrow checker are unique concepts that
require a different way of thinking about memory management and data lifetimes.
If you are new to programming, Rust can be even more challenging because it
requires a solid understanding of programming fundamentals.

However, with dedication and practice, anyone can learn Rust. There are many
great resources available online, including the official Rust documentation,
tutorials, and community forums. Additionally, there are many books and courses
that can help you learn Rust from scratch.

Start with the Rust Book, and concurrently do the exercises in Rustlings.
Rustlings is an invaluable companion to the Rust Book.

[The Rust Book](https://doc.rust-lang.org/book/)

![Rustlings](screenshots/rustlings1.png)

![Rustlings](screenshots/rustlings2.png)

## Neovim

And last, but certainly not least in my list of essential tools is Neovim. I
loved vim and used it for many years. I can't say whether Vim or Neovim is
better because both have evolved to provide competitive features. I can say,
for me, Neovim plugins seem more accessible, and it has everything I want and
more.

![Neovim](screenshots/Neovim.png)

## Lazyvim

Lazyvim is a Neovim configuration that is designed to be fast, minimal, and
easy to use. It comes with a set of pre-configured plugins and settings that
enhance the Neovim experience. Lazyvim is highly customizable, allowing users
to tailor their Neovim setup to their specific needs.

![Lazyvim](screenshots/Lazyvim.png)

To install Neovim and Lazyvim, follow the instructions on the
[Lazyvim website](https://www.lazyvim.org/).

## Ghostty

Ghostty is a terminal emulator that is designed to be fast, lightweight, and
highly customizable. It is built using Rust and leverages modern technologies
to provide a smooth and efficient terminal experience. Ghostty supports
features such as GPU acceleration, ligatures, and a wide range of customization
options.

To install Ghostty, you can use the following commands based on your operating
system:

### Ghostty On Linux

![Ghostty With Neovim and Lazyvim](screenshots/Ghostty.png)

![Ghostty With Neovim and Lazyvim](screenshots/Neovim-v.png)

After installing Rust, you can install Ghostty using Cargo, Rust's package
manager:

What you see above may not be useful for development projects, but it
demonstrates Neovim, Ghostty, and Lazyvim working together in harmony to
provide great flexibility.

This feature is actually quite useful when searching for a particular source
file among many files in a large project. You can quickly preview files
without opening them fully in Neovim.

```bash
    cargo install ghostty
```

### Ghostty On macOS

```bash
    brew install ghostty
```

### Ghostty On Windows

```powershell
    choco install ghostty
```

After installation, you can verify that Ghostty is installed correctly by running:

```bash
    ghostty --version
```

---

## Useful Configurations

This document describes some useful configurations for C-Menu and terminal
emulators. These configurations are optional but recommended for a better
user experience.

### Alternate Bash Executable

If you have had problems with the bash executable distributed with your OS,
you may want to download the bash source distribution and build your own
bash executable. There are many reasons why you might want to do this,
including security concerns, bugs in the bash version provided by your OS,
or the need for a specific feature that is not available in your OS's bash
version.

After building your own bash executable, you should consider renaming it to
something other than "bash". For example, you might name it "jcalc", "mmdb", or
"xmlock", something that will blend in with a ps listing. This is especially
important if you are using a system where the default bash executable is known
to have security vulnerabilities or other issues.

There are horror stories about systems that have been compromised because
the bash executable was exploited. In one instance, an OS update overwrote
the bash executable with one that would only start in restricted mode. It
wouldn't even allow running executables from /bin and /sbin. By using a
custom-named bash executable, you can reduce the risk of such exploits
and mistakes. You can leave the original bash executable intact as a backup.

One more caveat with certain configurations of bash is that you may
not be able to setuid on rsh from a root shell created by su. This
depends on the distribution and configuration of your system. In
that case, you can probably use su to start an interactive root
shell, and manually install rsh setuid. From the cmenu src directory:

```bash

$ su
Password:

cp rsh ~/menuapp/bin
chmod 4755 ~/menuapp/bin/rsh

ls -l ~/menuapp/bin/rsh

-rwsr-xr-x 1 root root 123456 Jan 01 12:

```

### Shell Configuration

The following shell commands are sourced by xsh, bash, and sh.

This snippet, stolen from bashfaq #24 prepends directories to PATH if the
directories exist and aren't already in PATH. It also defines two convenience
functions, xx and mm, to start a root shell and the menuapp, respectively. It
sets the default shell to xsh if available, otherwise bash.

It sets a shorter
delay for curses escape sequences

and configures a colorful prompt with red for root and green for normal users.

- prepend_path makes it easy to prepend directories to PATH without producing
  duplicate entries. Use it to prepend ~/menuapp/bin or other directories to PATH.

```bash

prepend_path() {
case ":$PATH:" in
        *:"$1":*) ;;
        *) PATH="$1:$PATH" ;;
esac
}

[ -d "$HOME/.cargo/bin" ] && prepend_path "$HOME/.cargo/bin"
[ -d "$HOME/.local/bin" ] && prepend_path "$HOME/.local/bin"
[ -d "$HOME/go/bin" ] && prepend_path "$HOME/go/bin"
[ -d "$HOME/menuapp/bin" ] && prepend_path "$HOME/menuapp/bin"

```

- from a shell prompt, type xx to start a root shell

```bash

which rsh >/dev/null 2>&1 && xx() { rsh; }

```

- from a shell prompt, type mm to start menuapp

```bash

which menu >/dev/null 2>&1 && mm() { menu; }

```

- Prefer xsh over bash

```bash

export SHELL=bash
which xsh >/dev/null 2>&1 && export SHELL=xsh

```

- set a shorter delay for curses escape sequences

```bash

export ESCDELAY=50

```

- set colorful prompt, red for root, green for normal user

```bash
export XUSER="$(id -un)"
export PS1="\[\e[1;32m\]\u@\h:\w>\[\e[0m\] "
[ "$XUSER" = "root" ] && export PS1="\[\e[1;31m\]\u@\h:\w>\[\e[0m\] "
echo PS1="$PS1"

```

### Customize Your Terminal Emulator

The configurations herein are not required to run the C-Menu system, but
provided as an example of how to customize the terminal emulator to your
liking. You may want to modify the font, font size, window dimensions,
colors, and other settings to suit your preferences.

#### Fonts

If you don't already have a patched font, you may want to install
JetBrains Mono. It is a free, open-source, monospaced font designed
for developers. It includes a large number of programming ligatures and
is optimized for readability on screens of all sizes.

The JetBrains Mono font is available from:

[JetBrains](https://www.jetbrains.com/lp/mono/)

Also check out Nerd Fonts for other patched fonts that include programming
ligatures and icons.

[NerdFonts](https://www.nerdfonts.com/)

#### Example Terminal Configurations

The standard color palette is rather drab, so you may want to include a more
colorful palette such as the high contrast palette below.

This configuration was designed for a dark terminal background and a
resolution of 3840x2160 (4k). You may want to adjust the font size and window
dimensions for your own display. The window dimensions are specified in
character cells. For example, a window-width of 95 means 95 character cells
wide. The actual pixel width of the window will depend on the font size and the
font used.

##### Ghostty Configuration

![Ghostty Configuration](screenshots/ghostty.config.png)

##### Kitty Configuration

![Kitty Configuration](screenshots/kitty.conf.png)

##### Alacritty Configuration

![Alacritty Configuration](screenshots/alacritty.toml.png)

---

## Additional Resources

For more information on Rust and its tools, you can visit the following resources:

[The Rust Programming Language Book](https://doc.rust-lang.org/book/)

[Rustlings Exercises](https://rustlings.rust-lang.org/)

[Cargo Documentation](https://doc.rust-lang.org/cargo/)

[Bat GitHub Repository](https://github.com/sharkdp/bat)

[Bat Official Website](https://bat.dev/)

[Ripgrep GitHub Repository](https://github.com/BurntSushi/ripgrep)

[Tree-Sitter GitHub Repository](tree-sitter.github.io/tree-sitter)

[Neovim Official Website](https://neovim.io/)

[Lazyvim](https://www.lazyvim.org/)

## Conclusion

By following the above instructions, you can easily install Rust and various
Rust-based tools to enhance your C-Menu experience. Enjoy coding with these
powerful tools!
