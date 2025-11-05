# CMenu Application Suite - AI Coding Instructions

## Project Overview

CMenu is a terminal-based menu application suite written in C, providing interactive console interfaces for menus, forms, pickers, and file viewers. It's a classic Unix-style toolkit with ncurses-based windowing.

## Architecture

### Core Components (4 main applications)
- **menu**: Interactive menu system with configurable options and sub-menus
- **pick**: Multi-selection picker for files/objects with keyboard navigation
- **form**: Input form handler with field validation and formatting
- **view**: File viewer with search, navigation, and paging capabilities

### Shared Library Structure
All applications link against a common `cmenu` library containing:
- `dwin.c`: Window management and ncurses abstraction
- `scriou.c`: Terminal I/O control and signal handling
- `init.c`: Configuration parsing and initialization
- `*_engine.c`: Core logic for each application type

### Key Header: `menu.h`
Central header defining all structs, enums, and function prototypes. Critical structures:
- `Init`: Master configuration container
- `Menu`, `Form`, `Pick`, `View`: Application-specific contexts
- Window management arrays: `win_win[]`, `win_box[]`

## Build System

### Primary: Traditional Makefile
```bash
make all          # Build all applications
make menu         # Build specific application
make install      # Install to ~/menuapp/bin
```

### Secondary: CMake (experimental)
Basic CMake support available but Makefile is canonical.

## Development Patterns

### Configuration System
- Uses `~/.minitrc` for user configuration
- Geometry format: `lllcccLLLCCC` (lines, cols, begy, begx)
- Color system: 16-color palette with foreground/background/border

### Error Handling
- `abend()`: Immediate termination with error message
- `display_error_message()`: User-friendly error display
- Global `f_stop_on_error` flag controls error behavior

### Memory Management
- Manual allocation/deallocation patterns
- `strz_dup()`, `strnz_dup()`: String duplication utilities
- Careful cleanup in `close_*()` functions

### File Specification Pattern
```c
bool derive_file_spec(char *base, char *dir, char *file);
bool verify_file(char *path, int flags);
```
Consistent path resolution across all components.

## Critical Development Commands

### Build & Test Workflow
```bash
make clean && make all    # Full rebuild
./menu -d examples/       # Test with sample data
./inst.sh --             # Clear install tracking
```

### Debugging
- Compile with `-g3 -O0` (already default)
- Use `f_debug` global flag for verbose output
- `display_curses_keys()` for key code debugging

## Code Conventions

### Naming Patterns
- Functions: `snake_case` with module prefix (`win_new`, `menu_engine`)
- Structs: `PascalCase` (`Menu`, `Form`, `Pick`, `View`)
- Enums: `UPPER_CASE` with prefix (`MT_CHOICE`, `MA_DISPLAY_MENU`)
- Files: `lowercase.c` with descriptive names

### Window Management
Always pair window operations:
```c
win_new() -> win_del()
win_open_box() -> win_close_box()
```

### String Handling
Use project utilities over standard library:
- `strnz_cpy()` instead of `strncpy()`
- `MAXLEN` constant for buffer sizing
- Always null-terminate manually

## Dependencies

- **ncursesw**: Wide character support for Unicode box drawing
- **tinfo**: Terminal information library
- Standard C library with POSIX extensions

## Integration Points

### Terminal Control
`scriou.c` manages terminal state transitions between shell and curses modes.

### File System Integration
All applications support `~/menuapp/` directory structure for data, help, and configuration files.

### Command Execution
`exec.c` provides `fork_exec()` for launching external commands while preserving terminal state.

## Testing Locations

- `test/`: Unit tests and validation programs
- `work/`: Help files and example data
- `doc/examples/`: Sample configurations and bashrc integration