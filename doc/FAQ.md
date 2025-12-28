# C-Menu FAQ

---

### C-Menu View - Why View Displays Question Marks

Q: When I try to view a document that contains line-drawing characters, C-Menu View displays question marks instead of the line-drawing characters. How can I fix this?

A: The file may contain characters above 0x80, which can't be translated by C-Menu View's multi-byte to wide character translation.These characters may be line drawing characters from the CP-437 character set. In that case, you can convert the file to UTF-8 encoding using a tool like 'iconv' or 'recode'. For example, you can use the following command:

```
    iconv -f CP437 -t UTF-8 inputfile.txt -o outputfile.txt
```

This will convert the line-drawing characters to their correct UTF-8 representations, allowing C-Menu View to display them properly. The images below show before and after using iconv.

<img src="screenshots/cp437_to_utf8.png" title="CP437 TO UTF8" />

---

### C-Menu View - How to Colorize Manual Pages

Q: How can I add color to manual pages?

A: Manual pages use ANSI SGR escape sequences to add color. You can use the following sed script to substitute your own colors for:

0x1b[1m bold
0x1b[2m dim
0x1b[3m italic
0x1b[4m underline
0x1b[22m normal intensity (bold/dim off)
0x1b[23m italic off
0x1b[24m underline off


```sed
s/\[2m/\[35;1m/g
s/\[3m/\[33;3;1m/g
s/\[4m/\[31;1m/g
s/\[22m/\[22;0m/g
s/\[23m/\[23;0m/g
s/\[24m/\[24;0m/g
```

You can save this script to a file, or use the one that comes with C-Menu, (~/menuapp/msrc/man.sed) and then use it like this:

    man bash
    man -Tutf8 bash | sed -f ~/menuapp/msrc/man.sed | view


This will display the bash manual page with the specified colors in C-Menu View.

<img src="screenshots/man-page.png" title="Colorizer" />

---

### C-Menu View - How to Colorize HTML Color Codes

Q: I want to colorize six digit html style hexadecimal colors, such as #RRGGBB, in C-Menu View. How can I do this?

A: You can use the following awk script to colorize six digit html style hexadecimal colors in C-Menu View:

```bash
awk -f ~/menuapp/msrc/colorize.awk yourfile.txt | view
```

This script matches six digit hexadecimal colors in the format #RRGGBB and adds the ANSI escape sequences to set the background color to the specified RGB values.

This will display the contents of yourfile.txt in C-Menu View with the specified hexadecimal colors colorized. The image below shows before and after colorizing.

<img src="screenshots/Colorizer.png" title="Colorizer" />

---

### C-Menu View - How to Customize Colors

Q: How can I customize the color scheme in C-Menu View?

A: If you have a modern color display, C-Menu View can display up to 16,777,216 different colors using ANSI escape sequences applicable to foreground and background. You can also redefine the standard ANSI color palette in ~/.minitrc. When you exit C-Menu, your system colors revert to their previous state.

<img src="screenshots/minitrc.png" title="C-Menu .minitrc" />

---

### C-Menu Menu, Form, Pick and View API

Q: I want to use the C-Menu API to develop my own code. How can I do that.

A: At the moment, you will have to rely on C-Menu's source code for documentation. If build C-Menu using CMake in the build directory, a C library will be installed in the lib64 directory, which you can link to your own executables. If there is sufficient interest, that capability will be expanded, improved, and a reference guide will be created for the API.

---

### C-Menu View - How to Use Tree-sitter With View

Q: How do I use the tree-sitter highlighter with C-Menu View?

A: Documentation on this feature is sparse at the moment.

1. Install tree-sitter
2. Install tree-sitter-cli
3. Install (and build) the tree-sitter parser for your language
4. Edit ~/.config/tree-sitter/config.json

You will find an example config.json in C-Menu's tree-sitter directory.

Type the following command:

```bash
    tree-sitter highlight source-file | view
```

<img src="screenshots/tree-sitter.png" title="C-Menu View Tree-Sitter" />


---