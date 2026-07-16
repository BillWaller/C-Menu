# CHANGELOG

## C-Menu-0.2.9

*2026-07-15T21:05:46-05:00* - FEATURE: view: smart line wrapping. The testing and debugging for this update is incomplete, but the feature is somewhat functional. By starting View with the "-w" option, or entering "-w" from a View command prompt, the user can enable smart line wrapping. See the README.md for a comparison of View with other popular text viewers. C-Menu's View is the perfect tool for examining large log files with long lines. Where other pagers just chop the lines in at the right edge, C-Menu's View intelligently wraps lines at word boundaries rendering 1000+ byte lines in a readable format. The user can also toggle the smart line wrapping feature on and off while viewing a file. The difference in readability is dramatic. Check it out. In the meantime, I will continue to test and debug in preparation for the next release. 

*2026-07-12T21:40:40-05:00* - Update CHANGELOG.md 

*2026-07-12T21:39:32-05:00* - FEATURE: Line wrapping is not quite ready yet, but I wanted to get this in before the next release. 

*2026-07-11T19:19:07-05:00* - Update CHANGELOG.md 

*2026-07-11T19:07:45-05:00* - FEATURE: Adding a toggle option for line wrapping in the view engine. The code that splits long lines into multiple lines is already present in the view engine, but it is not complete. When it is, I will add a toggle option to enable or disable line wrapping. 

*2026-07-07T23:11:06-05:00* - Update CHANGELOG.md 

*2026-07-07T23:06:39-05:00* - FIX: dwin.c: make Perror() window width large enough to accommodate chyron. 
/menuapp/themes: Adjust theme colors to be more readable. 

*2026-07-07T18:09:46-05:00* - Update CHANGELOG.md 

*2026-07-07T18:09:14-05:00* - DOCS: Documentation Updates 

*2026-07-07T17:13:03-05:00* - Update CHANGELOG.md 

*2026-07-07T17:12:41-05:00* - FIX: Update themes and scripts in menuapp 

*2026-07-07T17:09:57-05:00* - Update CHANGELOG.md 

*2026-07-07T17:09:23-05:00* - FIX: Insert code to read theme file in init.c 

*2026-07-07T16:48:45-05:00* - Update CHANGELOG.md 

*2026-07-07T16:47:00-05:00* - FIX: CMakeLists.txt: added UAL_UI components to CMakeLists.txt and tested build. Executables and libraries built successfully. 

*2026-07-07T11:53:42-05:00* - Update CHANGELOG.md 

*2026-07-07T11:51:19-05:00* - FIXES: The width of the pad for full-screen mode was set to COLS instead of PAD_COLS, which truncated lines as pmincol was shifted to the right in horizontal scrolling. Fixed. 

*2026-07-07T10:23:15-05:00* - Update CHANGELOG.md 

*2026-07-07T10:22:32-05:00* - FIX: pick_engine.c: hide page down on last page 

*2026-07-07T09:18:20-05:00* - Update CHANGELOG.md 

*2026-07-07T09:15:44-05:00* - FIX: Added clrtoeol after displaying the input field in pick_engine.c search line. 

*2026-07-06T19:54:09-05:00* - Update CHANGELOG.md 

*2026-07-06T19:53:52-05:00* - DOCS: Documentation Updates 

*2026-07-06T19:51:04-05:00* - Update CHANGELOG.md 

*2026-07-06T19:50:42-05:00* - DOCS: Documentation Updates 

*2026-07-06T19:46:06-05:00* - Update CHANGELOG.md 

*2026-07-06T19:27:02-05:00* - FIXES: At this point, we are examining the code and resolving as many issues as possible while laying the groundwork for the next phase of development. We resolved several issues in this update related to the pick and view engines, as well as the initialization process. We are continuously improving screen handling and aesthetics to make C-Menu more intuitive and user-friendly. 

*2026-07-05T22:39:50-05:00* - Update CHANGELOG.md 

*2026-07-05T22:39:12-05:00* - FIX: view_engine.c: Continuing refinements of command line editor. 

*2026-07-05T21:02:24-05:00* - Update CHANGELOG.md 

*2026-07-05T21:01:15-05:00* - FIX: This is the clean-up of the second form of input used on the view command line. 

*2026-07-05T15:28:25-05:00* - Update CHANGELOG.md 

*2026-07-05T15:06:14-05:00* - FIX: View's command line editor takes two kinds of input. The first is an optional numeric argument that precedes single-character commands. For example, to scroll 5 spaces to the right, you can type "5l" in the command line. The second is a command argument that follows a command. For example, to search for the string "foo", you can type "/foo" in the command line. We are shoring up the command line editor to handle arguments more robustly. We also added logic to elide sections of the prompt string that are not relevant to the current command, or would produce a prompt string that is too long to fit in the command line. 

*2026-07-04T19:03:16-05:00* - Update CHANGELOG.md 

*2026-07-04T19:02:47-05:00* - DOCS: Documentation Updates 

*2026-07-04T16:54:34-05:00* - Update CHANGELOG.md 

*2026-07-04T16:19:48-05:00* - FIXES: Several issues with derived windows, panels, and pads in view_engine.c and init_view.c. The refactoring of C-Menu in preparation for the new Uniform Abstraction Layer User Interface (UAL_UI) turned out to be more disruptive than expected. Nevertheless, the refactoring continues to bring C-Menu closer to the UAL_UI design objectives. Scalability, resilience to change, and maintainability are improving as well. Z-stack management with derived windows, panels, and pads is more robust, paving the way for cellular grapheme clusters, multi-media integration, and plug-in interfaces. 

*2026-07-03T20:34:08-05:00* - Update CHANGELOG.md 

*2026-07-03T19:59:28-05:00* - FIXES: Several issues with the view engine fixed, but a few more remain. We have added many new and powerful features in the last few months, including highlighter integration, Neovim code completion, Unicode support, pads, panels, an async worker queue thread pool, direct kernel virtual memory file mapping, live file preview with View in Pick, and more, but the next wave will be even more exciting. We have only scratched the surface of what is possible with the new view engine. but The next wave will be even more exciting, with a Uniform Abstraction Layer User Interface (UAL_UI). That will allow us to provide Notcurses as a UI backend, and that will give us a bridge to other platforms including Windows, and MacOS. However, we should probably get to release 1.0.0 first, and that means we need to fix the remaining bugs and issues, and we need to make sure that the new features are stable and well-tested. We also need to improve the documentation and provide more examples and tutorials for users. That's an invitation to all users to help us test and report any issues they find, and to contribute to the project if they can. We welcome all contributions, big or small, and we appreciate any help we can get. 

*2026-07-02T22:30:43-05:00* - Update CHANGELOG.md 

*2026-07-02T22:07:56-05:00* - REFACTOR: Preparation for Uniform Abstraction Layer User Interface. Added cm_fields.c, which will eventually replace fields.c. cm_fields.c will be a simpler, more streamlined, and yet functionally equivalent replacement for fields.c. fields.c works well, but it turned out to be overly complicated and laborious to maintain. 

*2026-07-02T10:25:51-05:00* - Update CHANGELOG.md 

*2026-07-02T10:23:24-05:00* - FIXES: init_view.c: fixed issues in view when resizing the terminal window 

*2026-06-29T22:40:47-05:00* - Update CHANGELOG.md 

*2026-06-29T22:40:10-05:00* - FIX: Cosmetic adjustments to View and Pick engines. 

*2026-06-29T21:28:37-05:00* - DOCS: Documentation Updates 

*2026-06-29T14:55:04-05:00* - Update CHANGELOG.md 

*2026-06-29T14:32:08-05:00* - FEATURE: pick_engine.c, dwin.c: Added an activation flag to the chyron structure. This makes it easier for C-Menu to display content that is relevant to the context of the application. 

*2026-06-29T07:42:30-05:00* - Update CHANGELOG.md 

*2026-06-29T07:39:38-05:00* - FIX: pick_engine.c: corrected handling of multiple object selections 

*2026-06-28T16:04:39-05:00* - Update CHANGELOG.md 

*2026-06-28T16:01:51-05:00* - FIX: pick_engine.c: After entering a search expression and returning to the selection window, and then returning to the search window, the previous position pointer was initialized causing the field editor to get confused. Fixed that. 

*2026-06-28T15:56:02-05:00* - Update CHANGELOG.md 

*2026-06-28T15:49:52-05:00* - FIXES: The box window was too small for the picker window, which caused scroll to overwrite the search window and part of the border. Internally C-Menu uses ratios in percentages to calculate window sizes, and one of the calculations was incorrect. 

*2026-06-28T12:26:23-05:00* - Update CHANGELOG.md 

*2026-06-28T12:21:55-05:00* - FIXES: Corrected issues in src/fields.c and src/form_engine.c, which were causing a number of problems. Also resolved some issues that had surfaced in valgrind. If you are wondering why valgrind includes the ncurses references, here is the answer:  To prevent a subprogram from interacting with the NCurses instance in the current process, it is idiomatic to call endwin(). By design, NCurses retains nominal state information to facilitate efficient reactivation. This is normal behavior for NCursesw and is not a memory leak. 

*2026-06-27T16:55:31-05:00* - Update CHANGELOG.md 

*2026-06-27T16:38:31-05:00* - FIXES: Many various and sundry fixes to the codebase. The recent addition of Ncurses Panels followed immediately by integrating framework for the Uniform Abstraction Layer (UAL) has caused a number of issues to crop up in the codebase. This commit addresses those issues and improves the overall stability and functionality of the application. The UAL will provide plug-in support for many new and diverse capabilities. 

*2026-06-27T00:47:47-05:00* - Update CHANGELOG.md 

*2026-06-27T00:36:07-05:00* - FIXES: The refactoring to accommodate the new Uniform Abstraction Layer User Interface (UAL_UI) stressed the C-Menu code base, resulting in a number of issues. After a long day of debugging, the major issues have been resolved. Among the issues were three definitely lost memory blocks. There are still 64 bytes dangling, but not lost. They are NCurses panels, which may not be freed until the end program. 

*2026-06-25T17:26:17-05:00* - DOCS: Update Changelog 

*2026-06-25T17:14:56-05:00* - REFACTOR: Many changes today as we back the Uniform Abstraction Layer User Interface (UAL_UI) into the code base. This code isn't supposed to do anything new yet. The objective is to provide a uniform interface through which C-Menu can communicate with an abstracted UI backend. The UAL_UI is a work in progress, and the current implementation is not complete. The goal is to eventually allow C-Menu to support multiple UI backends, such as ncurses, notcurses, and potentially others. The changes made today include - Adding new files for the notcurses backend, including example.c, ui_draw_with_channels.c, and ui_mvaddcell.c. - Modifying existing files to integrate the UAL_UI # Please enter the commit message for your changes. Lines starting 

*2026-06-24T00:39:27-05:00* - Update CHANGELOG.md 

*2026-06-24T00:12:32-05:00* - FEATURE: menuapp/msrc/main.m: Rustlings Source: Added "-S" (sort) option to lf command. 

*2026-06-23T12:33:00-05:00* - FIX: lf somehow regressed to a previous version in which hidden directories were not being traversed unless the "f_include_hidden" option was set to true. This commit fixes that regression and ensures that hidden directories are always traversed, regardless of the "f_include_hidden" option. 

*2026-06-23T11:12:17-05:00* - CLEANUP: in anticipation of UALUI integration with the new UI engine. This commit includes modifications to various source files, including curskeys.c, dwin.c, fields.c, form_engine.c, cm.h, init_view.c, menu.c, menu_engine.c, pick_engine.c, ui_ncurses.c, ui_ncurses_internal.h, and view_engine.c. The changes aim to improve the overall functionality and user experience of the application. 

*2026-06-23T09:08:06-05:00* - FIX: dwin.c someone inadvertently removed a line of code, getmaxyx(), causing core dump on start-up. Fixed that. 

*2026-06-22T21:24:47-05:00* - FIX: Ncurses documentation states that libpanelw should be placed before libncursesw in the link order. This commit updates the Makefile to reflect this change, ensuring that libpanelw is linked before libncursesw. This adjustment is crucial for proper functionality and compatibility with the ncurses library. 

*2026-06-22T21:13:04-05:00* - DOCS: CHANGELOG.md updated with new release notes. 

*2026-06-22T20:02:27-05:00* - FIX: view_engine.c: pad_refresh was placed before update_panels/doupdate, so NCurses was correctly updating pad, and then the panels, which overwrote the pad. Fixed that. 

*2026-06-21T23:34:48-05:00* - FIX: Fix a few memory leaks and some minor bugs after adding the UALUI code, but a 32 byte win_box panel is definitely lost. I am still tracking it down, but C-Menu seems to be functioning properly. A valgrind output file has been saved in the C-Menu/src directory. 

*2026-06-20T18:25:44-05:00* - REFACTOR: UI_NCURSES isolation and Uniform Abstraction Layer User Interface (UI_BACKEND) Implementation 

*2026-06-20T13:42:23-05:00* - DOCS: Update README.md 

*2026-06-20T11:39:49-05:00* - DOCS: Update CHANGELOG.md 

*2026-06-20T11:25:44-05:00* - FIX BUG: Replaced internal border function with NCurses setborder(), but that won't work for Pick because it has a split-window, so this fix reverts to the internal border function for Pick. The idea behind abandoning internal code for setborder() is to standardize the code and reduce maintenance. The internal border function for split-windows works fine, so it will be retained for Pick. 

*2026-06-19T23:55:27-05:00* - FEATURE: Update README and CHANGELOG 

*2026-06-19T23:40:50-05:00* - FEATURE: Yes, something is underway. There should be no functional changes at this point. This is a refactor, and the first step is to move all of the ncurses-specific code into a separate directory. This will allow us to eventually support multiple backends, and to cleanly separate the concerns of the UI from the core logic of the application. The next step will be to abstract the UI layer, so that we can have a clean interface between the core logic and the UI. This will allow us to more easily support multiple backends, and to make the code more modular and maintainable. The final step will be to implement a new backend, perhaps using a modern graphics library, to provide a more visually appealing and responsive user interface. This will be a significant undertaking, but it will ultimately result in a much better user experience and a more maintainable codebase. In the meantime, we will continue to support the existing ncurses backend, and we will work to ensure that the refactor does not introduce any regressions or break any existing functionality. This is an exciting time for the project, and I look forward to seeing the improvements that will come from this refactor. Thank you to everyone who has contributed to the project so far, and I encourage everyone to continue contributing and providing feedback as we move forward with this refactor. Together, we can make this project even better and more enjoyable for everyone who uses it. Let's keep up the great work and continue to push the boundaries of what we can achieve with this project. Thank you again for your support and contributions, and I look forward to seeing the amazing things that we will accomplish together in the future. Let's keep up the momentum and continue to make this project the best it can be. Thank you all for your hard work and dedication, and let's keep pushing forward with this refactor and the exciting improvements that it will bring to the project. Together, we can make this project even better and more enjoyable for everyone who uses it. Let's keep up the great work and continue to push the boundaries of what we can achieve with this project. Thank you again for your support and contributions, and I look forward to seeing the amazing things that we will accomplish together in the future. Let's keep up the momentum and continue to make this project the best it can be. 

*2026-06-19T17:40:23-05:00* - Docs: Update CHANGELOG.md 

*2026-06-19T17:32:53-05:00* - [ui] Add ncurses UI backend 

*2026-06-19T12:30:07-05:00* - Doc - CHANGELOG 

*2026-06-19T12:27:51-05:00* - Doc: Updated CHANGELOG.md 

*2026-06-19T08:46:53-05:00* - view_engine.c: Correct error in panel stack updates. 

*2026-06-18T22:25:54-05:00* - The panels feature of NCurses has slightly different requirements for refreshing and updating pads. It is not difficult, but anywhere it is not handled, a blank pad view will be displayed. I will try to catch all of those occurrences and apply fixes. 

*2026-06-18T14:56:13-05:00* - After search, View was not displaying its pad_view window, so I added prefresh pad, update_panels, and doupdate() to search function. This assures synchronization between the physical screen and the virtual screen after updating the panels and pad. 

*2026-06-18T00:04:04-05:00* - src/lf.c (lightweight find): lf's "-Ho" (hidden files only) option caused directories that were not hidden to be rejected before placing them on the work queue. This meant that lf would not descend into those directories, and thus would not find any hidden files within them. This commit changes the order of the checks so that directories are added to the work queue before checking if they are hidden. This allows lf to descend into all directories, but only display hidden files when the "-Ho" option is used. 

*2026-06-17T15:41:15-05:00* - Documentation Updates 

*2026-06-17T14:19:26-05:00* - Documentation Updates 

*2026-06-17T13:31:08-05:00* - src/lf.c (lightweight file finder): added option "-Ho" to include only hidden files. 

*2026-06-17T13:00:39-05:00* - Documentation Updates 

*2026-06-17T12:34:44-05:00* - Finally, the recent window management upgrades are coming together. The codebase has been refactored from the old 2 plane (y and x) windowing system that was becoming increasingly difficult to maintain. NCurses panels, along with derived windows and pads not only provides a z axis stack, but does much of the heavy lifting for window management, including handling of overlapping windows, input focus, and efficient redrawing. This refactor has allowed for a significant reduction in code complexity, as well as improved performance and maintainability. The new system is more modular, making it easier to add new features and fix bugs in the future. Overall, this refactor has been a major step forward in improving the window management capabilities of the application, and it sets a solid foundation for future enhancements. 

*2026-06-17T00:03:28-05:00* - Continuing bug fixes related to refactoring window management. 

*2026-06-16T17:43:34-05:00* - Finally, the flickering is gone. I do know why it was happening, but that was among the easiest things to fix. The difficult part is becoming familiar with a whole new way managing windows with a hierarchy of panels and derived windows. It is clear to me that the old way I was managing windows was becoming unwieldy and C-Menu must be able to scale to achieve its ambitions. I apologize for the bugs, and I expect there will be more as I go through the codebase and refactor it to use panels. I have been testing the changes as I go, but I have not been able to test every possible combination of windows and panels. I have been testing the changes on Linux, and I expect there will be some issues on Windows and MacOS. I will do my best to fix any issues that arise, but I would appreciate any help from the community in testing and reporting bugs. 

*2026-06-15T23:42:56-05:00* - Documentation Update 

*2026-06-15T23:37:06-05:00* - Beware: There are still bugs in this release as the most recent feature additions involved a complete rewrite of the window management system. Please report any bugs you find to the issue tracker BillWaller/C-Menu on GitHub. If you have any questions about the new features, please ask in the discussions section of the same repository. Thank you for your support and happy menuing! 

*2026-06-15T20:40:13-05:00* - Documentation Updates 

*2026-06-15T20:34:06-05:00* - Combined Pick and View to create a File Browser. 

*2026-06-13T22:25:47-05:00* - Documentation Updates 

*2026-06-13T22:22:42-05:00* - Documentation Updates 

*2026-06-13T22:20:13-05:00* - Documentation Updates 

*2026-06-13T22:18:08-05:00* - Documentation Updates 

*2026-06-13T22:12:31-05:00* - Documentation Updates 

*2026-06-13T22:00:31-05:00* - Documentation updates 

*2026-06-13T19:38:06-05:00* - Minor bug in pick_engine.c fixed. 

*2026-06-13T17:05:11-05:00* - Mini bug in Build documentation. 

*2026-06-13T17:01:18-05:00* - Synchronized the build instructions in the documentation with the actual build system. Updated the CMakeLists.txt and Makefiles to reflect the changes in the build process. Removed the outdated README.md file from the docs directory as it is no longer relevant. 

*2026-06-13T15:49:48-05:00* - Several bug fixes related to panels upgrade. 

*2026-06-12T22:23:18-05:00* - Documentation Updates 

*2026-06-12T21:41:53-05:00* - The latest tranche of changes to As C-Menu is likely to be the most significant yet, in terms of what it will bring to C-Menu in the form of scalability as it gears up for tackling more sophisticated applications. The changes include reorganizing the window structure with NCurses Panels to make it much easier to introduce new features while improving resilience and maintainability. As C-Menu grew to a staggering 10,000 lines of code, it became clear that the original window structure was too rigid and made it difficult to implement new features without breaking existing functionality. The new window structure allows for more modular and reusable code, making it easier to add new features and maintain the codebase. Additionally, the new structure allows for better performance and scalability, as it can handle larger and more complex applications without sacrificing performance. Overall, these changes are a significant step forward for As C-Menu as it continues to grow and evolve as a powerful and flexible application development toolkit. 

*2026-06-12T01:11:44-05:00* - lf had an error that caused sort to fail. 

*2026-06-11T18:03:14-05:00* - Brought both Makefiles to synch, and removed the old build scripts. Both Makefile and CMakeLists.txt now support the same targets, and the Makefile is now a GNU Makefile, which is more portable and has better support for parallel builds. The old build scripts were redundant and not well maintained, so they have been removed to simplify the build process. The .gitignore file has been updated to ignore the new build artifacts generated by the Makefile and CMakeLists.txt, such as the compile_commands.json and install_manifest.txt files. This should help keep the repository clean and prevent unnecessary files from being committed. Overall, these changes should improve the build process and make it easier for developers to build and maintain the project. 

*2026-06-11T16:06:06-05:00* - Modified the Makefile (GNU) to produce 3 binaries for rsh. "rsh" is dynamically linked, "rsh_static" is statically linked, and "rsh_pam" is dynamically linked with PAM support. The reason for doing so is that you have different requirements depending on the use case, and many developers and administrators have a variety of use cases. For rescue environments, a statically linked binary is often preferred because it does not rely on shared libraries that may not be available. For regular use, a dynamically linked binary is more common and can take advantage of shared libraries for better performance and smaller size. For environments that require PAM support, having a separate binary allows users to choose the appropriate version based on their needs without having to recompile the entire project. This approach provides flexibility and caters to a wider range of use cases while maintaining the integrity of the project. It gives you a selection of tools so you always have the right one for the job. 

*2026-06-11T15:17:33-05:00* - Several bug fixes and improvements to the build system, including: Brought the C-Menu build up to date, adding conditional builds for a static executable and PAM support. To build static rsh with CMake, cd to src, and type: cmake -DCMAKE_BUILD_TYPE=Release -DRSH_STATIC=ON . To build rsh with PAM support, cd to src, and type: cmake -DCMAKE_BUILD_TYPE=Release -DRSH_PAM=ON . Then make and make install as usual. Static rsh and rsh with PAM support have vastly different use cases, and it doesn't make sense to build PAM support into a static rsh binary, so these options are mutually exclusive. We may add a CMake option to build both types of executable in a single build in the future, but with different names. The static binary is 3.3 MB compared to 20 KB for dynamic. 

*2026-06-10T15:54:58-05:00* - Modified view_engine.c to allow clipping text with mouse. Added horizontal scroll default shift width, which can be overridden by entering a shift width before right or left arrow keys. Shift width sticky and doesn't change until you enter a different shift width before pressing left or right arrow keys. 

*2026-06-09T22:02:15-05:00* - Bug fixes and Documentation Updates 

*2026-06-09T15:32:35-05:00* - Documentation Updates 

*2026-06-09T15:01:34-05:00* - Documentation Updates 

*2026-06-09T14:48:11-05:00* - Documentation Updates 

*2026-06-09T14:09:44-05:00* - Testing, catching edge cases and improving the overall user experience. This includes refining the color schemes, enhancing the configuration options, and ensuring better compatibility across different environments. The new theme script allows users to easily create and apply custom themes, while the updated documentation provides clearer guidance on how to utilize these features effectively. Additionally, various bug fixes and performance optimizations have been implemented to ensure a smoother and more responsive interface. 

*2026-06-09T12:32:26-05:00* - Documentation Updates 

*2026-06-09T12:22:03-05:00* - Documentation Updates 

*2026-06-09T10:23:29-05:00* - In previous testing, I had commented out the code that restored ncurses windows after executing a command. Therefore, the windows were not being restored after executing a command. Fixed that. My wife said the problem is the loose nut on the keyboard. 

*2026-06-08T21:34:49-05:00* - Documentation Updates 

*2026-06-08T21:22:30-05:00* - Documentation Updates 

*2026-06-08T21:21:38-05:00* - Documentation Updates 

*2026-06-08T19:35:54-05:00* - Documentation Upodates 

*2026-06-08T19:23:31-05:00* - Added code to load and apply themes instantly without restarting the application. This includes changes to the Makefile, detach, dwin.c, futil.c, include/cm.h, include/common.h, include/pick.h, init.c, mem.c, and pick_engine.c. Additionally, updated the README.md and themes.png to reflect the new theme functionality. 

*2026-06-08T11:25:23-05:00* - Added detach command to the Makefile, which allows the user to detach a process from the terminal and run it in the background. The detach command is implemented in the src/detach.c file, which uses the fork() and setsid() system calls to create a new process that is detached from the terminal. 

*2026-06-08T11:04:46-05:00* - Documentation Updates, bug fixes. 

*2026-06-07T20:37:51-05:00* - Modified init.c to load themes from the themes directory. Added a new script to list themes and another to change themes. Added some themes. Modified init.c to include descriptive comments from argp when writing configuration files. 

*2026-06-06T23:55:43-05:00* - Move C-Menu/src/test to C-Menu/src/work 

*2026-06-06T20:56:32-05:00* - Themes 

*2026-06-06T20:31:06-05:00* - Themes 

*2026-06-06T19:32:00-05:00* - Added selection to the C-Menu example main menu, which will start an independent instance of C-Menu in a new terminal window. This is an example of the flexibility of C-Menu, and how it can be used to create multiple menus that can be launched from each other. The new menu is a copy of the main menu, but it can be modified independently to show different options or information. This demonstrates how C-Menu can be used to create complex menu systems with multiple levels and branches, allowing for a more dynamic and interactive user experience. The changes include modifications to the configuration files for the Green, Red, and default themes, as well as changes to the main.m file to add the new menu option and launch the new menu in a new terminal window. Overall, this update enhances the functionality and versatility of the C-Menu example, making it a more powerful tool for creating custom menu interfaces. 

*2026-06-06T18:17:22-05:00* - Modified config file (
/menuapp/.minitrc) processing to accommodate comments on configuration lines. This allows users to add comments to their configuration files without breaking the parsing logic. The changes include updating the parsing function to ignore any text following a comment character (e.g., '#') on a line, ensuring that only the actual configuration settings are processed. This enhancement improves the usability of the configuration files by allowing users to document their settings directly within the file. 

*2026-06-06T14:26:49-05:00* - Added menu selection to select C-Menu theme. Corrected an error in the C-Menu Kitty theme selector. Added Selections Processed message to Pick. 

*2026-06-06T01:28:57-05:00* - After adding so many new features in the past month, I expected lots of bugs, but it hasn't been as bad as I thought. And the bugs are getting easier to fix as the codebase nears the end of the 0.2.9 cycle. I have a few more features to add, and I think the next release will be a good one. The new themes are really nice, and I think users will like them. I did find a significant error in lf file type handling, but that is fixed in this commit. 

*2026-06-05T16:09:14-05:00* - Added new colors for the themes, added a new theme, and made some minor adjustments to the code to make it more efficient and easier to read. Also updated the README.md file to reflect the changes made to the themes and the new features added. 

*2026-06-05T02:53:32-05:00* - menuapp updates 

*2026-06-05T02:50:31-05:00* - Added color, brackets_fg. 

*2026-06-05T02:13:08-05:00* - Makefile and documentation update 

*2026-06-04T16:47:02-05:00* - Preview of RSH certificate screenshot added to README.md 

*2026-06-04T16:35:17-05:00* - General cleanup and update of the C-Menu example application directory. 

*2026-06-04T16:26:41-05:00* - Makefile added help, config, and manifest. Added $CMENU_HOME/lib64 to RPATH. RPATH is very useful in a development environment where you may have several versions of a library installed. As C-Menu becomes more mature, we will likely remove the RPATH and rely on the user to set their LD_LIBRARY_PATH. But for now, this makes it easier to test new versions of the library without having to worry about the system version of the library. 

*2026-06-04T14:01:35-05:00* - Added optional manifest display at end of Makefile. Activate by setting DISPLAY_MANIFEST to 1 in the Makefile. Deactivate by setting it to 0. This is useful information for debugging and development, but has no effect on the final product. It is not intended for end users, and may be removed in future versions. 

*2026-06-04T11:01:58-05:00* - Corrected error in parse_menu_desc.c so that choice letters designated with "-" are properly reserved, preventing some letters from occurring more than once in a menu. Also updated cmenu manual page to explain the handling of input and output pipes in direct execution mode. 

*2026-06-04T00:25:45-05:00* - Substituted native linux install command for the inst.sh script in  the Makefile in the interest of standardization and convenience of developers who are familiar with the native linux install command. 

*2026-06-03T16:10:14-05:00* - Added color definition for Form's fill character, fill_char_fg. Fixed artifact from previous commit that caused Form fields not to update properly. 

*2026-06-03T10:55:46-05:00* - Documentation Updates 

*2026-06-03T09:12:46-05:00* - Add manual page for rsh. 

*2026-06-03T09:09:17-05:00* - Documentation Updates 

*2026-06-02T20:35:44-05:00* - Documentation Updates, Unicode and wide character processing in form. 

*2026-06-01T14:17:30-05:00* - Added a new menu command type, "dexe", which detaches from the current process and executes a command. This allows for running simultaneous concurrent processes without blocking the menu system. The implementation involves changes to the command execution logic, menu description parsing, and the menu engine to support this new command type. 

*2026-06-01T10:59:47-05:00* - Documentation Updates 

*2026-06-01T10:53:26-05:00* - lf manual page 

*2026-06-01T10:42:32-05:00* - Add lf manual link to README.md 

*2026-06-01T10:36:20-05:00* - Remove spurious line in lf. 

*2026-06-01T10:23:57-05:00* - Added lf manual page, updated lf help files, screenshots, and the Makefile to include a section to build and install manual pages. 

*2026-05-31T13:22:18-05:00* - Documentation Updates 

*2026-05-31T13:18:20-05:00* - Documentation Updates 

*2026-05-31T00:32:22-05:00* - Documentation Updates 

*2026-05-30T23:05:58-05:00* - Documentation Updates 

*2026-05-30T21:17:21-05:00* - Documentation Updates 

*2026-05-30T18:26:11-05:00* - Various and sundry cleanup and fixes. 

*2026-05-30T14:04:02-05:00* - Documentation Updates 

*2026-05-30T13:35:07-05:00* - Corrected bug in Form command processing 

*2026-05-29T23:46:22-05:00* - Documentation Updates 

*2026-05-29T20:52:04-05:00* - Documentation Updates 

*2026-05-29T20:17:31-05:00* - Menu application directory general cleanup. Added Main Menu selection to edit menu description files. 

*2026-05-29T15:38:44-05:00* - Cleaned up lf "-D1" output. 

*2026-05-28T22:36:28-05:00* - Documentation Updates 

*2026-05-28T22:21:42-05:00* - Documentation Updates 

*2026-05-28T18:25:20-05:00* - Testing the new help system and adding a few screenshots to the docs. 

*2026-05-28T00:39:26-05:00* - Documentation Updates 

*2026-05-28T00:12:12-05:00* - Applied changes to test program C-Menu/test/iso8601.c. 

*2026-05-27T23:50:23-05:00* - Documentation Updates 

*2026-05-27T21:45:54-05:00* - Documentation Updates 

*2026-05-27T21:19:03-05:00* - Resolved lf date/time issues with Co-Pilot's help. The date/times at issue were the after and before date/times defining the range of files to be included in lf's output. I discovered this issue using the "-D18" option, which reveals the date/time information captured from the command line. The option processing code captured the date/times entered, but the verification date/times printed in init_find were off by by an hour for date/times in the CDT domain. The problem was that tm1_isdst must be initialized to -1 before calling mktime, which is used to convert the date/time information into a time_t value. By setting tm1_isdst to -1, mktime can correctly determine whether daylight saving time is in effect for the given date/time, which resolves the issue with the date/time being off by an hour. After making this change, the date is now correctly formatted and displayed in the application. This fix ensures that users will see the correct date information without any formatting errors. 

*2026-05-27T19:56:59-05:00* - Documentation Updates 

*2026-05-27T19:51:29-05:00* - Documentation Updates 

*2026-05-27T19:48:01-05:00* - Bug fixes and documentation updates. 

*2026-05-27T11:28:01-05:00* - I suspended organized testing during the last tranche of feature additions and improvements in the interest of expedient completion. As a result, I have a backlog of testing. I am adding more test cases to ensure that the code is robust and can handle a wide variety of operating conditions. This commit fixes several issues exposed by examining the output of lf using the "-D18" option to verify option processing. I will continue to test and fix any bugs that I find, and add more test cases to ensure that the code is robust and can handle a wide variety of inputs and scenarios. I am committed to ensuring that the code is of high quality and meets the needs of users, and I will continue to work diligently to achieve that goal. Thank you for your patience and support as I continue to improve the code and add more features. 

*2026-05-26T17:33:04-05:00* - Clean-up following scan-build report. lf: Corrected thread number calculation. 

*2026-05-26T12:56:26-05:00* - Switching back to alternate screen buffer after running nvim as a subprogram. This is to deal with screen buffer corruption when exiting C-Menu after running nvim as a subprogram. 

*2026-05-26T11:55:01-05:00* - lf: fixed error in argument processing, which could cause regular expression failure and SIGSEGV (core dumped) 

*2026-05-25T22:30:18-05:00* - Update C-Menu application directory 

*2026-05-25T21:42:45-05:00* - lf: corrected issues with exclusion regex leading to SIGSEGV 

*2026-05-25T17:49:37-05:00* - Documentation Updates 

*2026-05-25T16:04:43-05:00* - Menu: Due to some menu's running out of key characters that make sense, I have removed the restriction on lower case letters and some special characters. The characters  "!" through "
" (0x21 - 0x7e) are now valid key characters. Menu uses a rudamentary algorithm to determine the key character for a menu item. It scans the menu text for the first character that is not a space, not already reserved, and in the valid key character range. If no such character is found, Menu will select the first unreserved letter in the valid range, even though it may not be in the menu text. You may specify any character by including it immediately (no space separation) after a dash ("-") as the first non-blank character in the the menu item text. Lower case ("q") is always reserved for the hidden "Quit", "Exit", "Return", etc. menu item. As an additional visual queue, key characters in the menu text will be displayed using "nt_hl_rev_fg" and "nt_hl_rev_bg" color pair. These colors are defined as six-digit hex RGB values in the C-Menu configuration (normally 
/menuapp/.minitrc). 

*2026-05-25T10:27:08-05:00* - Added color definitions to specify reverse and highlight for Menu items. These are nt_fg, nt_bg, nt_hl_fg, nt_hl_bg, nt_hl_rev_fg, nt_hl_rev_bg, where nt is for normal text, hl is highlighted, rev is reverse, and fg for background and bg for background. 

*2026-05-22T13:44:36-05:00* - lf: Continuing optimizations and enhancements 

*2026-05-20T20:24:01-05:00* - lf: Continuing performance enhancements and adding tests 

*2026-05-19T23:41:23-05:00* - lf: added several features and performance enhancements 

*2026-05-19T02:04:51-05:00* - lf: continuing performance improvements, added features 

*2026-05-17T20:01:19-05:00* - Documentation Updates 

*2026-05-17T18:54:22-05:00* - Experimental feature additions and optimizations. 

*2026-05-16T14:36:38-05:00* - Documentation Updates 

*2026-05-16T14:04:47-05:00* - CHANGELOG updates 

*2026-05-16T13:45:35-05:00* - lf: Attempts to integrate mmap into lf resulted in degraded performanc. As a result, the mmap implementation has been removed and the previous file handling method has been restored. This change aims to improve the overall performance of lf while maintaining its functionality. 

*2026-05-14T23:01:43-05:00* - Documentation Update 

*2026-05-14T22:51:11-05:00* - lf performance tuning 

*2026-05-14T14:36:56-05:00* - lf corrected conditionals grouping errors in file type deselector 

*2026-05-13T23:25:03-05:00* - Further lf optimizations and Documentation updates 

*2026-05-13T01:34:28-05:00* - lf performance tuning 

*2026-05-12T13:22:03-05:00* - Documentation Updates 

*2026-05-12T13:01:06-05:00* - lf performance tuning. Set default number of threads to 6 if available. Added -T option to specify number of threads. Using more threads isn't always better, and can actually decrease performance, I have 16 threads available, but testing showed that 6 threads gave the best performance. Use the time command and pay most attention to real time, not user or system time. You can specify the number of threads you want to use with the -T option if you want to experiment with it. lf checks the number of threads available on the system on the system and limits the user setting accordingly. lf's -D (debug) option displays the number of threads being used. 

*2026-05-12T01:09:47-05:00* - lf: include directory names by default 

*2026-05-12T01:07:11-05:00* - lf: corrected error in finder loop 

*2026-05-11T23:51:01-05:00* - Documentation Updates 

*2026-05-11T01:02:51-05:00* - Documentation Updates 

*2026-05-11T00:51:37-05:00* - lf default was to follow symbolic links to directories. New default is to not follow symbolic links to directories. The -L option has been added to follow symbolic links to directories. 

*2026-05-10T23:45:39-05:00* - Documentation Updates 

*2026-05-10T17:28:09-05:00* - lf: Changed default to exclude hidden files, removed -n option to exclude hidden files, and added the -H option to include hidden files. Updated the help file and screenshot accordingly. 

*2026-05-10T11:44:15-05:00* - lf: tidying up the coded added for concurrent directory processing. Further benchmark testing indicates the concurrent directory processing provides the expected performance gains, and in all cases so far, C-Menu's lf finder is outperforming fd and find, and in some cases significantly so. 

*2026-05-10T00:13:48-05:00* - Added concurrent directory processing to lf using a thread pool and a worker to manage thread queues. Threads are mutex protected to ensure safe access to shared resources. The implementation includes a thread pool for efficient task management and a worker to handle the distribution of tasks among threads. This enhancement allows for faster processing of directories by utilizing multiple threads concurrently, improving the overall performance of the lf application. The changes include modifications to the Makefile, the addition of thread management code in futil.c, and updates to the header file cm.h to support the new threading functionality. The documentation has also been updated to reflect these changes. 

*2026-05-08T08:27:52-05:00* - Documentation Updates 

*2026-05-08T00:31:43-05:00* - Documentation Updates 

*2026-05-07T17:43:37-05:00* - Documentation Updates 

*2026-05-07T15:05:15-05:00* - Identified a logic error in lf that was causing it to recurse into lf_process for regular files. This induced quite a performance penalty on lf because lf_process was being called for all files, regardless of whether they were directories or not. The fix was to rearrange the comparisons of the controlling if statement. This prevents the unnecessary recursion and allows lf to function correctly when processing regular files. 

*2026-05-07T00:01:50-05:00* - Documentation Updates 

*2026-05-06T23:59:56-05:00* - Documentation Updates 

*2026-05-06T23:33:43-05:00* - Added -S (sort) option to lf. -Sr sorts in reverse order. 

*2026-05-06T12:22:08-05:00* - Documentation Updates and minor bug fixes 

*2026-05-05T18:39:49-05:00* - amort, the executable receiver for the Form example only had the format yyyy-mm-dd specified in strptime, and the default output of Form is unformatted without the "-". As a reault the amortization View output had incorrect dates. Added a second format, Yyyymmdd, to make it more robust and it's working now. 

*2026-05-05T18:19:27-05:00* - Documentation Updates 

*2026-05-05T18:06:35-05:00* - Reworked form_engine exec_receiver_cmd to make it more robust like pick. Added simple amortization program to demonstrate Form's -R execute_receiver_cmd functionality 

*2026-05-05T10:22:39-05:00* - Form was failing to display field brackets. Assumed it was either an error in ncurses or a loose nut on my computer and rearranged placements of wnoutrefresh calls to manipulate screen updates in an order that worked. 

*2026-05-05T00:48:48-05:00* - Documentation Updates 

*2026-05-04T20:59:35-05:00* - Documentation Updates 

*2026-05-04T19:32:32-05:00* - popups.c remove superfluous new_form 

*2026-05-04T18:51:33-05:00* - Documentation Updates 

*2026-05-04T18:45:37-05:00* - Documentation Updates 

*2026-05-04T17:22:00-05:00* - Documentation Updates 

*2026-05-04T17:09:03-05:00* - Documentation Updates 

*2026-05-04T17:04:54-05:00* - form_fields.c pressing escape in field editor resulted in endless loop. Added validation for printable ASCII characters to prevent this. Documentation Updates 

*2026-05-03T17:49:43-05:00* - Documentation Updates 

*2026-05-03T11:53:19-05:00* - Documentation Updates 

*2026-05-03T10:55:25-05:00* - Documentation Updates 

*2026-05-03T00:43:48-05:00* - Documentation Updates 

*2026-05-03T00:36:47-05:00* - pick_engine.c: remove toggle selection on enter key 

*2026-05-02T23:15:34-05:00* - Documentation Updates 

*2026-05-02T23:14:15-05:00* - Documentation Updates 

*2026-05-02T23:08:18-05:00* - Documentation Updates 

*2026-05-02T22:58:53-05:00* - Documentation Updates 

*2026-05-02T21:39:57-05:00* - Documentation Updates 

*2026-05-02T15:27:56-05:00* - Documentation Updates 

*2026-05-01T20:19:52-05:00* - Documentation Updates 

*2026-05-01T19:06:41-05:00* - Documentation Updates 

*2026-05-01T19:05:08-05:00* - Documentation Updates 

*2026-05-01T14:00:49-05:00* - Documentation Updates 

*2026-04-30T22:36:52-05:00* - Documentation Updates 

*2026-04-30T15:14:08-05:00* - Documentation Updates 

*2026-04-30T13:52:08-05:00* - Added screenshots to Rustlings exercises and updated the exercises.md file to reflect the changes. The new screenshots show the output of the exercises after completing them, while the old screenshots have been removed as they were outdated. The exercises.md file has been modified to include the new screenshots and to provide a better visual representation of the exercises. This update will help users understand the expected output of the exercises and make it easier for them to follow along with the Rustlings course. 

*2026-04-30T11:50:08-05:00* - Remove hard-coded paths from main.m 

*2026-04-30T10:56:50-05:00* - Provide tilde expansion for external commands. 

*2026-04-29T20:58:43-05:00* - Documentation Updates 

*2026-04-29T11:50:13-05:00* - Documentation Updates 

*2026-04-28T10:03:14-05:00* - Documentation Updates 

*2026-04-28T08:46:19-05:00* - Added "Tab Pick" and "Tab Edit" to chyron. 

*2026-04-27T23:58:27-05:00* - pick_engine: accept line editor field on enter key 

*2026-04-27T22:11:28-05:00* - pick_engine: pick->tbl_pages calculation is correct in line editor main loop. Also fixed updating of pick->tbl_pages on info line. 

*2026-04-27T21:52:17-05:00* - pick_engine: fixed pick->tbl_pages calculation 

*2026-04-27T16:59:55-05:00* - Fixed fork_exec root shell. Fixed spurious mouse click in object selection window triggering selection of the first object in the list. 

*2026-04-27T12:30:40-05:00* - pick_engine revert search term to previous state if change would result in match failure 

*2026-04-27T07:45:11-05:00* - Removed spurious setting of pick->d_cnt in pick_engine.c, which was causing the pick engine to display only the first page of objects. 

*2026-04-26T20:06:43-05:00* - Fix child processes inheriting stderr and corrupting ncurses display. 

*2026-04-26T17:57:35-05:00* - Before all execvp calls, redirect stderr to /dev/null to prevent corruption of ncurses windows. 

*2026-04-26T16:19:33-05:00* - Redirect child output to /dev/null after fork and before execvp to prevent the child from corrupting the ncurses display. Fixes a bug where the child process could write to the terminal and mess up the display of the parent process. 

*2026-04-26T13:08:07-05:00* - pick_engine: minor cosmetic update 

*2026-04-26T12:33:55-05:00* - pick_engine: track number of pages 

*2026-04-26T10:13:09-05:00* - Pick Engine: Turn cursor off in inactive window 

*2026-04-25T20:54:11-05:00* - lf: correcting issues with arguments 

*2026-04-25T20:02:21-05:00* - if lf cannot validate the first positional argument as directory, but determines it is a valid regular expression, it should also check the second positional argument, and if it is a valid regular expression, reject the first positional argument. 

*2026-04-25T19:38:57-05:00* - Fixed failure of lf to detect symlink to valid directory. 

*2026-04-25T16:28:57-05:00* - Pick Engine: refinement of information display. 

*2026-04-25T15:55:03-05:00* - Pick Engine: added line and page numbers to separator line. 

*2026-04-25T15:00:16-05:00* - Pick Engine, window width error. 

*2026-04-25T08:34:46-05:00* - Pick Engine: Accept mouse clicks from line editor window when object selector is active. 

*2026-04-25T08:10:28-05:00* - Increase width of Pick Window to accommodate chyron. 

*2026-04-24T19:05:01-05:00* - Pick Engine. Various improvements and bug fixes. 

*2026-04-24T10:54:24-05:00* - Documentation Updates 

*2026-04-24T10:01:18-05:00* - In menu.c, move _atexit() after curses initialization because _atexit() calls destroy_curses(). 

*2026-04-24T07:51:45-05:00* - Pick Engine set y_offset to 0 when not used. 

*2026-04-24T07:13:10-05:00* - Pick Engine line editor field positioning errors corrected. Added logic to make object selector and line editor cede control when a mouse click comes from other than their assigned window. 

*2026-04-23T22:15:56-05:00* - Documentation Updates 

*2026-04-23T22:03:00-05:00* - Documentation Updates 

*2026-04-23T20:37:41-05:00* - Documentation Updates 

*2026-04-23T10:09:01-05:00* - Pick_engine, fine tuning the odds and ends. 

*2026-04-23T09:28:41-05:00* - Fixed bug in Pick Engine. Pressing the page down key when there is only one page failed to set in_key to 0 resulting in endless loop. 

*2026-04-22T14:16:57-05:00* - Added wrefresh to line editor in Pick Engine. 

*2026-04-22T13:35:33-05:00* - Documentation Update 

*2026-04-22T13:31:24-05:00* - Adjusted Pick Engine box geometry to remove extra background line. 

*2026-04-22T10:51:17-05:00* - Pick Engine redisplay window 2 after displaying help. 

*2026-04-22T06:31:43-05:00* - Somehow lost wmouse_trafo() in dxwgetch, which caused the mouse to not work in dxwgetch. Reinstating it. 

*2026-04-21T21:22:02-05:00* - Documentation Updates 

*2026-04-21T19:48:21-05:00* - Continue fine tuning Pick Engine documentation and screenshots. 

*2026-04-21T16:33:35-05:00* - Documentation Updates 

*2026-04-21T16:07:17-05:00* - Continuing work on Pick Engine. 

*2026-04-21T15:38:29-05:00* - Reworked Pick Engine logic to be more efficient and handle edge cases better. Updated the user interface to provide clearer feedback on pick results. Refactored code for improved readability and maintainability. Added comments to explain complex sections of the code. 

*2026-04-21T10:02:11-05:00* - Update object cursor position. 

*2026-04-20T21:39:24-05:00* - Still more tuning on Pick Engine. 

*2026-04-20T21:21:03-05:00* - Continuing to fine tune the pick engine. 

*2026-04-20T17:17:32-05:00* - Fine tuning Pick's line editor. 

*2026-04-20T08:49:42-05:00* - Valgrind documentation update Merge: 1475850 5f7fa6d 

*2026-04-20T07:17:59-05:00* - Merge pull request #4 from BillWaller/copilot/fix-valgrind-still-reachable Fix Valgrind still-reachable leaks: free new_init() allocations, suppress ncurses internals 

*2026-04-20T03:28:42Z* - Fix Valgrind still-reachable leaks: free new_init allocs, add suppression file, Makefile target, and docs Agent-Logs-Url: https://github.com/BillWaller/C-Menu/sessions/79b9264f-614c-417f-b8d5-a1a6aab29c5f Co-authored-by: BillWaller <10166578+BillWaller@users.noreply.github.com> 

*2026-04-20T03:23:51Z* - Initial plan 

*2026-04-19T22:07:29-05:00* - Fixing valgrind errors. 

*2026-04-19T21:57:17-05:00* - Chasing down valgrind errors and fixing them. 

*2026-04-19T20:15:55-05:00* - Extensively reworked the Pick Engine to make it much more intuitive and easier to use. The new API is much more consistent and easier to understand, and the new implementation is much more efficient and easier to maintain. Specifically, a line editor has been added to the Pick Engine to allow users to enter search strings. The Pick Engine automatically filters the list of items displayed in real time, as you type. It's sort of like Telescope, except that it maintains distinct modes for the Picker and Editor to preserve the individual features and optimizations of each. As a result, Pick delivers a powerful multi-column, multi-page Picker and a full-function line editor. 

*2026-04-18T22:11:35-05:00* - Fixed bug in mem.c which was causing view to fail when reading input from a command line pipe. 

*2026-04-17T20:43:42-05:00* - Cleaning up possible memory leaks in lf.c and futil.c, and fixing a minor issue in the Makefile. 

*2026-04-17T15:06:30-05:00* - Complex characters must be initialized with proper wide character strings and mbstate must be initialized to zeros. 

*2026-04-17T07:22:04-05:00* - lf after and before date/time changes, and some refactoring to make the code more readable and maintainable. 

*2026-04-16T19:10:39-05:00* - lf -i does not have an argument. 

*2026-04-16T19:02:24-05:00* - Documentation Updates 

*2026-04-16T19:00:34-05:00* - Documentation Updates 

*2026-04-16T18:26:38-05:00* - Add -s, file_size, option to lf. For example, lf -t f -s 100k, will only list files greater than or equal to 100k. 

*2026-04-16T11:01:14-05:00* - Added after and before date ranges to lf. 

*2026-04-14T15:31:36-05:00* - Fixed bug in lf. Erroneously used continue where I should have suppressed. 

*2026-04-14T14:40:09-05:00* - Added -u, --user option to lf. Usage is -u user_name. I am trying to keep lf as simple as possible, but this seems like a pretty important capability. Also changed some instances of "-N f" to "-Nf" in the code. This became an error when I added a boolean argument to the -N option without removing the space between N and f. 

*2026-04-14T11:23:17-05:00* - Documentation Updates 

*2026-04-14T11:03:24-05:00* - After adding OPTION_ARG_OPTIONAL to the "-N" option of view, argp interpreted the "f" of "-N f" as a separate argument, causing C-Menu to fail as it tried to open "f" as a file". Reading the documentation for argp, this is normal and expected behavior. The solution, as suggested by the documentation, is to eliminate the space after the -N option. The scripts will be changed and the documentation will clarify that the -N option should be used without a space when providing an argument. I also ran across a problem caused by popup_view using the function arguments for line, col, begy, and begx, when command line arguments were also specified. The desired behavior os to override the function arguments with the command line arguments when they are provided. I have also modified the code to check for the presence of command line arguments and use them instead of the function arguments when they are available. 

*2026-04-13T18:39:03-05:00* - More minor fixes for View line numbering 

*2026-04-13T15:00:52-05:00* - Fine tuning line numbering toggle 

*2026-04-12T20:11:26-05:00* - Documentation Updates 

*2026-04-12T12:15:39-05:00* - Fixed View trying to open line number window when line number flag set to false. 

*2026-04-12T06:35:51-05:00* - More work on resizing View windows 

*2026-04-11T21:03:43-05:00* - Documentation Updates 

*2026-04-11T20:52:57-05:00* - Working on View window resizing. It's not perfect yet, but we are making progress. As always, if you have any suggestions or feedback, please let me know. I am open to any ideas that can help improve the resizing behavior and overall user experience. Thank you for your continued support and contributions to the project. Let's keep pushing forward and making this software better together! 

*2026-04-10T15:36:55-05:00* - Expand view line number window to 8 characters wide. 

*2026-04-10T14:17:15-05:00* - Expanded line number window to 7 digits. To accommodate more than 7 digits, we will need to allow scalinig of the line number window. 

*2026-04-10T13:45:35-05:00* - Corrected line numbering issue in view when scrolling up and then down. 

*2026-04-10T11:05:18-05:00* - Don't activate cursor after deleting a window 

*2026-04-10T11:00:49-05:00* - Erase and refresh stdscr after execvp() 

*2026-04-10T10:52:16-05:00* - Replace old submenu code with popup_menu() 

*2026-04-10T08:27:20-05:00* - Correct color rendition in view 

*2026-04-10T08:03:10-05:00* - set menu to init->menu to avoid SIGSEGV 

*2026-04-10T07:32:29-05:00* - Set background to NORMAL when deleting windows 

*2026-04-10T05:43:44-05:00* - Uncommented endwin() in destroy_curses() to prevent the terminal from being left in an unusable state if the program crashes. 

*2026-04-09T21:42:19-05:00* - Documentation Updates 

*2026-04-09T19:44:49-05:00* - Reset terminal on exit 

*2026-04-09T18:31:10-05:00* - Add atexit and end_pgm functions to menu.c to ensure proper cleanup of resources when the program exits. This will help prevent memory leaks and ensure that any necessary finalization steps are performed before the program terminates. 

*2026-04-09T18:14:55-05:00* - Taking care of valgrind issues. While I am working hard to chase down memory leaks, the fixes may have side-effects leading to other issues. Be assured, I am addressing all known issues as quickly as possible. 

*2026-04-09T15:20:01-05:00* - Fixed fastbin chunk detected 

*2026-04-09T15:16:08-05:00* - Valgrind fixes, and some other minor cleanups. See individual commits for details. 

*2026-04-09T11:31:35-05:00* - Off by one free error in destroy_view. 

*2026-04-09T11:21:46-05:00* - Fixed memory leak in view. 

*2026-04-08T23:49:19-05:00* - Minor fixes 

*2026-04-08T16:43:56-05:00* - Documentation Updates 

*2026-04-08T13:53:35-05:00* - Removed -a (List all files, including hidden) option from lf and added -n (Don't list hidden files) to conform with find's default of listing all files. The -n option is equivalent to find . -not -name ".*" 

*2026-04-08T13:10:19-05:00* - Changed lf default depth to unlimited, in conformance with find. lf did have a default depth of 3 because it was handy for initial testing. As it no longer serves a purpose, the default depth is 0, which means unlimited depth like find. My next quandry is whether to change the behavior of lf concerning hidden files. By default, lf ignores hidden files, while find includes them. I am leaning towards changing lf to include hidden files by default. 

*2026-04-08T10:22:38-05:00* - Documentation Updates 

*2026-04-08T10:18:31-05:00* - Documentation Updates 

*2026-04-08T08:22:56-05:00* - Documentation Updates 

*2026-04-08T07:17:06-05:00* - Fixed 

*2026-04-07T22:45:43-05:00* - Fixed errors in fork_exec(). 

*2026-04-07T14:45:19-05:00* - Documentation Updates 

*2026-04-07T14:13:22-05:00* - Documentation Updates 

*2026-04-07T14:11:38-05:00* - Documentation Updates 

*2026-04-07T14:10:10-05:00* - Documentation Updates 

*2026-04-07T13:56:52-05:00* - Documentation Updates 

*2026-04-07T13:32:16-05:00* - Documentation Updates 

*2026-04-07T13:11:28-05:00* - Documentation Updates 

*2026-04-07T10:30:02-05:00* - Documentation Updates 

*2026-04-07T10:07:55-05:00* - Documentation Updates 

*2026-04-07T09:05:27-05:00* - Documentation Updates 

*2026-04-07T09:03:17-05:00* - Documentation Updates 

*2026-04-07T08:48:48-05:00* - Documentation Updates 

*2026-04-07T08:46:14-05:00* - Documentation Updates 

*2026-04-07T08:45:04-05:00* - Documentation Updates 

*2026-04-07T08:17:33-05:00* - Documentation updates. 

*2026-04-07T08:14:14-05:00* - Documentation Updates 

*2026-04-06T23:15:29-05:00* - Documentation updates. 

*2026-04-06T22:56:32-05:00* - Documentation Updates. 

*2026-04-06T22:54:15-05:00* - Documentation Updates 

*2026-04-06T22:14:49-05:00* - Documentation Updates 

*2026-04-06T21:36:45-05:00* - Update Installation Guide 

*2026-04-06T20:50:43-05:00* - Clean up script errors 

*2026-04-06T20:30:22-05:00* - Resolving issues between NCurses and Neovim handling of alternate and main screens related to smcup and rmcup. This commit includes changes to the Makefile, menu_engine.c, and pick_engine.c to ensure compatibility and proper handling of screen states in both environments. 

*2026-04-06T16:02:05-05:00* - Fixed several errors in scripts. 

*2026-04-06T15:27:18-05:00* - Documentation Updates. 

*2026-04-06T15:25:23-05:00* - Documentation updates. 

*2026-04-06T13:00:39-05:00* - Reworked menu_engine logic, added c23 standard to .clangd, made SIGSEGV handler messages more informative. 

*2026-04-05T23:11:28-05:00* - Fixed bug in waiting for process. Added confirmation of program execution. 

*2026-04-05T11:09:59-05:00* - Clear screen before execvp. 

*2026-04-04T20:55:16-05:00* - Cleaned up entry and exit for fork-exec executables and added checks for directories in workstation setup scripts. 

*2026-04-04T13:46:26-05:00* - Clear before execvp. 

*2026-04-04T13:41:24-05:00* - Clear artifacts from screen before and after fork_exec. 

*2026-04-04T09:43:01-05:00* - Documentation Updates 

*2026-04-04T04:58:54-05:00* - pick_engine - pick->obj_idx was being erroneously set to 0 in picker main loop. 

*2026-04-03T21:05:44-05:00* - Tuning some of the fine details. 

*2026-04-03T15:15:08-05:00* - When starting a full screen root shell from the main.m menu, the ncurses screen was not being cleared, and the root shell was being launched on top of the ncurses screen. This was because the clear() function was not being called before launching the root shell. To fix this issue, I added a call to clear() before launching the root shell in the main.m file. This ensures that the ncurses screen is cleared before the root shell is launched, providing a clean and clear interface for the user. 

*2026-04-03T12:00:28-05:00* - Added a complete user session with Form to the User Guide. 

*2026-04-03T00:14:36-05:00* - Update menuapp files. 

*2026-04-02T22:16:28-05:00* - Catching the edge cases now. I would love to have some help locating bugs so 0.2.9 can be wrapped up soon. I have been testing the new features and they seem to be working well, but as the designer, I am functionally fixed. If you have some time, please try out the latest version and make a list of things that don't work they way they should. I will be working on the list and trying to resolve as many of the issues as I can. I was a lead developer for many years, so I an no newby to criticism. I know what kinds of bugs live in the shadows of every software project. No one shapes a software product more than the early adopters. Theirs is perhaps the greatest contribution to great software. 

*2026-04-02T15:30:12-05:00* - Valgrinding and fixing some memory leaks and other minor issues. Also added some comments to the code for better readability. 

*2026-04-02T11:17:50-05:00* - My tree-sitter parser for shell scripts isn't working properly, so i removed the "sh" entries from source queries. 

*2026-04-02T11:12:31-05:00* - Fixed bug in form_engine related to F5 process. 

*2026-04-02T10:41:18-05:00* - Documentation update 

*2026-04-02T10:31:29-05:00* - Clean up screen artifacts after running various processes in C-Menu. 

*2026-04-02T09:20:13-05:00* - Prevent nvim from leaving artifacts on screen. 

*2026-04-02T08:10:42-05:00* - Added help files to Menu, Form, and Pick and cleaned up the code to display help. 

*2026-04-02T06:22:20-05:00* - Included form help file and fixed bug in form_engine. 

*2026-04-02T04:32:51-05:00* - Fixed bug in form engine help. 

*2026-04-01T22:06:10-05:00* - Documentation updates. 

*2026-04-01T21:56:46-05:00* - Documentation updates. 

*2026-04-01T21:32:26-05:00* - Documentation updates. 

*2026-04-01T21:23:32-05:00* - Documentation updates. 

*2026-04-01T21:21:31-05:00* - Documentation Updates 

*2026-04-01T17:03:45-05:00* - Tidy up the chyron a bit. 

*2026-04-01T15:09:50-05:00* - Further testing proved that "wchar_t wstr[2] = {L'\0', L'\0'};" will work for "setcchar(&cc, wstr, WA_NORMAL, cpx, nullptr);", so re-reverted to wstr. 

*2026-04-01T14:55:23-05:00* - Changing final setcchar(&cc, &wc, WA_NORMAL, cpx, nullptr) to setcchar(&cc, wstr, WA_NORMAL, cpx, nullptr) lost termination of cmplx_buf. Reverted back to scalar wc. 

*2026-04-01T14:03:13-05:00* - Replace all ocrrurences of scalar wc with wstr 

*2026-04-01T13:52:18-05:00* - Remove unnecessary code from whence 

*2026-04-01T11:21:52-05:00* - pick_engine.c - highlight selection when toggled on with mouse 

*2026-04-01T07:47:12-05:00* - Added wclrtoeol() at end of chyron 

*2026-04-01T07:43:02-05:00* - Removed extra space at end of chyron. 

*2026-03-31T19:56:36-05:00* - Added set_chyron_key_cp to insert/overwrite toggle switch in the field editor. 

*2026-03-31T19:35:38-05:00* - Added color pair to to set_chyron_key so that certain keys can be highlighted. This is used as a hint to the user as to the next logical key to press or click with the mouse. 

*2026-03-31T13:04:36-05:00* - Fixed bug in pick_engine related to argp processing. Essentially, argp provides all the non-option arguments to be captured in an array not including argv[0]. The getopt code returned an array of all arguments with an index to the first non-option argument. This caused the pick_engine code to fail when it tried to access the non-option arguments as it was accessing the wrong indices. The fix was to adjust the indices in pick_engine to account for the fact that argp does not include argv[0] in the array of non-option arguments. This should resolve the issue and allow pick_engine to function correctly with argp. 

*2026-03-30T18:39:10-05:00* - More clean-up and documentation updates. 

*2026-03-30T17:45:07-05:00* - Refactoring some function calls to use the new API, and removing some unused code. 

*2026-03-30T17:34:53-05:00* - Resolved more issues with argp conversion. 

*2026-03-30T14:24:41-05:00* - Incorporate argp into whence 

*2026-03-30T12:36:34-05:00* - Fixed another error resulting from switching to argp from getopt. There is nothing wrong with the argp library. These issues are solely my fault for not testing the code more thoroughly after switching to argp. I will be more careful in the future. 

*2026-03-30T11:28:14-05:00* - Fixed off-by-one error introduced by migration to argp in commit 9c8e5b1. The error caused the last argument to be ignored, which was the path to the form description file. This involved setting optind to state->next - 1 instead of state->next. 

*2026-03-29T23:56:02-05:00* - Fixed bugs related to upgrading from getopt to argp. 

*2026-03-28T19:16:54-05:00* - Switched from getopt to argp for better argument parsing and help message generation. This change improves the user experience by providing clearer usage instructions and more robust handling of command-line arguments. The new implementation also allows for easier maintenance and future enhancements to the argument parsing logic. 

*2026-03-28T06:55:01-05:00* - Documentation Updates 

*2026-03-27T23:20:38-05:00* - Documentation Updates 

*2026-03-27T23:13:53-05:00* - Documentation Updates 

*2026-03-27T20:40:11-05:00* - Delete C-Menu-0.2.9-Linux-x86_64.tar.xz Signed-off-by: Bill Waller <billxwaller@gmail.com> 

*2026-03-27T18:28:22-05:00* - Add files via upload Signed-off-by: Bill Waller <billxwaller@gmail.com> 

*2026-03-27T18:16:34-05:00* - Documentation Updates 

*2026-03-27T16:18:24-05:00* - Testing revealed some off-by-one errors in view. Fixed all that I know about. 

*2026-03-26T19:42:30-05:00* - Fixed menu calling popup_view instead of mview, which caused SIGSEGV on exit. 

*2026-03-26T18:17:48-05:00* - Making the build a little cleaner 

*2026-03-26T18:00:46-05:00* - view clrtobot at eod 

*2026-03-26T17:21:37-05:00* - Restore cursor at end of program 

*2026-03-26T17:11:28-05:00* - Cosmetic improvement to view 

*2026-03-26T17:05:30-05:00* - Removed artifacts from bottom line of line number window 

*2026-03-26T15:13:23-05:00* - Fixed several bugs arising from refactoring xwgetch() to expand its usefulness for managing wait states, synchronizing forked processes, and catching signals. 

*2026-03-26T00:04:43-05:00* - Refresh pad after selecting -n line numbering from command line 

*2026-03-25T21:46:50-05:00* - Refactor field input after xwgetch changes 

*2026-03-25T13:02:54-05:00* - Remove mview and updated CMakeLists.txt and Makefile 

*2026-03-25T12:35:54-05:00* - Removed unneeded source files menu.c, form.c, pick.c, view.c, and ckeys.c 

*2026-03-25T11:09:57-05:00* - Combined menu, form, pick, view, and ckeys into a single executable, main, which substitutes for menu, form, pick, view, and ckeys when executed via symbolic links of those same names to main. Created a module, popups.c, that contains function calls to menu, form, pick, view, and ckeys, which will function as pop-ups and drop-downs within the main executable. 

*2026-03-24T22:59:29-05:00* - Put #ifdefs in rsh to make SSH authentication and system logging optional. 

*2026-03-24T21:15:33-05:00* - Changelog Updates 

*2026-03-24T20:37:31-05:00* - RSH Activating SSH Authentication 

*2026-03-24T19:50:35-05:00* - Added ssh authentication and system logging to RSH 

*2026-03-24T12:13:05-05:00* - Documentation Updates 

*2026-03-24T11:04:45-05:00* - Fix view display_page clearing last line. 

*2026-03-24T10:54:59-05:00* - Get rid of flickering in view display_page by moving wclrtobot to after the last line is displayed. 

*2026-03-24T09:14:41-05:00* - Documentation Updates 

*2026-03-24T05:38:50-05:00* - Remove DEBUG from cm.h 

*2026-03-24T05:35:33-05:00* - Fine tuning of wait timing 

*2026-03-23T21:20:31-05:00* - Testing and debugging of the pick engine and the view initialization. 

*2026-03-23T19:44:43-05:00* - Resolve timing issues with pick and view engines 

*2026-03-23T19:28:06-05:00* - Synchronize forked processes 

*2026-03-23T18:03:28-05:00* - Update to init_view waiting display 

*2026-03-23T17:55:27-05:00* - Working with waiting pop-ups 

*2026-03-23T17:49:22-05:00* - Further work on the wait displays 

*2026-03-23T16:22:28-05:00* - Add -rdynamic option to debug builds 

*2026-03-23T16:04:39-05:00* - Fine tuning the timeout and fixing a bug. 

*2026-03-23T15:02:13-05:00* - Added pop-up wait message with countdown timer in view and pick while waiting for input from provider processes. Default timeout is 5 seconds. 

*2026-03-22T08:46:21-05:00* - Documentation Updates 

*2026-03-21T22:54:39-05:00* - Suppress "File of" if input is stdin 

*2026-03-21T15:01:00-05:00* - Documentation Updates 

*2026-03-21T11:50:11-05:00* - Documentation Updates 

*2026-03-21T00:11:08-05:00* - Toggle line numbers with "-n" in view 

*2026-03-20T22:35:52-05:00* - Documentation Updates 

*2026-03-20T22:10:32-05:00* - Documentation Updates 

*2026-03-20T13:50:45-05:00* - Documentation Updates 

*2026-03-20T09:06:11-05:00* - Documentation Updates 

*2026-03-20T08:50:16-05:00* - General clean-up of the code 

*2026-03-19T23:36:07-05:00* - Fixed bug in xwgetch that caused it to miss mouse clicks. 

*2026-03-19T21:37:22-05:00* - Fix bug in View vertical scrolling with mouse wheel 

*2026-03-19T21:19:15-05:00* - Fixed mouse wheel vertical scrolling in view_engine. 

*2026-03-19T14:49:02-05:00* - Documentation Updates 

*2026-03-19T14:41:26-05:00* - Documentation Updates 

*2026-03-19T14:36:17-05:00* - Documentation Updates 

*2026-03-19T14:32:05-05:00* - Documentation Updates 

*2026-03-19T14:30:49-05:00* - Documentation Updates 

*2026-03-19T14:21:47-05:00* - Documentation Updates 

*2026-03-18T22:59:34-05:00* - Fixed some logic errors in view related to page positioning. 

*2026-03-18T21:48:51-05:00* - Fine tuning the paging logic 

*2026-03-18T18:20:38-05:00* - Corrected Page Up logic in view 

*2026-03-18T14:24:44-05:00* - fixed Pick failing to operate on the last item in the object list. This was caused by an off-by-one error in the loop that iterates through the objects. The loop condition was checking for `i < num_objects - 1` instead of `i < num_objects`, which caused it to skip the last object. This fix changes the loop condition to `i < num_objects`, allowing Pick to properly operate on all objects in the list, including the last one. 

*2026-03-18T13:39:29-05:00* - Close line number window properly to clear artifact 

*2026-03-18T10:57:07-05:00* - free data structures view->ln_tbl, view->ln_win 

*2026-03-17T23:02:47-05:00* - Documentation Updates 

*2026-03-17T22:50:19-05:00* - Added line number option (-N) to view command. 

*2026-03-14T11:32:38-05:00* - lf enhanced to selectively list any combination of 8 file types. 

*2026-03-12T21:14:40-05:00* - Documentation Updates 

*2026-03-12T20:57:41-05:00* - Documentation Updates 

*2026-03-12T18:12:54-05:00* - Documentation Updates 

*2026-03-12T18:10:16-05:00* - Documentation Updates 

*2026-03-12T00:20:24-05:00* - fixed bug in lf -a not showing hidden files 

*2026-03-10T17:23:07-05:00* - Documentation Updates 

*2026-03-10T16:30:40-05:00* - Documentation Updates 

*2026-03-10T16:29:27-05:00* - Documentation Updates 

*2026-03-10T16:24:39-05:00* - Documentation Updates 

*2026-03-10T14:37:53-05:00* - Documentation Updates 

*2026-03-10T14:34:33-05:00* - Documentation Updates 

*2026-03-10T09:30:41-05:00* - Documentation Updates 

*2026-03-10T07:59:40-05:00* - Prevent lf from following links 

*2026-03-09T17:55:46-05:00* - Modified scripts to sort output 

*2026-03-09T17:47:22-05:00* - Field editor refinements/cleanup 

*2026-03-09T12:00:35-05:00* - Removed intentional segmentation fault used for testing. This function was designed to verify that the program would core dump instead of continuing to run after a SIGSEGV interrupt and it functioned correctly, breaking the program as designed. It is a testing artifact not intended for production code. 

*2026-03-09T11:55:25-05:00* - Fixed bug in field editor insert mode. 

*2026-03-09T11:06:46-05:00* - Modified lf to use current directory instead of complaining if first non-option argument is not a valid directory. 

*2026-03-08T22:16:45-05:00* - Fixed problem with lf 

*2026-03-08T18:27:43-05:00* - Hopefully fixed stubborn bug in lf 

*2026-03-08T16:06:42-05:00* - Many bug fixes and improvements, including: integrating chyron into form and pick, fixing a long-standing bug in the form engine, improving the pick engine. Fixed another bug in lf. 

*2026-03-07T11:08:34-06:00* - Added section on lf 

*2026-03-07T10:01:54-06:00* - Documentation Updates 

*2026-03-07T09:14:31-06:00* - Corrected Pick KEY_DOWN behavior 

*2026-03-06T18:58:15-06:00* - Additions to Changelog 

*2026-03-06T18:15:43-06:00* - Upgraded the menuapp and src code with various modifications, including changes to the sddm_chbg.sh script, main.m file, and several C source files. Additionally, a new file diag.m was added to the menuapp/msrc directory, and an old file xx was deleted. These changes aim to enhance the functionality and performance of the application while maintaining compatibility with existing features. The chyron functions, which have been updated to handle mouse events, will now provide a more interactive user experience. The modifications in the form_engine.c and related files will improve the overall efficiency of the form handling process,while the updates in the include files will ensure better code organization and maintainability. Overall, these changes contribute to a more robust and user-friendly application, aligning with the project's goals and objectives. 

*2026-03-06T12:16:41-06:00* - Added incomplete message 

*2026-03-06T10:16:56-06:00* - Documentation Updates 

*2026-03-05T20:22:02-06:00* - Documentation Updates 

*2026-03-05T18:45:14-06:00* - Bug fixes, exercises, scripts and screenshots 

*2026-03-05T14:02:06-06:00* - Fixed bugs in menu and added exclusion regex to lf. 

*2026-03-03T21:16:58-06:00* - Documentation Updates 

*2026-03-02T14:25:58-06:00* - Replace NULL with nullptr 

*2026-03-01T19:58:26-06:00* - Pick failing to draw box window if no title 

*2026-03-01T09:18:23-06:00* - Documentation Updates 

*2026-03-01T09:16:23-06:00* - Documentation Updates 

*2026-02-28T13:55:51-06:00* - Documentation Updates 

*2026-02-28T13:53:30-06:00* - Documentation Updates 

*2026-02-27T21:55:31-06:00* - Perfect sub-window positioning 

*2026-02-27T21:52:15-06:00* - Documentation Updates 

*2026-02-27T20:29:01-06:00* - Code and Documentation inmprovements 

*2026-02-27T09:40:27-06:00* - Use new options with lf 

*2026-02-27T09:33:18-06:00* - Fixed another issue with lf 

*2026-02-27T08:50:16-06:00* - Fixed lf (list files) 

*2026-02-26T19:57:38-06:00* - Documentation Updates 

*2026-02-26T19:42:31-06:00* - Documentation Updates 

*2026-02-26T11:05:52-06:00* - Documentation Updates 

*2026-02-26T11:02:24-06:00* - Documentation Updates 

*2026-02-25T23:47:54-06:00* - Added Desktop startup files 

*2026-02-25T22:52:59-06:00* - Correct transposition of iso8601 

*2026-02-25T17:48:44-06:00* - Documentation Updates 

*2026-02-24T20:44:48-06:00* - Documentation Updates 

*2026-02-24T20:35:23-06:00* - Documentation Updates 

*2026-02-24T14:34:09-06:00* - Corrected search page-top, page-bottom positions 

*2026-02-24T12:59:37-06:00* - Corrected issue with lf -d, (depth) 

*2026-02-24T12:15:49-06:00* - Corrected problem with lf 

*2026-02-24T07:49:23-06:00* - Move chktty to bin 

*2026-02-24T07:47:19-06:00* - Documentation Updates 

*2026-02-23T16:03:24-06:00* - Added message if tty directory not found 

*2026-02-23T15:50:01-06:00* - View reverse search logic fixes 

*2026-02-22T16:39:57-06:00* - Make include directives more portable for API use 

*2026-02-22T16:20:25-06:00* - Correcting type-o 

*2026-02-22T16:07:16-06:00* - More work on CMake build system 

*2026-02-22T12:41:43-06:00* - Remove installed include and lib64 directories 

*2026-02-22T12:25:46-06:00* - CMake - update installation logic 

*2026-02-22T12:02:07-06:00* - Install CMenu.conf in ld.so.conf 

*2026-02-21T09:45:55-06:00* - Documentation Updates 

*2026-02-21T09:43:29-06:00* - Documentation Updates 

*2026-02-21T09:15:19-06:00* - View Window Geometry 

*2026-02-20T14:48:25-06:00* - Reworking View's search logic 

*2026-02-19T16:57:06-06:00* - Fixed problem with multi-byte conv. mbrtowc() 

*2026-02-19T12:27:28-06:00* - Documentation Updates 

*2026-02-19T11:44:46-06:00* - Documentation Updates 

*2026-02-18T20:15:00-06:00* - Added environment variable for static rsh build 

*2026-02-18T15:41:26-06:00* - Added option to statically link rsh 

*2026-02-18T12:11:31-06:00* - Corrected issue with View wrapping 

*2026-02-17T20:49:33-06:00* - Add .bashrc to .gitignore 

*2026-02-17T20:35:02-06:00* - Remove GH_TOKEN 

*2026-02-17T20:05:25-06:00* - Deleted GH_TOKEN from .bashrc 

*2026-02-17T20:03:14-06:00* - Logic correction in view write buffer to file 

*2026-02-16T20:20:44-06:00* - CMake Build install directory 

*2026-02-16T20:01:20-06:00* - Documentation Updates 

*2026-02-16T19:51:46-06:00* - Added logic for editing current buffer 

*2026-02-15T19:26:51-06:00* - Library versioning CMake and Makefile 

*2026-02-15T13:49:03-06:00* - Correct Misspelling in menu.m 

*2026-02-15T11:38:43-06:00* - Fix several programs that weren't expanding tilde 

*2026-02-15T10:01:33-06:00* - Documentation Updates 

*2026-02-13T22:37:54-06:00* - Documentation Updates 

*2026-02-13T21:40:17-06:00* - Documentation Updates 

*2026-02-13T21:19:00-06:00* - Documentation Updates 

*2026-02-13T21:17:27-06:00* - Documentation Updates 

*2026-02-13T21:12:22-06:00* - Documentation Updates 

*2026-02-12T23:07:57-06:00* - Documentation Updates 

*2026-02-12T22:53:24-06:00* - Documentation Updates 

*2026-02-12T19:27:52-06:00* - Improving Help display 

*2026-02-12T17:09:50-06:00* - Improved Signal Handling, Help mechanism 

*2026-02-11T18:53:32-06:00* - Documentation Updates 

*2026-02-11T18:49:57-06:00* - Documentation Updates 

*2026-02-11T18:39:58-06:00* - Documentation Updates 

*2026-02-11T18:02:41-06:00* - Documentation Updates 

*2026-02-11T18:07:44-06:00* - Add files via upload Signed-off-by: Bill Waller <billxwaller@gmail.com> 

*2026-02-11T18:06:14-06:00* - Add files via upload Signed-off-by: Bill Waller <billxwaller@gmail.com> 

*2026-02-11T15:38:26-06:00* - Update C-Menu API link to use HTTPS Signed-off-by: Bill Waller <billxwaller@gmail.com> 

*2026-02-11T14:50:33-06:00* - Documentation Updates 

*2026-02-10T22:34:27-06:00* - Reorganize files, added doxygen API 

*2026-02-08T23:39:17-06:00* - Documentation Updates 

*2026-02-06T23:51:51-06:00* - Documentation Updates 

*2026-02-06T20:56:59-06:00* - Documentation Updates 

*2026-02-06T15:11:12-06:00* - Documentation Update 

*2026-02-06T14:58:50-06:00* - Processing of mview parameters, Documentation 

*2026-02-05T18:06:14-06:00* - Corrections to Makefile and CMake install paths 

*2026-02-05T09:44:19-06:00* - Added dependencies to make install 

*2026-02-05T09:41:10-06:00* - Misspelling in Makefile 

*2026-02-05T00:36:06-06:00* - Mogrify Images 

*2026-02-05T00:27:31-06:00* - Documentation Updates 

*2026-02-04T09:07:22-06:00* - Documentation Updates 

*2026-02-03T21:30:17-06:00* - Calculate Window Size if zero 

*2026-02-03T13:19:07-06:00* - API placed in libcm.so 

*2026-01-31T19:57:53-06:00* - Documentation Updates 

*2026-01-31T18:25:55-06:00* - Documentation Updates 

*2026-01-31T16:45:04-06:00* - View Engine Improvements and Bug Fixes 

*2026-01-31T09:37:11-06:00* - Corrected errors in View Search Logic 

*2026-01-30T23:27:38-06:00* - Refine horizontal scrolling behavior in view 

*2026-01-30T19:15:08-06:00* - Modified Horizantal Scrolling default to pagewidth 

*2026-01-30T10:26:25-06:00* - Documentation Updates 

*2026-01-29T23:48:30-06:00* - Documentation Updates 

*2026-01-29T14:47:36-06:00* - Documentation Updates 

*2026-01-28T14:11:55-06:00* - Throw an error for bad ANSI escape sequences 

*2026-01-28T11:41:44-06:00* - Documentation Updates 

*2026-01-28T10:35:06-06:00* - Documentation Updates 

*2026-01-28T09:44:42-06:00* - Documentation updates and bug fixes 

*2026-01-27T22:56:00-06:00* - Corrected issue with italics attribute 

*2026-01-27T13:48:56-06:00* - Remove unneeded debugging code 

*2026-01-27T13:13:28-06:00* - Update terminfo documentation 

*2026-01-27T12:21:21-06:00* - Tweaking Color Supprt 

*2026-01-26T14:44:53-06:00* - Continuing work on view_engine.c 

*2026-01-26T13:53:55-06:00* - Corrected issue with input line index 

*2026-01-26T13:24:49-06:00* - view-engine fmt_line input handling optimization 

*2026-01-26T00:35:58-06:00* - Signal Handler, Documentation Updates 

*2026-01-25T00:36:03-06:00* - Delete Unneeded directory 

*2026-01-24T23:30:14-06:00* - Documentation Updates 

*2026-01-24T22:27:44-06:00* - Documentation Updates 

*2026-01-24T22:19:01-06:00* - Documentation Updates 

*2026-01-24T20:44:42-06:00* - Improve Signal Handling, Doc Updates 

*2026-01-24T10:45:44-06:00* - Documentation Updates 

*2026-01-24T10:34:42-06:00* - Documentation Updates 

*2026-01-24T10:29:03-06:00* - Documentation Updates 

*2026-01-24T09:07:22-06:00* - Corrected Gamma Calculation, added gray gamma 

*2026-01-23T22:48:59-06:00* - Documentation Updates 

*2026-01-23T18:41:53-06:00* - Documentation Updates 

*2026-01-23T18:35:33-06:00* - Documentation Updates 

*2026-01-23T18:33:45-06:00* - Documentation Updates 

*2026-01-23T13:20:07-06:00* - Documentation Updates 

*2026-01-22T21:49:10-06:00* - Documentation Updates 

*2026-01-22T18:44:17-06:00* - Documentation Updates 

*2026-01-22T17:25:03-06:00* - Documentation Updates 

*2026-01-22T17:19:10-06:00* - Documentation Updates 

*2026-01-22T12:46:28-06:00* - Documentation Updates 

*2026-01-22T12:43:46-06:00* - Documentation Updates 

*2026-01-22T12:35:16-06:00* - Documentation Updates 

*2026-01-22T00:54:41-06:00* - Clean-up of obsolete test and work files 

*2026-01-21T23:45:27-06:00* - Documentation Updates 

*2026-01-21T20:31:13-06:00* - Documentation Updates 

*2026-01-21T18:12:53-06:00* - Documentation Updates 

*2026-01-21T17:42:02-06:00* - Corrected issues with ANSI to Complex Char 

*2026-01-21T12:02:35-06:00* - Added Terminal Emulator Example Configurations 

*2026-01-21T11:55:01-06:00* - Documentation Updates 

*2026-01-21T10:42:00-06:00* - Corrected rgb_to_xterm256_idx, gcc -O2 issues 

*2026-01-20T21:23:30-06:00* - Conflicting signatures for str_tok_r 

*2026-01-20T17:50:53-06:00* - Update str_tok_r 

*2026-01-20T16:52:49-06:00* - Documentation Updates 

*2026-01-20T12:37:38-06:00* - Documentation Updates 

*2026-01-20T11:45:25-06:00* - Documentation Updates 

*2026-01-19T20:34:55-06:00* - Documentation Updates 

*2026-01-19T20:30:30-06:00* - Documentation Updates 

*2026-01-19T20:24:24-06:00* - Documentation Updates 

*2026-01-19T20:15:44-06:00* - Documentation Updates 

*2026-01-19T17:11:25-06:00* - Documentation Updates 

*2026-01-19T17:02:03-06:00* - File handling and memory management issues. 

*2026-01-18T19:27:11-06:00* - Documentation Updates 

*2026-01-18T18:15:59-06:00* - Documentation Updates 

*2026-01-18T12:15:47-06:00* - Documentation Updates 

*2026-01-18T11:42:56-06:00* - Documentation Updates 

*2026-01-18T10:18:45-06:00* - Documentation Updates 

*2026-01-17T23:05:46-06:00* - Documentation Updates 

*2026-01-17T22:20:52-06:00* - Documentation Updates 

*2026-01-17T22:12:13-06:00* - Documentation Updates 

*2026-01-17T19:58:45-06:00* - Documentation Updates 

*2026-01-17T12:43:19-06:00* - Documentation Updates 

*2026-01-16T21:05:03-06:00* - Documentation Updates 

*2026-01-16T21:00:56-06:00* - Documentation Updates 

*2026-01-16T17:38:31-06:00* - Documentation Updates 

*2026-01-16T17:13:13-06:00* - Documentation Updates 

*2026-01-16T01:34:27-06:00* - Documentation Updates 

*2026-01-16T00:48:47-06:00* - stripansi - Handle \033[K, API.md Update Doc 

*2026-01-15T22:31:41-06:00* - Documentation Updates 

*2026-01-15T20:28:49-06:00* - Same as previous commit, except for boxwin 

*2026-01-15T19:53:36-06:00* - View - enforce horizontal scrolling limit 

*2026-01-15T09:40:43-06:00* - Fixed unable to read stdin in view (ssize_t issue) 

*2026-01-14T19:24:32-06:00* - Documentation Updates 

*2026-01-14T12:11:44-06:00* - Documentation updates. 

*2026-01-14T10:03:33-06:00* - Data type corrections 

*2026-01-13T22:58:06-06:00* - Corrections to data types in view 

*2026-01-13T21:35:31-06:00* - View - interpret ANSI SGR \033[K correctly 

*2026-01-12T16:24:12-06:00* - Documentation updates 

*2026-01-09T18:10:51-06:00* - Documentation Updates 

*2026-01-09T13:27:19-06:00* - Documentation Updates 

*2026-01-09T13:12:51-06:00* - Documentation update 

*2026-01-09T12:36:50-06:00* - Documentation Updates 

*2026-01-08T23:11:10-06:00* - Added functionality to handle window resizing in the view engine. 

*2026-01-08T18:24:44-06:00* - Resolve problems exposed by ccc-analyzer 

*2026-01-08T17:52:00-06:00* - Documentation Updates 

*2026-01-08T01:07:22-06:00* - Updates 

*2026-01-08T00:12:46-06:00* - Documentation update 

*2026-01-07T23:18:10-06:00* - Documentation update 

*2026-01-07T23:16:08-06:00* - Documentation uppdate 

*2026-01-07T23:12:09-06:00* - Documentation update 

*2026-01-07T23:06:22-06:00* - Documentation update 

*2026-01-07T21:09:26-06:00* - Documentation Updates 

*2026-01-07T08:35:43-06:00* - Listed recent changes 

*2026-01-07T08:16:06-06:00* - Movement keys 

*2026-01-06T23:09:33-06:00* - Documentation updates Merge: 66704b9 0f77a44 

*2026-01-06T22:26:19-06:00* - Merge remote-tracking branch 'refs/remotes/origin/main' 

*2026-01-06T22:09:20-06:00* - Clean-up 

*2026-01-06T22:07:31-06:00* - Delete doc/extras.html 

*2026-01-06T21:56:33-06:00* - Update image paths in extras.html 

*2026-01-06T21:52:46-06:00* - Delete doc/screenshots 

*2026-01-06T21:41:15-06:00* - Clean-up 

*2026-01-06T21:25:48-06:00* - Continuing updates 

*2026-01-06T20:55:55-06:00* - Added hjkl to motion keys, added Form entry to View 

*2026-01-05T20:09:07-06:00* - Modified image directory in extras.html 

*2026-01-05T20:06:08-06:00* - Fix image paths in extras.md Updated image paths in extras.md to remove '../' prefix. 

*2026-01-05T19:59:40-06:00* - Move extras to root directory 

*2026-01-05T19:54:06-06:00* - Updates for documentation and code comments 

*2026-01-04T19:58:12-06:00* - Continuing clean-up 

*2026-01-04T11:14:22-06:00* - Updates to CMake build 

*2026-01-04T09:03:57-06:00* - Added extras.html file and updated documentation files 

*2026-01-03T21:12:52-06:00* - Continuing clean-up 

*2026-01-02T18:18:10-06:00* - Additions 

*2026-01-02T14:49:19-06:00* - Title banner box view 

*2026-01-02T14:12:51-06:00* - Next page test for EOD 

*2026-01-02T11:06:25-06:00* - Mouse Support for Function Keys in Form_Engine 

*2026-01-02T09:07:40-06:00* - Add -d (depth) option to lf 

*2026-01-01T21:52:14-06:00* - Deal with superfluous escape character in input file 

*2026-01-01T13:47:37-06:00* - Continuing clean-up 

*2026-01-01T08:17:06-06:00* - Continuing clean-up 

*2025-12-31T22:25:56-06:00* - Fixed issue with -S, -R, and -c options 

*2025-12-30T22:31:28-06:00* - clean-up 

*2025-12-30T22:19:52-06:00* - Continuing clean-up 

*2025-12-30T18:31:04-06:00* - Clean-up 

*2025-12-30T17:03:30-06:00* - Clean-up 

*2025-12-30T16:41:52-06:00* - Continuing clean-up 

*2025-12-29T23:45:40-06:00* - Continuing clean-up 

*2025-12-29T21:56:00-06:00* - Fix typeo 

*2025-12-29T20:01:03-06:00* - Typeo correction 

*2025-12-29T18:07:41-06:00* - Added section on external executables 

*2025-12-29T17:01:05-06:00* - Form logic corrections 

*2025-12-29T00:58:07-06:00* - Update README and FAQ 

*2025-12-29T00:42:41-06:00* - Add name known as 

*2025-12-29T00:38:21-06:00* - Refine descriptions for receiver and provider 

*2025-12-29T00:33:22-06:00* - L and C options reversed 

*2025-12-28T22:45:25-06:00* - Continuing clean-up 

*2025-12-28T22:13:45-06:00* - Continuing clean-up 

*2025-12-28T21:47:59-06:00* - Continuing clean-up 

*2025-12-28T19:42:34-06:00* - Continuing clean-up 

*2025-12-28T18:47:31-06:00* - Continuing clean-up 

*2025-12-28T18:26:10-06:00* - Continuing updates 

*2025-12-28T17:16:07-06:00* - Added FAQ section to doc 

*2025-12-27T20:35:15-06:00* - Additional extras 

*2025-12-27T19:56:27-06:00* - Continuing clean-up 

*2025-12-27T18:38:42-06:00* - Continui 

*2025-12-27T17:39:54-06:00* - Continuing clean-up 

*2025-12-27T16:44:38-06:00* - Continuing clean-up 

*2025-12-27T14:13:20-06:00* - Continuing clean-up 

*2025-12-27T10:40:49-06:00* - Updated Help Screen to reflect option changes 

*2025-12-27T10:12:14-06:00* - Changed -c, and -S options, and added -R 

*2025-12-26T23:17:52-06:00* - Continuing clean-up 

*2025-12-26T22:53:02-06:00* - Continuing clean-up 

*2025-12-26T13:45:37-06:00* - Back to some level of sanity 

*2025-12-25T22:32:35-06:00* - Continuing updates 

*2025-12-25T21:40:06-06:00* - This set of patches may be buggy. Substantial changes. 

*2025-12-25T18:26:53-06:00* - Continuing clean-up 

*2025-12-25T16:58:28-06:00* - View screen geometry 

*2025-12-25T16:48:46-06:00* - View screen geometry corrected 

*2025-12-25T09:55:57-06:00* - Can't use treesitter with main.m (no parser) 

*2025-12-24T22:21:45-06:00* - Continuing clean-up 

*2025-12-24T22:19:47-06:00* - Continuing clean-up 

*2025-12-23T22:41:15-06:00* - Continuing clean-up 

*2025-12-23T22:39:24-06:00* - Continuing clean-up 

*2025-12-22T22:25:20-06:00* - Continuing clean-up 

*2025-12-21T23:08:58-06:00* - Add extras.md to doc 

*2025-12-21T23:07:43-06:00* - Add extras.md to doc 

*2025-12-21T23:05:48-06:00* - Added extras directory 

*2025-12-21T22:42:38-06:00* - Continuing clean-up 

*2025-12-21T19:04:38-06:00* - Continuing clean-up 

*2025-12-21T16:00:36-06:00* - Continuing clean-up 

*2025-12-21T14:57:37-06:00* - Continuing clean-up 

*2025-12-20T22:10:07-06:00* - Continuing clean-up 

*2025-12-20T22:08:33-06:00* - Continuing clean-up 

*2025-12-20T21:23:31-06:00* - Continuing clean-up 

*2025-12-19T10:22:22-06:00* - fix view KEY_UP disabling itself by setting view->f_bod 

*2025-12-18T23:22:38-06:00* - Clean-up 

*2025-12-18T22:49:58-06:00* - Continuing clean-up 

*2025-12-17T18:29:35-06:00* - Clean-up continues 

*2025-12-17T18:27:27-06:00* - Clean-up continues 

*2025-12-17T10:04:38-06:00* - Continuing clean-up 

*2025-12-17T07:19:03-06:00* - Expand line buffer to accommodate 2048 byte lines 

*2025-12-16T23:15:28-06:00* - Bug Fixes 

*2025-12-16T23:11:34-06:00* - Bug fixes for pick 

*2025-12-16T20:43:48-06:00* - Continuing the cleanup after refactoring. 

*2025-12-14T22:40:33-06:00* - Continuing the clean-up after added features 

*2025-12-09T17:53:58-06:00* - General Cleanup 

*2025-12-09T17:49:29-06:00* - General Cleanup 

*2025-12-09T17:48:31-06:00* - General Cleanup of Repository 

*2025-11-21T14:33:56-06:00* - More updates 

*2025-11-21T13:59:58-06:00* - C-Menu still needs lots of work 

*2025-11-06T21:04:58-06:00* - Update distribution files 

*2025-11-04T18:14:50-06:00* - Major revisions, but not complete yet. 

*2025-10-11T23:28:26-05:00* - Add files via upload 

*2025-10-11T23:07:06-05:00* - Initial commit 