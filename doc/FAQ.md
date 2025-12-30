# C-Menu FAQ

Bill Waller Copyright 2025

billxwaller@gmail.com

---

### C-Menu View - Why View Displays Question Marks

Q: When I try to view a document that contains line-drawing characters, C-Menu View displays question marks instead of the line-drawing characters. How can I fix this?

A: The file may contain characters above 0x80, which can't be translated by C-Menu View's character translators. If the offending characters CP-437 line drawing characters, you can convert the file to UTF-8 encoding using a tool like 'iconv' or 'recode'. For example, you can use the following command:

```
    iconv -f CP437 -t UTF-8 inputfile.txt -o outputfile.txt
```

The images below show, before, on the left, and after, on the right, using iconv.

<img src="screenshots/cp437_to_utf8.png" title="CP437 TO UTF8" />

As an interesting note, this also works for "less", which displays the decimal representation of of the CP437 characters. This could be handy if you have been coding since the 1980's and recognize them as CP437 line-drawing characters.

<img src="screenshots/cp437-line-draw-less.png" title="CP437 LESS" />

---

### C-Menu View - How to Colorize Manual Pages

Q: How can I add color to manual pages?

A: Manual pages use ANSI SGR escape sequences to add color.

```
0x1b[1m bold
0x1b[2m dim
0x1b[3m italic
0x1b[4m underline
0x1b[22m normal intensity (bold/dim off)
0x1b[23m italic off
0x1b[24m underline off
```

You can use the following sed script to substitute your own colors:

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

These instructions are admittedly sketchy and hard to follow. We will revise and clarify in the future.

---

### C-Menu Form - How to Integrate External Executables

Q: How can I integrate external programs into C-Menu Form, such as an SQL Query or calculation?

A: We can use the executable "iloan" included with C-Menu as an example.

The Form will have four fields. The user must enter any three of the four values. Only one field may be zero. Iloan will calculate the value of the missing field and C-Menu Form will display it.

First, create a form file named "~/menuapp/msrc/iloan.f" with the following contents:

```
H:Installment Loan Calculator

T:5:14:Principal Amount:
F:5:33:14:Currency

T:6:14:Number of Months:
F:6:33:5:Decimal_Int

T:7:10:Annual Interest Rate:
F:7:33:5:APR

T:8:16:Payment Amount:
F:8:33:12:Currency

T:10:1:First Payment Date (Yyyymmdd):
F:10:33:10:Yyyymmdd

C
?iloan.hlp
```

Next, create a help file named "~/menuapp/help/iloan.hlp" with the following contents:

```
Installment Loan Calculator Help
This form calculates the missing value of an installment loan.
Enter any three of the four values. Leave the value to be calculated as zero.
Only one field may be zero.
Fields:
Principal Amount - The total amount of the loan.
Number of Months - The total number of monthly payments.
Annual Interest Rate - The annual interest rate as a percentage.
Payment Amount - The amount of each monthly payment.
First Payment Date - The date of the first payment in YYYYMMDD format.
```

Add the following two lines to "~/menuapp/msrc/main.m":

```
:     Installment Loan Calculations
!form iloan.f -i iloan.dat -S iloan -o iloan.dat
```

This will add a menu item to the Main Menu that will launch the Iloan Form.
When you select the "Installment Loan Calculations" menu item, C-Menu Form will display the Iloan Form. After you enter any three of the four values and press Enter, C-Menu Form will call the "iloan" executable to calculate the missing value and display it in the form.

C-Menu Form will complain that "iloan.dat" does not exist the first time you run the form. This is normal. C-Menu Form will create the file when you exit the form.

### Installment Loan Calculator

<img src="screenshots/iloan.png" title="Installment Loan Calculator" />

After you enter the three values, you will see an option "F5 Calculate". Press F5 to calculate the missing value. When the calculation is complete, the missing value will be displayed in the form. You will then see an option, "F8 Edit", which will allow you to make changes and try again. If you don't wish to make changes, press "F10 Accept" to
write all four values to "iloan.dat" and exit the form.

The next time you run the form, C-Menu Form will read the values from "iloan.dat" and display them in the form.

Here's a summary of the important parts of the form file format:

### Line Type Speecifiers (H, T, F, and ?)

- # Comment line (ignored)
- H - The header to be displayed at the top of the form
- T - Text field (line:column:length:text)
- F - Input field (line:column:length:type)
- ? - A user supplied help file for the form. If no path is given, Form will look for a file with the same name as the form but with a .hlp extension. It will search in the current directory and then in the menu help directory, ~/menuapp/help.

### Field Delimiters:

The ":" character is used as a delimiter in the fields above, but any
character that is placed immediately after the line designator (H, T, F, ?)
can be used as a delimiter. For example, the following two lines are
equivalent:

```
T:2:4:Enter any three of the four values to calculate the fourth.
T|2|4|Enter any three of the four values to calculate the fourth.
```

### Data Types:

The following data types are currently supported for input fields:

       String: Any text

Decimal_Int: Integer number
Hex_Int: Hexadecimal integer
Float: Floating point number
Double: Double precision floating point number
Currency: Currency amount
APR: Annual Percentage Rate
Yyyymmdd: Date in YYYYMMDD format
HHMMSS: Time in HHMMSS format

Note that the data types determine field formatting, and on entry, numeric data is converted to its corresponding internal binary format, so that calculations can be performed. Both text and internal numeric binary data are available to the developer.

For applications that demand extreme accuracy, our plan is to integrate the "decNumber C Library", Copyright © IBM Corporation 2010, to provide 128 bit BCD (Binary Coded Decimal) in a future release.

The Field Format Specifiers can be any combination of upper and lower case,
and new types can be easily added by modifying the source code.

---
