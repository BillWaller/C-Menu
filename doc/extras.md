# Installing Programs to Augment C-Menu

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

## Additional Resources

For more information on Rust and its tools, you can visit the following resources:

- [The Rust Programming Language Book](https://doc.rust-lang.org/book/)

- [Cargo Documentation](https://doc.rust-lang.org/cargo/)

- [Bat GitHub Repository](https://github.com/sharkdp/bat)

- [Ripgrep GitHub Repository](https://github.com/BurntSushi/ripgrep)

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

## Conclusion

By following the instructions above, you can easily install Rust and augment your C-Menu experience with additional tools like Bat, Ripgrep, and lsd. Enjoy the enhanced functionality and features these tools bring to your workflow!
