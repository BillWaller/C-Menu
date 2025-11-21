# CMenu Field Development Guide

## Validation

CMenu's fields program only supports modest field validation types at this time.
The current validation types less than ideal for many applications.

    None       No validation
    Non-blank  Must not be blank
    Numeric    Must be a numeric value

Future versions of CMenu must include more advanced validation options, such as:

    Range       Within a specified numeric range
    File        Valid file path
    Email       Verified email address
    URL         Verified URL
    Date        Valid date
    Regex       Match a custom regular expression
    Database    Must exist and qualify in a specified database
    Choices     Specified set of choices

- Should implement Pick Lists for choices

## Formatting

CMenu's fields program currently only supports modest field formatting options.
The current formatting options are less than ideal for many applications.

Future versions of CMenu must include more advanced formatting options, such as:

    Decimal Places
    Thousand Separators
    Currency
    Percentage
    Scientific Notation
    Date/Time
    Custom Masks
    Uppercase/Lowercase
    Trimming/ Padding
    Alignment (Left, Right, Center)
    Conditional

## Completion

CMenu's fields program currently does not support field auto-completion options.
This could be very useful for many applications, especially when entering repetitive
data. Future versions of CMenu should include auto-completion options, such as:

    Suggest from previous entries
    Suggest from predefined list
    Suggest from database query
    Custom completion logic via scripting

## Custom Field Types

CMenu's fields program currently supports a limited set of field types. Future versions of CMenu should allow developers to create custom field types, such as:

    Date Picker
    Color Picker
    File Selector
    Rich Text Editor
    Multi-Select List
    Slider
    Toggle Switch
    Custom Widgets via Scripting

## Scripting and Extensibility

To enhance the flexibility of field definitions, CMenu should support scripting capabilities that allow developers to define custom validation, formatting, and completion logic. This could be achieved through:

    Embedded Scripting Languages (e.g., Lua, Python)
    Custom Plugins or Modules
    API for Field Manipulation
    Event Hooks for Field Events (e.g., onChange, onFocus)
    Keep running totals and balances for accounting applications

By implementing these enhancements, CMenu can become a more powerful and flexible
tool for managing fields in various applications.
