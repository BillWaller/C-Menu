# Useful Configurations

This document describes some useful configurations for the menu
system and terminal emulator. These configurations are optional
but recommended for a better user experience.

## Alternate Bash Executable

If you have had problems with the bash executable distributed with
your OS, you may want to download the bash source distribution and
build your own bash executable. There are many reasons why you
might want to do this, including security concerns, bugs in the
bash version provided by your OS, or the need for a specific
feature that is not available in your OS's bash version.

After building your own bash executable, you should consider
renaming it to something other than "bash". For example, you might
name it "mybash", "custombash", or "xsh". This is especially
important if you are using a system where the default bash
executable is known to have security vulnerabilities or other
issues.

By renaming your bash executable, you can leave the original bash
executable intact as a backup. Using a unique name for your custom
bash executable has the additional benefit of skirting interference
from system updates, AppArmor, and other programs that target bash
by name.

One more caveat with bash is that you may not be able to install rsh
setuid with make and instexe from a root shell created by su. It's
not that you can't install rsh setuid, it's that you can't setuid
from a shell. So, use su to start an interactive root shell, and
manually install rsh setuid. From the cmenu src directory:

```bash

$ su
Password:

cp rsh ~/menuapp/bin
chmod 4755 ~/menuapp/bin/rsh

ls -l ~/menuapp/bin/rsh

-rwsr-xr-x 1 root root 123456 Jan 01 12:

```

## Shell Configuration

The following shell commands are sourced by xsh, bash, and sh.

This snippet prepends directories to PATH if the directories exist
and aren't already in PATH. It also defines two convenience
functions, xx and mm, to start a root shell and the menuapp,
respectively. It sets the default shell to xsh if available,
otherwise bash. It sets a shorter delay for curses escape sequences
and configures a colorful prompt with red for root and green for
normal users.

- prepend_path makes it easy to prepend directories to PATH. Use it
  to prepend ~/menuapp/bin or other directories to PATH.

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

## Customize Your Terminal Emulator

The configurations herein are not required to run the menu system
They are provided as an example of how to customize the terminal
emulator to your liking. You may want to modify the font, font
size, window dimensions, colors, and other settings to suit your
preferences.

### NerdFonts

If you don't already have a NerdFont, you may want to
install the JetBrains Mono NerdFont. It is a free, open-source,
monospaced font designed for developers. It includes a large
number of programming ligatures and is optimized for readability
on screens of all sizes.

The JetBrains Mono font is available from:

[JetBrains](https://www.jetbrains.com/lp/mono/)

[NerdFonts](https://www.nerdfonts.com/)

### Example Configurations

The standard color palette is rather drab, so you may want to
include a more colorful palette such as the high contrast
palette below.

This configuration was designed for a dark terminal background
and a resolution of 3840x2160 (4k). You may want to adjust
the font size and window dimensions for your own display.
The window dimensions are specified in character cells.
For example, a window-width of 95 means 95 character cells
wide. The actual pixel width of the window will depend on
the font size and the font used.

- Ghostty

```
# ~/.config/ghostty/config
command = xsh -i
title = xsh
font-family = "JetBrainsMono NFM Medium"
font-family-bold = "JetBrainsMono NFM ExtraBold"
font-family-italic = " JetBrainsMono NFM Italic"
font-family-bold-italic = "JetBrainsMono NFM Bold Italic"
font-size = 15
window-width = 95
window-height = 50
background-opacity = 0.99
window-decoration = server
# High contrast colors
palette=0=#000000
palette=1=#ff3f3f
palette=2=#4ff07f
palette=3=#ffef4f
palette=4=#5fafff
palette=5=#f077f0
palette=6=#8fdfff
palette=7=#ff8f5f
palette=8=#bfbfbf
palette=9=#ff7f00
palette=10=#00ffa0
palette=11=#ffcf00
palette=12=#005fff
palette=13=#ff00ff
palette=14=#00ffff
palette=15=#e0d0d0
background = #000720
foreground = #e0d0d0
cursor-color = #f0f0f0
selection-background = #e0d0d0
selection-foreground = #000000
```

- Kitty

```

# ~/.config/kitty/kitty.conf
shell xsh
# include ~/.config/kitty/fonts/default_font
font_family      JetBrainsMono NFM Medium
bold_font        JetBrainsMono NFM ExtraBold
italic_font      JetBrainsMono NFM Italic
bold_italic_font JetBrainsMono NFM Bold Italic
font_size 15
remember_window_size  no
initial_window_width    95c
initial_window_height   40c
# include ~/.config/kitty/themes/default_theme
# High contrast color scheme
url_color               #a0e0ff
cursor                  #ffffff
cursor_text_color       #000000
# tabs
active_tab_background   #001e1e
active_tab_foreground   #afd0ff
inactive_tab_background #2030a0
inactive_tab_foreground #c0c0c0
# tab_bar_background     #313131
# windows
active_border_color     #79a8ff
inactive_border_color   #646464
color0        #000000
color1        #ff3f3f
color2        #4ff07f
color3        #ffef4f
color4        #5fafff
color5        #f077f0
color6        #8fdfff
color7        #ff8f5f
color8        #bfbfbf
color9        #ff7f00
color10        #00ffa0
color11        #ffcf00
color12        #005fff
color13        #ff00ff
color14        #00ffff
color15        #e0d0d0
# extended colors
background              #000720
foreground              #e0d0d0
cursor                  #f0f0f0
selection_background    #e0d0d0
selection_foreground    #000000

```

- Alacritty

```
# ~/.config/alacritty/alacritty.toml

[general]

import = [

#   "~/.config/alacritty/fonts/default_font.toml",

[font]
normal = { family = "JetBrainsMono NFM", style = "Regular" }
bold = { family = "JetBrainsMono NFM", style = "ExtraBold" }
italic = { family = "JetBrainsMono NFM", style = "Italic" }
bold_italic = { family = "JetBrainsMono NFM", style = "Bold Italic" }

# "~/.config/alacritty/themes/default_theme.toml",

[colors.primary]
background = '#000000'
foreground = '#c0c0c0'

[colors.normal]
black = '#000000'
red = '#ff0000'
green = '#00ff8f'
yellow = '#ffc700'
blue = '#009fff'
magenta = '#e070e0'
cyan = '#00cfdf'
white = '#ff7f00'
[colors.bright]

]

black = '#7f7f7f'
red = '#ff7f00'
green = '#00ffa0'
yellow = '#ffef00'
blue = '#0000ff'
magenta = '#ff00ff'
cyan = '#00ffff'
white = '#ffffff'

[font]
size = 15

# dynamic_title = true
# decorations = "Full"
# decorations_theme_variant = "Dark"

[window]
dimensions = { columns = 95, lines = 40 }

[scrolling]
history = 10000
```
