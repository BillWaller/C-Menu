# C-Menu-0.2.5.2

### 2025-11-30T13:26:32-06:00

### View

- View - Integrated NCurses pad functionality into View - Currently, the pad is
  set to 1024 bytes wide, which should be sufficient for most use cases. It's defined
  in a menu.h. The primary benefit of using pads is better handling of large content
  such as system logs.

- View - Updated regular expression search facility to hibghlight matches on each
  line, even those which extend beyond the visible screen area.

- View - Improved handling of very long lines by implementing horizontal scrolling
  using the arrow keys and ALT arrow keys.

### Form

- Form - Added a framework and code for several numeric types including decimal
  integer, hexadecimal, double, float, and currency. The framework is flexible
  enough to allow for easy addition of new numeric types in the future.

### CMake Build

- CMake Build for CMenu - Successfully tested CMake build, using the helper scripts,
  build.sh, install.sh, and clean.sh, in the build directory. It's not finished,
  but it will produce working binaries and properly install them. What I couldn't accomplish within CMake (because of my incomplete knowledge of CMake) was handled with the helper scripts. One example is setting the ownership of installed files
  before applying setuid permissions. I also use scripts to augment Makefile installs,
  and it has always worked great, but I had rather do as much as possible with CMake.
  See the CMakeLists.txt file in the source directory for more details.
