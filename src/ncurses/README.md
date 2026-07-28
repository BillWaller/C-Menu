# Directory UI_NCURSES

This directory contains user interface code that relies on the NCurses library.

Currently, this code focuses on proof-of-concept implementations of essential UI components using the NCurses API. Eventually, it will be used to compose a Uniform Abstraction Layer User Interface, which will assume the role of API/ABI.

The design objective is to create a Uniform API that will encapsulate NCurses so
that it is opaque to the rest of the codebase. This will make it possible to use different user interfaces without affecting the rest of the codebase.
