# Table of Contents

1. [Why View Displays Question Marks](#why-view-displays-question-marks)
2. [C-Menu View - How to Colorize Manual Pages](#c-menu-view---how-to-colorize-manual-pages)
3. [C-Menu View - How to Colorize HTML Color Codes](#c-menu-view---how-to-colorize-html-color-codes)
4. [C-Menu View - How to Customize Colors](#c-menu-view---how-to-customize-colors)
5. [C-Menu Menu, Form, Pick and View API](#c-menu-menu-form-pick-and-view-api)
6. [C-Menu View - How to Use Tree-sitter With View](#c-menu-view---how-to-use-tree-sitter-with-view)
7. [C-Menu Form - Integrating External Executables](#c-menu-form---integrating-external-executables)
8. [C-Menu - Interprocess Communications](#c-menu---interprocess-communications)
9. [C-Menu - What Happened to Delete by Inode](#c-menu---what-happened-to-delete-by-inode)
10. [C-Menu Pick - Selecting Multiple Files](#c-menu-pick---selecting-multiple-files)
11. [C-Menu lf - Where Are My Header Files?](#c-menu-lf---where-are-my-header-files)
12. [C-Menu View - View In A Box Window](#c-menu-view---view-in-a-box-window)

## Why View displays question marks

Q: When I try to view a document that contains line-drawing characters,
C-Menu View displays question marks instead of the line-drawing
characters. How can I fix this?

A: The file may contain characters above 0x80, which can't be
translated by C-Menu View's character translators. If the offending
characters CP-437 line drawing characters, you can convert the file to
UTF-8 encoding using a tool like 'iconv' or 'recode'. For example, you
can use the following command:

```bash
    iconv -f CP437 -t UTF-8 inputfile.txt -o outputfile.txt
```

The images below show, before, on the left, and after, on the right, using iconv.

![Convert CP437 to Unicode](screenshots/cp437_to_utf8.png)

As an interesting note, this also works for "less", which displays the
decimal representation of of the CP437 characters. This could be handy
if you have been coding since the 1980's and recognize them as CP437
line-drawing characters.

![CP437 less](screenshots/cp437-line-draw-less.png)

---

## C-Menu View - How to Colorize Manual Pages

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
C-Menu, (~/menuapp/msrc/man.sed) and then use it like this:

```bash
man bash
man -Tutf8 bash | sed -f ~/menuapp/msrc/man.sed | view
```

This will display the bash manual page with the specified colors in
C-Menu View.

![C-Menu View Manual Page](screenshots/man-page.png)

---

## C-Menu View - How to Colorize HTML Color Codes

Q: I want to colorize six digit html style hexadecimal colors, such as
\#RRGGBB, in C-Menu View. How can I do this?

A: You can use the following awk script to colorize six digit html
style hexadecimal colors in C-Menu View:

```bash
awk -f ~/menuapp/msrc/colorize.awk yourfile.txt | view
```

This script matches six digit hexadecimal colors in the format #RRGGBB
and adds the ANSI escape sequences to set the background color to the
specified RGB values.

This will display the contents of yourfile.txt in C-Menu View with the
specified hexadecimal colors colorized. The image below shows before
and after colorizing.

![Colorizer](screenshots/Colorizer.png)

---

## C-Menu View - How to Customize Colors

Q: How can I customize the color scheme in C-Menu View?

A: If you have a modern color display, C-Menu View can display up to
16,777,216 different colors using ANSI escape sequences applicable to
foreground and background. You can also redefine the standard ANSI
color palette in ~/.minitrc. When you exit C-Menu, your system colors
revert to their previous state.

![C-Menu Configuration File](screenshots/minitrc.png)

## C-Menu Menu, Form, Pick and View API

Q: I want to use the C-Menu API to develop my own code. How can I do that.

A: At the moment, you will have to rely on C-Menu's source code for
documentation. If build C-Menu using CMake in the build directory, a C
library will be installed in the lib64 directory, which you can link to
your own executables. If there is sufficient interest, that capability
will be expanded, improved, and a reference guide will be created for
the API.

---

## C-Menu View - How to Use Tree-sitter With View

Q: How do I use the tree-sitter highlighter with C-Menu View?

A: Documentation on this feature is sparse at the moment.

Here are the basic steps to get started with tree-sitter and C-Menu View.

## Install tree-sitter-cli

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

### download from github

[tree-sitter-c](https://github.com/tree-sitter/tree-sitter-c)

```bash
cd tree-sitter-c
tree-sitter build
tree-sitter generate
```

You should edit ~/.config/tree-sitter/config.json to include the
languages you want to use with C-Menu View.

You will find an example config.json in C-Menu's tree-sitter directory.

Type the following command:

```bash
    tree-sitter highlight source-file | view
```

![C-Menu View - Tree-Sitter](screenshots/tree-sitter1.png)
These instructions are admittedly sketchy and hard to follow. We will
revise and clarify in the future.

---

## C-Menu Form - Integrating External Executables

Q: How can I integrate external programs into C-Menu Form, such as an
SQL Query or calculation?

A: We can use the executable "iloan" included with C-Menu as an example.

The Form will have four fields. The user must enter any three of the
four values. Only one field may be zero. Iloan will calculate the
value of the missing field and C-Menu Form will display it.

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
C-Menu Form will display the Iloan Form. After you enter any three of
the four values and press Enter, C-Menu Form will call the "iloan"
executable to calculate the missing value and display it in the form.

C-Menu Form will complain that "iloan.dat" does not exist the first
time you run the form. This is normal. C-Menu Form will create the
file when you exit the form.

## C-Menu - Using the Installment Loan Calculator

![Installment Loan Calculator](screenshots/iloan.png)

After you enter the three values, you will see an option "F5
Calculate". Press F5 to calculate the missing value. When the
calculation is complete, the missing value will be displayed in the
form. You will then see an option, "F8 Edit", which will allow you to
make changes and try again. If you don't wish to make changes, press
"F10 Accept" to write all four values to "iloan.dat" and exit the form.

The next time you run the form, C-Menu Form will read the values from
"iloan.dat" and display them in the form.

Here's a summary of the important parts of the form file format:

## C-Menu Form - Line Type Speecifiers (H, T, F, and ?)

- \# Comment line (ignored)
- H - The header to be displayed at the top of the form
- T - Text field (line:column:length:text)
- F - Input field (line:column:length:type)
- ? - A user supplied help file for the form. If no path is given, Form
  will look for a file with the same name as the form but with a
  .hlp extension. It will search in the current directory and then in
  the menu help directory, ~/menuapp/help.

## C-Menu Form - Field Delimiters

The ":" character is used as a delimiter in the fields above, but any
character that is placed immediately after the line designator (H, T, F, ?)
can be used as a delimiter. For example, the following two lines are
equivalent:

T:2:4:Enter any three of the four values to calculate the fourth.
T|2|4|Enter any three of the four values to calculate the fourth.

## C-Menu Form - Data Types

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

## C-Menu - Interprocess Communications

Q: How does C-Menu send and receive data to external programs?

A: Currently, C-Menu is limited to communicating through files or pipes.

When you start C-Menu Form, Pick, or View, you can specify

-S executable
or
-R executable

-S runs executables that provide input data to C-Menu via pipe, and
-R runs executables that receive output data from C-Menu via pipe.

The way it works is, C-Menu creates dual ended pipes, each with a read
and write end before forking and spawning the executables. In the
example (Installment Loan Calculations) above, C-Menu Form substitutes
a write pipe for the standard output of the child process, and opens
the read end of the pipe for it's input. The called executable writes
data to the pipe and C-Menu form reads the other end.

We can just as easily use named pipes or network sockets, although it
requires a bit more configuration. If there is sufficient interest, C
-Menu will have an event-driven server to handle network communications
and asynchronous tasks. It is likely that server will be implemented in
Rust to take advantage of tools like the Tokio and Serde crates.

---

## C-Menu - What Happened to Delete by Inode

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

## C-Menu Pick - Selecting Multiple Files

Q: In C-Menu Pick, how can I select multiple files to edit with vi?

A: You can use the -M option to enable multi-select in C-Menu Pick.
Here is an example command that allows you to select multiple files and
open them in C-Menu Vi:

```bash
\!pick -S "lf -r ./ .*\.[ch]$" -M -R vi -T "Project Tree - Select Files to Edit"

```

Select as many files as you want to edit and press F10. C-Menu Pick
will open the first file. If you are using Nvim, Vim, Less, or C-Menu
View, you can type: \":n\<enter\>\" to open the next file.

---

## C-Menu lf - Where Are My Header Files?

Q: You have an option to edit C source files in the Project Tree menu,
but it doesn't list my header files. Can you fix that?

A: Sure. My bad. Of course you want your header files to be listed with
your C files. Here is the new C-Menu line with the regular expression
corrected to include both .c and .h files:

```bash
\!pick -S "lf -r ./ .\*\.[ch]$" -n 1 -R vi -T "Project Tree - Select File to
Edit"
```

---

## C-Menu View - View In A Box Window

Q: How do I highlight a C source file using tree-sitter and view it in
a C-Menu View box window.

```bash
view -L 40 -C 80 -S "tree-sitter highlight view_engine.c"
```

Leave out the -L 40 and -C 80 to display the file in full screen.

Alternatively, you can use the following command.

```bash
tree-sitter highlight view_engine.c | view -L 40 -C 80 -T "Highlighted view_engine.c"
```

The "-S" option will tell view to execute the command and display the
output. As an added bonus, if you don't provide a title with "-T, view
will use the "-S" command as the title:
