# Installing Programs to Augment C-Menu

## Table of Contents

1. [Installing Rust](#installing-rust)
2. [About C-Menu and R-Menu](#about-c-menu-and-r-menu)
3. [Installing Bat](#installing-bat)
4. [Installing Ripgrep](#installing-ripgrep)
5. [lsd](#lsd)
6. [Tree-Sitter](#tree-sitter)
7. [Tree-Sitter-CLI](#tree-sitter-cli)
8. [Tree-Sitter Parsers](#tree-sitter-parsers)
9. [Yazi](#yazi)
10. [Rustlings](#rustlings)
11. [Neovim](#neovim)

## Installing Rust

C-Menu is written in C, but there are many amazing tools written in Rust that can augment your experience using C-Menu. Below are instructions on how to install Rust and some of my favorite Rust-based tools.

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

This should display the installed Rust version, confirming that Rust is successfully installed on your system.

Rust comes with Cargo, the Rust package manager, which you can use to install additional Rust-based tools.

You can update Rust and Cargo at any time by running:

```bash
    rustup update
```

## About C-Menu and R-Menu

Eventually, if there is enough interest in C-Menu, it will be ported to Rust as well! And, they it will be called R-Menu. :)

## Installing Bat

To install Bat, a cat clone with syntax highlighting and Git integration, you can use the following commands based on your operating system:

### On Linux

After installing Rust, you can install Bat using Cargo, Rust's package manager:

```bash
    cargo install bat
```

### On macOS

```bash
    brew install bat
```

### On Windows

```powershell
    choco install bat
```

After installation, you can verify that Bat is installed correctly by running:

```bash
    bat --version

```

This should display the installed Bat version, confirming that Bat is successfully installed on your system.

## Installing Ripgrep

To install Ripgrep, a line-oriented search tool that recursively searches your current directory for a regex pattern, you can use the following commands based on your operating system:

### On Linux

After installing Rust, you can install Ripgrep using Cargo, Rust's package manager:

```bash
    cargo install ripgrep
```

### On macOS

```bash
    brew install ripgrep
```

### On Windows

```powershell
    choco install ripgrep
```

After installation, you can verify that Ripgrep is installed correctly by running:

```bash
    rg --version
```

This should display the installed Ripgrep version, confirming that Ripgrep is successfully installed on your system.

## lsd

To install lsd, a modern replacement for 'ls' with colorful output and additional features, you can use the following commands based on your operating system:

### On Linux

After installing Rust, you can install lsd using Cargo, Rust's package manager:

```bash
    cargo install lsd
```

### On macOS

```bash
    brew install lsd
```

### On Windows

```powershell
    choco install lsd
```

After installation, you can verify that lsd is installed correctly by running:

```bash
    lsd --version
```

This should display the installed lsd version, confirming that lsd is successfully installed on your system

## Tree-Sitter

To install Tree-Sitter, a parser generator tool and an incremental parsing library, you can use the following commands based on your operating system:

### On Linux

After installing Rust, you can install Tree-Sitter using Cargo, Rust's package manager:

```bash
    cargo install tree-sitter-cli
```

### On macOS

```bash
    brew install tree-sitter
```

### On Windows

```powershell
    choco install tree-sitter
```

After installation, you can verify that Tree-Sitter is installed correctly by running:

```bash
    tree-sitter --version
```

This should display the installed Tree-Sitter version, confirming that Tree-Sitter is successfully installed on your system.

## Tree-Sitter-CLI

To install Tree-Sitter-CLI, a command-line interface for Tree-Sitter, you can use the following commands based on your operating system:

### On Linux

After installing Rust, you can install Tree-Sitter-CLI using Cargo, Rust's package manager:

```bash
    cargo install tree-sitter-cli
```

### On macOS

```bash
    brew install tree-sitter-cli
```

### On Windows

```powershell
    choco install tree-sitter-cli
```

After installation, you can verify that Tree-Sitter-CLI is installed correctly by running:

```bash
    tree-sitter --version
```

This should display the installed Tree-Sitter-CLI version, confirming that Tree-Sitter-CLI is successfully installed on your system.

## Tree-Sitter Parsers

To install Tree-Sitter parsers for various programming languages, you can use the following commands based on your operating system:

### On Linux

After installing Rust, you can install Tree-Sitter parsers using Cargo, Rust's package manager:

```bash
    cargo install tree-sitter-<language>
```

### On macOS

```bash
    brew install tree-sitter-<language>
```

### On Windows

```powershell
    choco install tree-sitter-<language>
```

After installation, you can verify that the Tree-Sitter parser for your chosen language is installed correctly by running:

```bash
    tree-sitter <language> --version
```

This should display the installed Tree-Sitter parser version for your chosen language, confirming that it is successfully installed on your system.
Replace `<language>` with the specific programming language you want to install the parser for, such as `python`, `javascript`, `rust`, etc.

## Yazi

Yazi is a simple and elegant terminal-based markdown viewer built with Rust. It provides a clean and distraction-free way to read markdown files directly in your terminal.
To install Yazi, you can use the following commands based on your operating system:

### On Linux

After installing Rust, you can install Yazi using Cargo, Rust's package manager:

```bash
    cargo install yazi
```

### On macOS

```bash
    brew install yazi
```

### On Windows

```powershell
    choco install yazi
```

After installation, you can verify that Yazi is installed correctly by running:

```bash
    yazi --version
```

This should display the installed Yazi version, confirming that Yazi is successfully installed on your system

## Rustlings

Another tool I have really enjoyed is Rustlings. The Rustlings tutorials are an invaluable companion to the Rust Book.

[The Rust Book](https://doc.rust-lang.org/book/)

<img src="screenshots/rustlings1.png" title="Rustlings 1" />

<img src="screenshots/rustlings2.png" title="Rustlings 2" />

## Neovim

And last, but certainly not least in my list of essential tools is Neovim. I loved vim and used it for many years. I can't say whether Vim or Neovim is better because both have evolved to provide competitive features. I can say, for me, Neovim plugins seem more accessible, and it has everything I want and more.

<img src="screenshots/Neovim.png" title="Neovim" />

## Additional Resources

For more information on Rust and its tools, you can visit the following resources:

- [The Rust Programming Language Book](https://doc.rust-lang.org/book/)

- [Rustlings Exercises](https://rustlings.rust-lang.org/)

- [Cargo Documentation](https://doc.rust-lang.org/cargo/)

- [Bat GitHub Repository](https://github.com/sharkdp/bat)

- [Ripgrep GitHub Repository](https://github.com/BurntSushi/ripgrep)

## Conclusion

By following the above instructions, you can easily install Rust and various Rust-based tools to enhance your C-Menu experience. Enjoy coding with these powerful tools!
