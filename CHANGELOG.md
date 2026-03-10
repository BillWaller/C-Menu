# Changelog

## [Released] - 2026-02-04

0.2.9 - 2026-03-10

Moved documentation to a separate repository and website, which is now available
at:

[C-Menu Comprehensive Reference](https://decision-inc.com/)

This was necessary because the documentation had become too large and unwieldy to
be easily navigable within the C-Menu repository, so only the main page was available.
With the new website on Github Pages, I was able to include all the Doxygen
pages including function cross references and diagrams.

The new website is much more user-friendly and easier to navigate, and it will allow me to keep the documentation up-to-date more easily.

0.2.9 - 2026-03-09

Much more work on chyron integration. Appears to be working as intended. In the
process, I found and fixed a number of bugs in form and one in lf. The debugging
process is somewhat tedious following disruptive feature additions, but we are
making progress.

Still more work on inte

0.2.9 - 2026-03-08

Continuing to work on chyron integration into Form and Pick. When using the
Installment Loan example, the chyron has been adjusted to only include relevant
options, and the key handling has been reworked to accommodate the new structure.
For example, when the user finishes entering all the fields in the form, the
chyron provides only F1 Help, F5 Calculate, and F9 Cancel, which makes sense
because the form is still in edit mode and the user can continue to edit fields
by using movement keys or by clicking on the fields. Once the user clicks
calculate, the chyron changes to include F1 Help, F4 Edit, F9 Cancel, and F10
Commit. Upon clicking F9 Commit (or pressing the F9 key), Form will execute the appropriate action specified by the user.

0.2.9 - 2026-03-07

Corrected Pick Page Down and Page Up key handling, which had been broken by the integration of the chyron facility into xwgetch. This was a major problem, and it caused the Pick module to be completely unusable. I have reimplemented the key handling, and it is now working as intended.

Modified Pick so that the chyron only includes Page Up and Page Down options
when there is more than 1 page of Pick ojbects. This is part of a larger effort
to integrate the chyron facility into all C-Menu programs. The chyron facility
allows C-Menu to generate dynamic chyrons based on the current state of the
program, while automatically keeping track of click zones so that xwgetch
can interpret mouse clicks as key codes. The idea is that the chyron should
only include relevant options.

0.2.9 - 2026-03-06

Added -e exclude regular expression to lf, which allows users to exclude files from the listing based on a regular expression. This is a powerful feature that can be used to filter out unwanted files from the listing.

Replaced mouse handling in Pick, Form, and Menu with xwgetch, which reads chyron
mouse selections as key codes. Now all mouse events are handled through a single function. xwgetch also handles asynchronous signal events.

Some code in menu_engine.c, which handled hierarchical menu navigation, had
somehow been deleted. This was a major problem, and it caused the menu system to be completely broken. I have reimplemented it, and it is now working as intended.

The addition of an Exercises section will provide a test bed for debugging as well
as a proof-of-concept for C-Menu. The first project is a suite of Workstation
Configuration programs for SDDM, Ghostty, Kitty, and Alacritty. These have been
integrated into the C-Menu example main menu. I will continue to add more
exercises along with documentation. The exercises will be designed to be both
educational and practical, and they will cover a wide range of topics related to
C-Menu development. One of the next projects, if time allows, will be a C-Menu
based general ledger using SQLite as the backend database.

0.2.9 - 2026-02-27

Added a number of improvements to CMake build to accommodate C-Menu's
revised file structure. Both CMake and straight Makefile builds produce
similar results, but CMake is more flexible, easier to work with, and less
prone to difficult-to-solve problems.

View's search function has been reworked to fix a number of issues with
buffer and page management, such as off-by-one page top and bottom
positioning.

lf (list files) - recursive option has been replaced by a depth option and
lf's core logic has been reworked to enhance reliability. Also removed mangled
lf syntax which had been used in some scripts to compensate for deficiencies in
lf's option handler.

Many documentation updates - API/ABI documentation is substantially complete.
Function Call Graphs, Called Graphs, and forward and backward references added.

Every function is documented, and most variables. All documentation has been
consolidated into a single html file available at:

[C-Menu Comprehensive Reference](https://billwaller.github.io/C-Menu/)

In View, replaced calls to mbtowc() with mbrtowc() because of spurious
accent marks populating the complex characters structures (cchar_t). Apparently,
mbtowc() is broken. mbrtowc() is the new and improved replacement.

The ANSI SGR parser and gamma correction in View is working perfectly. Try a
setting of 2.2 in ~/.minitrc to brighten up dull documents.

0.2.9 - 2026-02-09

Split the large menu.h into Separate .h files for each module, which should
make it easier for developers who want to work on a particular feature.

Modified Makefile and CMakeLists.txt to reflect the new file structure, and to add the new shared library libcm.so

Many unimplemented functions have been implemented, and the project is now in a much more complete state. I am still working on the documentation, but I have made significant progress on it, and I will be uploading it to the repository as I go.

I will look into adding a website for the project, and I will be uploading the documentation to the repository as I go. I am also working on a website for the project, which will be up soon.

---

[0.2.8] - 2026-02-04

Makefile

menu - menu.c menu_engine.c parse_menu_desc.c

form - form.c

pick - pick.c

view - view.c

ckeys - ckeys.c

enterchr - enterchr.c

enterstr - enterstr.c

optsp - optsp.c

whence - whence.c

stripansi - stripansi.c

iloan - iloan.c

lf - lf.c

add_executable(form form.c)
target_sources(form PRIVATE $<TARGET_OBJECTS:cmenu>)
target_link_libraries(form cm ${LIBS})

add_executable(enterchr enterchr.c)
target_link_libraries(enterchr cm ${LIBS})

add_executable(enterstr enterstr.c)
target_link_libraries(enterstr cm ${LIBS})

add_executable(iloan iloan.c)
target_link_libraries(iloan cm ${LIBS})

add_executable(lf lf.c)
target_link_libraries(lf cm ${LIBS})

add_executable(pick pick.c)
target_sources(pick PRIVATE $<TARGET_OBJECTS:cmenu>)
target_link_libraries(pick cm ${LIBS})

TARGETS menu
form
pick
view
ckeys
enterchr
enterstr
lf
optsp
stripansi
whence
cm

dwin.c
futil.c
scriou.c
exec.c
sig.c

COMMON_SRCS
curskeys.c
fields.c
form_engine.c
init_view.c
pick_engine.c
view_engine.c
init.c
mem.c
mview.c
opts.c)

- 0.2.8 - 2026-02-03
- 0.2.9 - 2026-02-04

### Fixed

- Prevent segmentation fault upon `close()` ([#28](link-to-pr))

## [0.2.8]- 2026-02-04

### Added

- Initial release of the project ([`a1b2c3d`](link-to-commit))

: https://github.com/BillWAller/C-Menu/releases/tag/C-Menu-0.2.8
: https://github.com/BillWaller/C-Menu/releases/tag/C-Menu-0.2.8
