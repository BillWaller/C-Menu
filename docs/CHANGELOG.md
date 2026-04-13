# Changelog

## [Released] - 2026-02-04

0.2.9 - 2026-04-15

The -N option (line numbering) in view originally required an argument of true, false, yes, no, t, f, y, or n, but a user suggested that this was a little clumsy, and that it would be more intuitive if the -N option simply turned on line numbering when included, and turned it off when omitted. I agreed with this suggestion, and I have changed the behavior of the -N option accordingly. So, the -N option has been changed to be consistent with other boolean options. Line numbering will be turned on by specifying the -N option, whether or not it is followed by an affirmative indicator. In the case that you have f_ln set to true in your configuration, you can still turn off line numbering by including the "-N f" or "-N n" option, which will override the configuration setting. You can also toggle line numbering on and off from within view by pressing -N from a command prompt. I am trying to keep View's operation as close to vi, vim, nvim, and less as possible

0.2.9 - 2026-04-14

After thinking about the resizing issue in view, I decided to implement yet another option, and that is to fill the terminal emulator screen. My reasoning was that a user typically shrinks the terminal window to make more room for another application window, not to make the viewing area smaller. And, when a user expands the terminal window which contains the view window, they want to see more of the document. So, when resizing, the view window will first fill the terminal emulator window, and the user can continue to shrink the terminal window, but no viewing area will be lost to blank space. The objective is to achieve the most efficient use of screen area.

The really obnoxious bugs that emerged during the resizing refactor have been fixed, and I will be doing a lot of testing to find and fix any remaining bugs. The resizing code is sort of complicated, and it is likely that there are still some edge cases that I haven't encountered yet.

0.2.9 - 2026-04-12

Refactoring of View's live resizing for both full screen and box window modes.
If you need to shrink the View window, simply resize the terminal window.
Expansion is a little more complicated. The issue is not so much technical, but
rather expectations. If the View window is in box mode, and it is originally
sized to be 3/4 of the terminal window, on resizing of the terminal window,
should it calculate resize lines and columns based on the ratio of the View
window to the terminal window, should it add the difference in size to the View
window, or should it fill the terminal emulator screen? My current impulse is to implement the first option, which is to calculate the new size based on the ratio of the View window to the terminal window. This seems to be the most intuitive and consistent behavior, but I am open to feedback on this.

0.2.9 - 2026-04-11

Major progress is being made in refactoring the main components of C-Menu to provide pop-up and drop-down helper widgets. This is a major step in the evolution of C-Menu, and it will allow for much more flexible and powerful user interfaces. The first component to be refactored is the chyron, which is a pop-up widget that can be used to display information and options to the user. The chyron is implemented as a separate module, and it can be easily integrated into any C-Menu program. The next step will be to refactor the Pick module to use the chyron for displaying options, which will allow for much more dynamic and context-sensitive user interfaces. For example, when entering form fields, the user can press or click the help button to display a list of possible ogjects with which to fill the field and then select one with the mouse or keyboard. This will be a much more intuitive and efficient way to fill out forms, and it will also allow for much more complex forms with many fields. This is in preparation for the next major release, which will include a number of new features and improvements, including plug-ins for external data, such as weather, news, stock quotes, and more.

Resizing of the View windows is underway. This will require much testing as
there are four windows within the View window, the box window, the line number
window, the command line window, and the virtual pad that must be resized and
sequenced perfectly to avoid distracting artifacts.

0.2.9 - 2026-04-10

Used valgrind to find and fix a number of memory leaks and other memory-related issues. At this point, C-Menu is exiting with no memory leaks.

Removed the -a (list hidden files) option from lf, made that the default
behavior, and added -n option to exclude hidden files.

Until now, the default maximum depth for lf was set to 3, which was convenient for testing, but could be confusing for users accustomed to find. The default maximum depth has been changed to 0, which means no limit. Users may want to specify a reasonable maximum depth to avoid unnecessary voluminous output.

In benchmark testing, lf performed exceptionally well. It's fast.

0.2.9 - 2026-04-01

Most of the known bugs have been fixed, and the code is in a much more stable state. I will be doing a lot of testing and debugging to find and fix any remaining bugs, but I am confident that the code is in a good state for the next release, which will be version 0.3.x. The next release will include a number of new features and improvements, including plug-ins for external data, such as weather, news, stock quotes, and more.

0.2.9 - 2026-03-31

More bugs fixed related to transition to argp.

Added set_chyron_key_cp() function to allow the caller to specify a custom color pair
for individual chyron keys. This allows for more flexible and customizable chyrons, and it can be used to highlight important options or to differentiate between different types of options. The color pair is specified as a six-digit hexadecimal RGB value, and it can be set to any color the user desires. This function is part of the larger effort to integrate the chyron facility into all C-Menu programs, which will allow for much more dynamic and context-sensitive user interfaces.

0.2.9 - 2026-03-30

Continued testing and debugging after having substantially refactored the main components of C-Menu to provide pop-up and drop-down helper widgets. This is a major step in the evolution of C-Menu, and it will allow for much more flexible and powerful user interfaces. The first component to be refactored is the chyron, which is a pop-up widget that can be used to display information and options to the user. The chyron is implemented as a separate module, and it can be easily integrated into any C-Menu program. The next step will be to refactor the Pick module to use the chyron for displaying options, which will allow for much more dynamic and context-sensitive user interfaces. For example, when entering form fields, the user can press or click the help button to display a list of possible objects with which to fill the field and then select one with the mouse or keyboard. This will be a much more intuitive and efficient way to fill out forms, and it will also allow for much more complex forms with many fields. This is in preparation for the next major release, which will include a number of new features and improvements, including plug-ins for external data, such as weather, news, stock quotes, and more.

Specifically, the following improvements have been made:
Combined Menu, Form, Pick, View, and CKeys into a single executable
Incorporated argp for option processing
Added Popup and Drop-down facility for Menu, Form, Pick, View, and CKeys
Added line numbering to View
Refactored View navigation to maintain a line number table and use the table to improve navigation performance
Added file type selection to lf
Added an exclude regular expression option to lf
Modified Pick to return to the picker after executed the selected action
Added a new Exercises section to the documentation, which includes a number of exercises designed to help developers learn how to use C-Menu and to provide a test bed for debugging. The first exercise is a suite of Workstation Configuration programs for SDDM, Ghostty, Kitty, and Alacritty. These have been integrated into the C-Menu example main menu. I will continue to add more exercises along with documentation. The exercises will be designed to be both educational and practical, and they will cover a wide range of topics related to C-Menu development. One of the next projects, if time allows, will be a C-Menu based general ledger using SQLite as the backend database.
Added SSH authentication and logging to RSH
Added a wait popup to View and Pick for slow provider programs

This has no doubt introduced some bugs, so I will be doing a lot of testing and debugging before moving on to the next release, which will be version 0.3.x. The next release will include a number of new features and improvements, including plug-ins for external data, such as weather, news, stock quotes, and more.

0.2.9 - 2026-03-29

If anyone knows of bugs that I need to address, now is the time to let me know.
I will continue working with the pop-ups and drop-downs and I will be adding more exercises to the documentation, but I want to make sure the code is as error-free as possible before moving on to the next release, as version 0.3.x will embark on some major new features.

Testing has revealed a number of off-by-one errors in view, which I have fixed,
at least all those that I know about. I have also added a wait popup to view and pick, which will be displayed in the unlikely event that a provider program takes longer than 200ms to respond. The wait popup is a simple window with a message that counts down in seconds until input arrives or timeout, at which time the user is presented with an option to cancel. This is a possibility in the event that the provider program produces no output or stops waiting on user input. The timer checks the open pipe for input and also checks that the provider has terminated. If the program seems to be hung, the user can press Ctrl-C to cause a SIGTERM interrupt and then choose F9 to cancel. The wait popup is implemented in the parent process, which allows the countdown to timeout to be displayed without blocking the main thread of the program. The wait popup is designed to be non-intrusive, so it doesn't interfere with the user's workflow.

0.2.9 - 2026-03-28

Accomplished the first step in refactoring the main components of C-Menu to provide pop-up and drop-down helper widgets. This is a major step in the evolution of C-Menu, and it will allow for much more flexible and powerful user interfaces. The first component to be refactored is the chyron, which is a pop-up widget that can be used to display information and options to the user. The chyron is implemented as a separate module, and it can be easily integrated into any C-Menu program. The next step will be to refactor the Pick module to use the chyron for displaying options, which will allow for much more dynamic and context-sensitive user interfaces. For example, when entering form fields, the user can press or click the help button to display a list of possible ogjects with which to fill the field and then select one with the mouse or keyboard. This will be a much more intuitive and efficient way to fill out forms, and it will also allow for much more complex forms with many fields. This is in preparation for the next major release, which will include a number of new features and improvements, including plug-ins for external data, such as weather, news, stock quotes, and more.

0.2.9 - 2026-03-24

Addes SSH authentication to RSH for extra security, and also implemented logging
to the system log for all RSH sessions. The SSH authentication is implemented using the libssh library, and it allows users to authenticate using their SSH keys instead of passwords. This is a much more secure method of authentication, and it also allows for easier management of user credentials. The logging to the system log is implemented using the syslog() function, and it allows administrators to keep track of all RSH sessions for auditing purposes. On systems with systemd, the user can type journalctl -t rsh to view the log entries for RSH sessions.

Added a wait popup to View and Pick, for the unlikely cases in which a provider
program takes longer than 200ms to respond. The wait popup is a simple window with a message that counts down in seconds until input arrives or timeout, at which time the user is presented with an option to cancel. This is a possibility in the event that the provider program produces no output or stops waiting on user input. The timer checks the open pipe for input and also checks that the provider has terminated. If the program seems to be hung, the user can press Ctrl-C to cause a SIGTERM interrupt and then choose F9 to cancel. The wait popup is implemented in the parent process, which allows the countdown to timeout to be displayed without blocking the main thread of the program. The wait popup is designed to be non-intrusive, so it doesn't interfere with the user's workflow.

0.2.9 - 2026-03-19

Added line numbering option "-N" to View to display line numbers in the left
margin. The line numbers have a separate window, so it doesn't interfere with
horizontal scrolling. The line numbers are right-aligned and padded with spaces to maintain a consistent width. The C-Menu configuration now has options for foreground and background colors for line numbers. These are six-digit hexadecimal RGB values, and they can be set to any color the user desires.

Also refactored View navigation to maintain a line number table and use the
table to improve navigation performance. The table is allocated dynamically, and
consumes four bytes per line. The line number table is updated whenever the user scrolls vertically, and it allows for much faster navigation to specific lines, especially in large documents. The line number table is also used to maintain the correct line numbers in the left margin when the user scrolls horizontally.

0.2.9 - 2026-03-13

Added additional file selection options to lf. Now, you can
list files of specific types by specifying
-t bcdplfsu (in any order or combination)
b block devices
c character devices
d directories
p named pipes
l symbolic links
f regular files
s sockets
u unknown file types
You can select multiple file types and any you don't select will not be emitted.
For example, lf -t d will only list directories, while lf -t f will only list regular files. lf -t df will list both directories and regular files, and lf -t bcdplfsu will list all file types, which is the default behavior with no -t option.

0.2.9 - 2026-03-13

Modified Pick to return to the picker after executed the selected action. This
is a much more intuitive behavior, and it allows the user to execute multiple
actions without having to reopen the picker each time. One of the new Exercises
will make this abundantly clear.

Added a new Exercises section to the documentation, which includes a number of exercises designed to help developers learn how to use C-Menu and to provide a test bed for debugging. The first exercise is a suite of Workstation Configuration programs for SDDM, Ghostty, Kitty, and Alacritty. These have been integrated into the C-Menu example main menu. I will continue to add more exercises along with documentation. The exercises will be designed to be both educational and practical, and they will cover a wide range of topics related to C-Menu development. One of the next projects, if time allows, will be a C-Menu based general ledger using SQLite as the backend database.

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
