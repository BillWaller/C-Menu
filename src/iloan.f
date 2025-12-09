H:Installment Loan Calculator
#
T:1:4:Enter any three of the four values to calculate the fourth.
T:2:4:Only one field can be left blank or zero.
T:3:4:Press F5 to calculate the missing value.
#
#
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
# -- End of Form Definition --
#
# Line Type Speecifiers (H, T, F, and ?)
#
# # - Comment line (ignored)
# H - The header to be displayed at the top of the form
# T - Text field (line:column:length:text)
# F - Input field (line:column:length:type)
# ? - A user supplied help file for the form. If no path is given, Form will
#     look for a file with the same name as the form but with a .hlp extension.
#     It will search in the current directory and then in the menu help
#     directory, ~/menuapp/help.
#
# Field Delimiters:
#
# The ":" character is used as a delimiter in the fields above, but any
# character that is placed immediately after the line designator (H, T, F, ?)
# can be used as a delimiter. For example, the following two lines are
# equivalent:
# T:2:4:Enter any three of the four values to calculate the fourth.
# T|2|4|Enter any three of the four values to calculate the fourth.
#
# Data Types:
#
#        String: Any text
#   Decimal_Int: Integer number
#       Hex_Int: Hexadecimal integer
#         Float: Floating point number
#        Double: Double precision floating point number
#      Currency: Currency amount
#           APR: Annual Percentage Rate
#
# Note that the data types affect only input validation. All data is stored
# internally as strings.
#
# The Field Format Specifiers can be any combination of upper and lower case,
# and new types can be easily added by modifying the source code.
#
