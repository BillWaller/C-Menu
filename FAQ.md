# ![Menu](screenshots/Menu.png)

## FAQ - Table of Contents

- [TrueColor Support](#truecolor-support)
- [Why View Displays Question Marks](#why-view-displays-question-marks)
- [View - How to Colorize Manual Pages](#view---how-to-colorize-manual-pages)
- [View - How to Colorize HTML Color Codes](#view---how-to-colorize-html-color-codes)
- [View - How to Customize Colors](#view---how-to-customize-colors)
- [Menu, Form, Pick, and View API](#menu-form-pick-and-view-api)
- [View - How to Use Tree-Sitter with View](#view---how-to-use-tree-sitter-with-view)
- [Menu Form - Integrating External Executables](#menu-form---integrating-external-executables)
- [Menu - Using the Installment Loan Calculator](#menu---using-the-installment-loan-calculator)
- [Menu - Interprocess Communications](#menu---interprocess-communications)
- [Menu - What Happened to Delete by Inode](#menu---what-happened-to-delete-by-inode)
- [Pick - Selecting Multiple Files](#pick---selecting-multiple-files)
- [Menu lf - Where Are My Header Files?](#menu-lf---where-are-my-header-files)
- [View In a Box Window](#view-in-a-box-window)

## TrueColor Support

Q: How can I tell if my terminal supports TrueColor (24-bit color)?

A: You can run the following command to test if your terminal supports
TrueColor:

```bash
awk 'BEGIN{
    s="/\\/\\/\\/\\/";
    for (colnum = 0; colnum<77; colnum++) {
        r = 255-(colnum*255/76);
        g = (colnum*510/76);
        b = (colnum*255/76);
        if (g>255) g = 510-g;
        printf "\033[48;2;%d;%d;%dm", r,g,b;
        printf "\033[38;2;%d;%d;%dm", 255-r,255-g,255-b;
        printf "%s\033[0m", substr(s,(colnum%4)+1,1);
    }
    printf "\n";
}'
```

If your terminal supports TrueColor, you should see a smooth gradient
of colors from red to green to blue. If you see a limited number of
colors or a blocky gradient, your terminal may not support true color.

![TrueColor](screenshots/truecolor.png)

Another way to check is to type:

```bash
    tput colors
```

If the output is 16777216, your terminal supports true color.

You can also type:

```bash
    echo $COLORTERM
```

If the output is "Truecolor" or "24bit", your terminal supports true
color.

If your hardware supports TrueColor, but the above indicators report only
256 colors or less, you may need to set the TERM variable to a value
that supports Truecolor. For example, you can set the TERM variable to
"xterm-direct" by adding the following line to your ~/.bashrc or ~/.zshrc file:

```bash
export TERM=xterm-direct
```

Note: When I tried xterm-direct, I found that it does indeed support 16M
colors and 65K color pairs, but it fails to set "Can-Change-Color" ("ccc")
to true. That work with Menu because it allows the user to define colors.

I use one of::

```bash
export TERM=xterm-ghostty
export TERM=xterm-kitty
```

For some reason, both the ghostty and kitty terminfos set Number of Colors
to 256, and that is perfectly understandable considering that:

1. A terminal emulator primarily deals with character cells, and 256 colors
   is more than adequate.
2. Neither Ghostty nor Kitty rely on NCurses for color management, so it is
   likely they get their limits from hardware instead of terminfo.

If you need more than 256 colors, you can get around this issue by using
infocmp to create a custom terminfo entry that specifies higher color
capabilities. Here is an example of how to do this:

```bash
infocmp xterm-ghostty > xterm-ghostty.terminfo
```

Add or modify the following lines in the xterm-ghostty or xterm-kitty:

```bash
ccc, colors#0x1000000, pairs#0x10000
```

You can also refer to this helpful list of terminals and their color
support:

[https://gist.github.com/XVilka/8346728](https://gist.github.com/XVilka/8346728)

If your terminal still does not support true color, you can try using a
different terminal emulator that does, such as Alacritty, Kitty, or Ghostty.

---

## Why View Displays Question Marks

Q: When I try to view a document that contains line-drawing characters,
View displays question marks instead of the line-drawing
characters. How can I fix this?

A: The file may contain characters above 0x80, which can't be
translated by View's character translators. If the offending
characters CP-437 line drawing characters, you can convert the file to
UTF-8 encoding using a tool like 'iconv' or 'recode'. For example, you
can use the following command:

```bash
iconv -f CP437 -t UTF-8 inputfile.txt -o outputfile.txt
```

The images below show, before, on the left, and after, on the right, using iconv.

![Convert CP437 to Unicode](screenshots/cp437_to_utf8.png)

As an interesting note, this also works for "less", which displays the
decimal representation of the CP437 characters. This could be handy
if you have been coding since the 1980s and recognize them as CP437
line-drawing characters.

![CP437 less](screenshots/cp437-line-draw-less.png)

---

## View - How to Colorize Manual Pages

Q: How can I add color to manual pages?

A: Manual pages use ANSI SGR escape sequences to add color.

0x1b[1m bold

0x1b[2m dim

0x1b[3m italic

0x1b[4m underline

0x1b[22m normal intensity (bold/dim off)

0x1b[23m italic off

0x1b[24m underline off

You can use the following sed script to substitute your own colors:

```sed
s/\[2m/\[35;1m/g
s/\[3m/\[33;3;1m/g
s/\[4m/\[31;1m/g
s/\[22m/\[22;0m/g
s/\[23m/\[23;0m/g
s/\[24m/\[24;0m/g
```

You can save this script to a file, or use the one that comes with
Menu, (~/menuapp/msrc/man.sed) and then use it like this:

```bash
man -Tutf8 bash | sed -f ~/menuapp/msrc/man.sed | view
```

This will display the bash manual page with the specified colors in
View.

![View Manual Page](screenshots/man-page.png)

---

## View - How to Colorize HTML Color Codes

Q: I want to colorize six digit HTML style hexadecimal colors, such as
\#RRGGBB, in View. How can I do this?

A: You can use the following awk script to colorize six digit HTML
style hexadecimal colors in View:

```bash
awk -f ~/menuapp/msrc/colorize.awk yourfile.txt | view
```

This script matches six digit hexadecimal colors in the format #RRGGBB
and adds the ANSI escape sequences to set the background color to the
specified RGB values.

This will display the contents of yourfile.txt in View with the
specified hexadecimal colors colorized. The image below shows before
and after colorizing.

![Colorizer](screenshots/Colorizer.png)

---

## View - How to Customize Colors

Q: How can I customize the color scheme in View?

A: If you have a modern color display, View can display up to
16,777,216 different colors using ANSI escape sequences applicable to
foreground and background. You can also redefine the standard ANSI
color palette in ~/.minitrc. When you exit Menu, your system colors
revert to their previous state.

![Menu Configuration File](screenshots/minitrc.png)

## Menu, Form, Pick, and View API

Q: I want to use the Menu API to develop my own code. How can I do that.

A: At the moment, you will have to rely on Menu's source code for
documentation. If build Menu using CMake in the build directory, a C
library will be installed in the lib64 directory, which you can link to
your own executables. If there is sufficient interest, that capability
will be expanded, improved, and a reference guide will be created for
the API.

---

## View - How to Use Tree-Sitter with View

Q: How do I use the tree-sitter highlighter with View?

A: Documentation on this feature is sparse at the moment.

Here are the basic steps to get started with tree-sitter and View.

## Install Tree-Sitter-CLI

```bash
cargo install tree-sitter-cli
```

Verify installation:

```bash
tree-sitter --version
```

[tree-sitter-rust](https://github.com/tree-sitter/tree-sitter-rust)

```bash
cd tree-sitter-rust
tree-sitter build
tree-sitter generate
```

### Download From Github

[tree-sitter-c](https://github.com/tree-sitter/tree-sitter-c)

```bash
cd tree-sitter-c
tree-sitter build
tree-sitter generate
```

You should edit ~/.config/tree-sitter/config.json to include the
languages you want to use with View.

You will find an example config.json in Menu's tree-sitter directory.

Type the following command:

```bash
tree-sitter highlight source-file | view
```

![View - Tree-Sitter](screenshots/tree-sitter1.png)
These instructions are admittedly sketchy and hard to follow. We will
revise and clarify in the future.

---

## Menu Form - Integrating External Executables

Q: How can I integrate external programs into Menu Form, such as an
SQL Query or calculation?

A: We can use the executable "iloan" included with Menu as an example.

The Form will have four fields. The user must enter any three of the
four values. Only one field may be zero. Iloan will calculate the
value of the missing field and Menu Form will display it.

First, create a form file named "~/menuapp/msrc/iloan.f" with the
following contents:

![Installment Loan Description File](screenshots/iloan.f.png)

Next, create a help file named "~/menuapp/help/iloan.hlp" with the following
contents:

Installment Loan Calculator Help

This form calculates the missing value of an installment loan.

Enter any three of the four values. Leave the value to be calculated as
zero. Only one field may be zero.

Fields:
Principal Amount - The total amount of the loan.
Number of Months - The total number of monthly payments.
Annual Interest Rate - The annual interest rate as a percentage.
Payment Amount - The amount of each monthly payment.
First Payment Date - The date of the first payment in YYYYMMDD format.

Add the following two lines to "~/menuapp/msrc/main.m":

\: Installment Loan Calculations

!form iloan.f -i iloan.dat -S iloan -o iloan.dat

This will add a menu item to the Main Menu that will launch the Iloan
Form. When you select the "Installment Loan Calculations" menu item,
Menu Form will display the Iloan Form. After you enter any three of
the four values and press Enter, Menu Form will call the "iloan"
executable to calculate the missing value and display it in the form.

Menu Form will complain that "iloan.dat" does not exist the first
time you run the form. This is normal. Menu Form will create the
file when you exit the form.

## Menu - Using the Installment Loan Calculator

![Installment Loan Calculator](screenshots/iloan.png)

After you enter the three values, you will see an option "F(5)
Calculate". Press F(5) to calculate the missing value. When the
calculation is complete, the missing value will be displayed in the
form. You will then see an option, "F(8) Edit", which will allow you to
make changes and try again. If you don't wish to make changes, press
"F(10) Accept" to write all four values to "iloan.dat" and exit the form.

The next time you run the form, Menu Form will read the values from
"iloan.dat" and display them in the form.

Here's a summary of the important parts of the form file format:

## Menu Form - Line Type Speecifiers (H, T, F, and ?)

- \# Comment line (ignored)
- H - The header to be displayed at the top of the form
- T - Text field (line:column:length:text)
- F - Input field (line:column:length:type)
- ? - A user supplied help file for the form. If no path is given, Form
  will look for a file with the same name as the form but with a
  .hlp extension. It will search in the current directory and then in
  the menu help directory, ~/menuapp/help.

## Menu Form - Field Delimiters

The ":" character is used as a delimiter in the fields above, but any
character that is placed immediately after the line designator (H, T, F, ?)
can be used as a delimiter. For example, the following two lines are
equivalent:

T:2:4:Enter any three of the four values to calculate the fourth.
T|2|4|Enter any three of the four values to calculate the fourth.

## Menu Form - Data Types

The following data types are currently supported for input fields:

- String: Any text
- Decimal_Int: Integer number
- Hex_Int: Hexadecimal integer
- Float: Floating point number
- Double: Double precision floating point number
- Currency: Currency amount
- APR: Annual Percentage Rate
- Yyyymmdd: Date in YYYYMMDD format
- HHMMSS: Time in HHMMSS format

Note that the data types determine field formatting, and on entry,
numeric data is converted to its corresponding internal binary format,
so that calculations can be performed. Both text and internal numeric
binary data are available to the developer.

For applications that demand extreme accuracy, our plan is to integrate
the "decNumber C Library", Copyright © IBM Corporation 2010, to provide
128 bit BCD (Binary Coded Decimal) in a future release.

The Field Format Specifiers can be any combination of upper and lower
case, and new types can be easily added by modifying the source code.

---

## Menu - Interprocess Communications

Q: How does Menu send and receive data to external programs?

A: Currently, Menu is limited to communicating through files or pipes.

When you start Menu Form, Pick, or View, you can specify

-S executable
or
-R executable

-S runs executables that provide input data to Menu via pipe, and
-R runs executables that receive output data from Menu via pipe.

The way it works is, Menu creates dual ended pipes, each with a read
and write end before forking and spawning the executables. In the
example (Installment Loan Calculations) above, Menu Form substitutes
a write pipe for the standard output of the child process, and opens
the read end of the pipe for its input. The called executable writes
data to the pipe and Menu form reads the other end.

We can just as easily use named pipes or network sockets, although it
requires a bit more configuration. If there is sufficient interest, C
-Menu will have an event-driven server to handle network communications
and asynchronous tasks. It is likely that server will be implemented in
Rust to take advantage of tools like the Tokio and Serde crates.

---

## Menu - What Happened to Delete by Inode

Q: I noticed you have a menu option named, "Delete by Inode", but it doesn't work.

A: Thanks for reminding me. I will remove that choice from the menu
until I can figure out a way to make it safe.

The original command was:

```bash
find . -inum [inode-number] -delete
```

That is too dangerous if the user makes a mistake, so I replaced the
find with "rm -i".

```bash
  !pick -S "ls -i" -n 1 -R "rm -i"
```

It doesn't delete files by inode, but at least it prompts the user
before deleting files. Still, it's not what I intended, so it will be
removed until I find a better solution.

I may eventually write a C or Rust program that will safely delete
files by inode after confirming the correct file with the user as well
as saving deleted files in a trash bin, on an opt-out basis.

---

## Pick - Selecting Multiple Files

Q: In Pick, how can I select multiple files to edit with vi?

A: You can use the -M option to enable multi-select in Pick.
Here is an example command that allows you to select multiple files and
open them in Menu Vi:

```bash
\!pick -S "lf -r ./ .*\.[ch]$" -M -R vi -T "Project Tree - Select Files to Edit"

```

Select as many files as you want to edit and press F10. Pick
will open the first file. If you are using Nvim, Vim, Less, or Menu
View, you can type: \":n\<enter\>\" to open the next file.

---

## Menu lf - Where Are My Header Files?

Q: You have an option to edit C source files in the Project Tree menu,
but it doesn't list my header files. Can you fix that?

A: Sure. My bad. Of course you want your header files to be listed with
your C files. Here is the new Menu line with the regular expression
corrected to include both .c and .h files:

```bash
\!pick -S "lf -r ./ .\*\.[ch]$" -n 1 -R vi -T "Project Tree - Select File to
Edit"
```

---

## View In a Box Window

Q: How do I highlight a C source file using tree-sitter and view it in
a View box window.

```bash
view -L 40 -C 80 -S "tree-sitter highlight view_engine.c"
```

Leave out the -L 40 and -C 80 to display the file in full screen.

Alternatively, you can use the following command.

```bash
tree-sitter highlight view_engine.c | view -L 40 -C 80 -T "Highlighted view_engine.c"
```

The "-S" option will tell view to execute the command and display the
output. As an added bonus, if you don't provide a title with "-T", view
will use the "-S" command as the title:
