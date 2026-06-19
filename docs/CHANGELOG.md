# CHANGELOG

## C-Menu-0.2.9[33m

*2026-06-19T08:46:53-05:00[0m* - view_engine.c: Correct error in panel stack updates. [33m

*2026-06-18T22:25:54-05:00[0m* - The panels feature of NCurses has slightly different requirements for refreshing and updating pads. It is not difficult, but anywhere it is not handled, a blank pad view will be displayed. I will try to catch all of those occurrences and apply fixes. [33m

*2026-06-18T14:56:13-05:00[0m* - After search, View was not displaying its pad_view window, so I added prefresh pad, update_panels, and doupdate() to search function. This assures synchronization between the physical screen and the virtual screen after updating the panels and pad. [33m

*2026-06-18T00:04:04-05:00[0m* - src/lf.c (lightweight find): lf's "-Ho" (hidden files only) option caused directories that were not hidden to be rejected before placing them on the work queue. This meant that lf would not descend into those directories, and thus would not find any hidden files within them. This commit changes the order of the checks so that directories are added to the work queue before checking if they are hidden. This allows lf to descend into all directories, but only display hidden files when the "-Ho" option is used. [33m

*2026-06-17T15:41:15-05:00[0m* - Documentation Updates [33m

*2026-06-17T14:19:26-05:00[0m* - Documentation Updates [33m

*2026-06-17T13:31:08-05:00[0m* - src/lf.c (lightweight file finder): added option "-Ho" to include only hidden files. [33m

*2026-06-17T13:00:39-05:00[0m* - Documentation Updates [33m

*2026-06-17T12:34:44-05:00[0m* - Finally, the recent window management upgrades are coming together. The codebase has been refactored from the old 2 plane (y and x) windowing system that was becoming increasingly difficult to maintain. NCurses panels, along with derived windows and pads not only provides a z axis stack, but does much of the heavy lifting for window management, including handling of overlapping windows, input focus, and efficient redrawing. This refactor has allowed for a significant reduction in code complexity, as well as improved performance and maintainability. The new system is more modular, making it easier to add new features and fix bugs in the future. Overall, this refactor has been a major step forward in improving the window management capabilities of the application, and it sets a solid foundation for future enhancements. [33m

*2026-06-17T00:03:28-05:00[0m* - Continuing bug fixes related to refactoring window management. [33m

*2026-06-16T17:43:34-05:00[0m* - Finally, the flickering is gone. I do know why it was happening, but that was among the easiest things to fix. The difficult part is becoming familiar with a whole new way managing windows with a hierarchy of panels and derived windows. It is clear to me that the old way I was managing windows was becoming unwieldy and C-Menu must be able to scale to achieve its ambitions. I apologize for the bugs, and I expect there will be more as I go through the codebase and refactor it to use panels. I have been testing the changes as I go, but I have not been able to test every possible combination of windows and panels. I have been testing the changes on Linux, and I expect there will be some issues on Windows and MacOS. I will do my best to fix any issues that arise, but I would appreciate any help from the community in testing and reporting bugs. [33m

*2026-06-15T23:42:56-05:00[0m* - Documentation Update [33m

*2026-06-15T23:37:06-05:00[0m* - Beware: There are still bugs in this release as the most recent feature additions involved a complete rewrite of the window management system. Please report any bugs you find to the issue tracker BillWaller/C-Menu on GitHub. If you have any questions about the new features, please ask in the discussions section of the same repository. Thank you for your support and happy menuing! [33m

*2026-06-15T20:40:13-05:00[0m* - Documentation Updates [33m

*2026-06-15T20:34:06-05:00[0m* - Combined Pick and View to create a File Browser. [33m

*2026-06-13T22:25:47-05:00[0m* - Documentation Updates [33m

*2026-06-13T22:22:42-05:00[0m* - Documentation Updates [33m

*2026-06-13T22:20:13-05:00[0m* - Documentation Updates [33m

*2026-06-13T22:18:08-05:00[0m* - Documentation Updates [33m

*2026-06-13T22:12:31-05:00[0m* - Documentation Updates [33m

*2026-06-13T22:00:31-05:00[0m* - Documentation updates [33m

*2026-06-13T19:38:06-05:00[0m* - Minor bug in pick_engine.c fixed. [33m

*2026-06-13T17:05:11-05:00[0m* - Mini bug in Build documentation. [33m

*2026-06-13T17:01:18-05:00[0m* - Synchronized the build instructions in the documentation with the actual build system. Updated the CMakeLists.txt and Makefiles to reflect the changes in the build process. Removed the outdated README.md file from the docs directory as it is no longer relevant. [33m

*2026-06-13T15:49:48-05:00[0m* - Several bug fixes related to panels upgrade. [33m

*2026-06-12T22:23:18-05:00[0m* - Documentation Updates [33m

*2026-06-12T21:41:53-05:00[0m* - The latest tranche of changes to As C-Menu is likely to be the most significant yet, in terms of what it will bring to C-Menu in the form of scalability as it gears up for tackling more sophisticated applications. The changes include reorganizing the window structure with NCurses Panels to make it much easier to introduce new features while improving resilience and maintainability. As C-Menu grew to a staggering 10,000 lines of code, it became clear that the original window structure was too rigid and made it difficult to implement new features without breaking existing functionality. The new window structure allows for more modular and reusable code, making it easier to add new features and maintain the codebase. Additionally, the new structure allows for better performance and scalability, as it can handle larger and more complex applications without sacrificing performance. Overall, these changes are a significant step forward for As C-Menu as it continues to grow and evolve as a powerful and flexible application development toolkit. [33m

*2026-06-12T01:11:44-05:00[0m* - lf had an error that caused sort to fail. [33m

*2026-06-11T18:03:14-05:00[0m* - Brought both Makefiles to synch, and removed the old build scripts. Both Makefile and CMakeLists.txt now support the same targets, and the Makefile is now a GNU Makefile, which is more portable and has better support for parallel builds. The old build scripts were redundant and not well maintained, so they have been removed to simplify the build process. The .gitignore file has been updated to ignore the new build artifacts generated by the Makefile and CMakeLists.txt, such as the compile_commands.json and install_manifest.txt files. This should help keep the repository clean and prevent unnecessary files from being committed. Overall, these changes should improve the build process and make it easier for developers to build and maintain the project. [33m

*2026-06-11T16:06:06-05:00[0m* - Modified the Makefile (GNU) to produce 3 binaries for rsh. "rsh" is dynamically linked, "rsh_static" is statically linked, and "rsh_pam" is dynamically linked with PAM support. The reason for doing so is that you have different requirements depending on the use case, and many developers and administrators have a variety of use cases. For rescue environments, a statically linked binary is often preferred because it does not rely on shared libraries that may not be available. For regular use, a dynamically linked binary is more common and can take advantage of shared libraries for better performance and smaller size. For environments that require PAM support, having a separate binary allows users to choose the appropriate version based on their needs without having to recompile the entire project. This approach provides flexibility and caters to a wider range of use cases while maintaining the integrity of the project. It gives you a selection of tools so you always have the right one for the job. [33m

*2026-06-11T15:17:33-05:00[0m* - Several bug fixes and improvements to the build system, including: Brought the C-Menu build up to date, adding conditional builds for a static executable and PAM support. To build static rsh with CMake, cd to src, and type: cmake -DCMAKE_BUILD_TYPE=Release -DRSH_STATIC=ON . To build rsh with PAM support, cd to src, and type: cmake -DCMAKE_BUILD_TYPE=Release -DRSH_PAM=ON . Then make and make install as usual. Static rsh and rsh with PAM support have vastly different use cases, and it doesn't make sense to build PAM support into a static rsh binary, so these options are mutually exclusive. We may add a CMake option to build both types of executable in a single build in the future, but with different names. The static binary is 3.3 MB compared to 20 KB for dynamic. [33m

*2026-06-10T15:54:58-05:00[0m* - Modified view_engine.c to allow clipping text with mouse. Added horizontal scroll default shift width, which can be overridden by entering a shift width before right or left arrow keys. Shift width sticky and doesn't change until you enter a different shift width before pressing left or right arrow keys. [33m

*2026-06-09T22:02:15-05:00[0m* - Bug fixes and Documentation Updates [33m

*2026-06-09T15:32:35-05:00[0m* - Documentation Updates [33m

*2026-06-09T15:01:34-05:00[0m* - Documentation Updates [33m

*2026-06-09T14:48:11-05:00[0m* - Documentation Updates [33m

*2026-06-09T14:09:44-05:00[0m* - Testing, catching edge cases and improving the overall user experience. This includes refining the color schemes, enhancing the configuration options, and ensuring better compatibility across different environments. The new theme script allows users to easily create and apply custom themes, while the updated documentation provides clearer guidance on how to utilize these features effectively. Additionally, various bug fixes and performance optimizations have been implemented to ensure a smoother and more responsive interface. [33m

*2026-06-09T12:32:26-05:00[0m* - Documentation Updates [33m

*2026-06-09T12:22:03-05:00[0m* - Documentation Updates [33m

*2026-06-09T10:23:29-05:00[0m* - In previous testing, I had commented out the code that restored ncurses windows after executing a command. Therefore, the windows were not being restored after executing a command. Fixed that. My wife said the problem is the loose nut on the keyboard. [33m

*2026-06-08T21:34:49-05:00[0m* - Documentation Updates [33m

*2026-06-08T21:22:30-05:00[0m* - Documentation Updates [33m

*2026-06-08T21:21:38-05:00[0m* - Documentation Updates [33m

*2026-06-08T19:35:54-05:00[0m* - Documentation Upodates [33m

*2026-06-08T19:23:31-05:00[0m* - Added code to load and apply themes instantly without restarting the application. This includes changes to the Makefile, detach, dwin.c, futil.c, include/cm.h, include/common.h, include/pick.h, init.c, mem.c, and pick_engine.c. Additionally, updated the README.md and themes.png to reflect the new theme functionality. [33m

*2026-06-08T11:25:23-05:00[0m* - Added detach command to the Makefile, which allows the user to detach a process from the terminal and run it in the background. The detach command is implemented in the src/detach.c file, which uses the fork() and setsid() system calls to create a new process that is detached from the terminal. [33m

*2026-06-08T11:04:46-05:00[0m* - Documentation Updates, bug fixes. [33m

*2026-06-07T20:37:51-05:00[0m* - Modified init.c to load themes from the themes directory. Added a new script to list themes and another to change themes. Added some themes. Modified init.c to include descriptive comments from argp when writing configuration files. [33m

*2026-06-06T23:55:43-05:00[0m* - Move C-Menu/src/test to C-Menu/src/work [33m

*2026-06-06T20:56:32-05:00[0m* - Themes [33m

*2026-06-06T20:31:06-05:00[0m* - Themes [33m

*2026-06-06T19:32:00-05:00[0m* - Added selection to the C-Menu example main menu, which will start an independent instance of C-Menu in a new terminal window. This is an example of the flexibility of C-Menu, and how it can be used to create multiple menus that can be launched from each other. The new menu is a copy of the main menu, but it can be modified independently to show different options or information. This demonstrates how C-Menu can be used to create complex menu systems with multiple levels and branches, allowing for a more dynamic and interactive user experience. The changes include modifications to the configuration files for the Green, Red, and default themes, as well as changes to the main.m file to add the new menu option and launch the new menu in a new terminal window. Overall, this update enhances the functionality and versatility of the C-Menu example, making it a more powerful tool for creating custom menu interfaces. [33m

*2026-06-06T18:17:22-05:00[0m* - Modified config file (
/menuapp/.minitrc) processing to accommodate comments on configuration lines. This allows users to add comments to their configuration files without breaking the parsing logic. The changes include updating the parsing function to ignore any text following a comment character (e.g., '#') on a line, ensuring that only the actual configuration settings are processed. This enhancement improves the usability of the configuration files by allowing users to document their settings directly within the file. [33m

*2026-06-06T14:26:49-05:00[0m* - Added menu selection to select C-Menu theme. Corrected an error in the C-Menu Kitty theme selector. Added Selections Processed message to Pick. [33m

*2026-06-06T01:28:57-05:00[0m* - After adding so many new features in the past month, I expected lots of bugs, but it hasn't been as bad as I thought. And the bugs are getting easier to fix as the codebase nears the end of the 0.2.9 cycle. I have a few more features to add, and I think the next release will be a good one. The new themes are really nice, and I think users will like them. I did find a significant error in lf file type handling, but that is fixed in this commit. [33m

*2026-06-05T16:09:14-05:00[0m* - Added new colors for the themes, added a new theme, and made some minor adjustments to the code to make it more efficient and easier to read. Also updated the README.md file to reflect the changes made to the themes and the new features added. [33m

*2026-06-05T02:53:32-05:00[0m* - menuapp updates [33m

*2026-06-05T02:50:31-05:00[0m* - Added color, brackets_fg. [33m

*2026-06-05T02:13:08-05:00[0m* - Makefile and documentation update [33m

*2026-06-04T16:47:02-05:00[0m* - Preview of RSH certificate screenshot added to README.md [33m

*2026-06-04T16:35:17-05:00[0m* - General cleanup and update of the C-Menu example application directory. [33m

*2026-06-04T16:26:41-05:00[0m* - Makefile added help, config, and manifest. Added $CMENU_HOME/lib64 to RPATH. RPATH is very useful in a development environment where you may have several versions of a library installed. As C-Menu becomes more mature, we will likely remove the RPATH and rely on the user to set their LD_LIBRARY_PATH. But for now, this makes it easier to test new versions of the library without having to worry about the system version of the library. [33m

*2026-06-04T14:01:35-05:00[0m* - Added optional manifest display at end of Makefile. Activate by setting DISPLAY_MANIFEST to 1 in the Makefile. Deactivate by setting it to 0. This is useful information for debugging and development, but has no effect on the final product. It is not intended for end users, and may be removed in future versions. [33m

*2026-06-04T11:01:58-05:00[0m* - Corrected error in parse_menu_desc.c so that choice letters designated with "-" are properly reserved, preventing some letters from occurring more than once in a menu. Also updated cmenu manual page to explain the handling of input and output pipes in direct execution mode. [33m

*2026-06-04T00:25:45-05:00[0m* - Substituted native linux install command for the inst.sh script in  the Makefile in the interest of standardization and convenience of developers who are familiar with the native linux install command. [33m

*2026-06-03T16:10:14-05:00[0m* - Added color definition for Form's fill character, fill_char_fg. Fixed artifact from previous commit that caused Form fields not to update properly. [33m

*2026-06-03T10:55:46-05:00[0m* - Documentation Updates [33m

*2026-06-03T09:12:46-05:00[0m* - Add manual page for rsh. [33m

*2026-06-03T09:09:17-05:00[0m* - Documentation Updates [33m

*2026-06-02T20:35:44-05:00[0m* - Documentation Updates, Unicode and wide character processing in form. [33m

*2026-06-01T14:17:30-05:00[0m* - Added a new menu command type, "dexe", which detaches from the current process and executes a command. This allows for running simultaneous concurrent processes without blocking the menu system. The implementation involves changes to the command execution logic, menu description parsing, and the menu engine to support this new command type. [33m

*2026-06-01T10:59:47-05:00[0m* - Documentation Updates [33m

*2026-06-01T10:53:26-05:00[0m* - lf manual page [33m

*2026-06-01T10:42:32-05:00[0m* - Add lf manual link to README.md [33m

*2026-06-01T10:36:20-05:00[0m* - Remove spurious line in lf. [33m

*2026-06-01T10:23:57-05:00[0m* - Added lf manual page, updated lf help files, screenshots, and the Makefile to include a section to build and install manual pages. [33m

*2026-05-31T13:22:18-05:00[0m* - Documentation Updates [33m

*2026-05-31T13:18:20-05:00[0m* - Documentation Updates [33m

*2026-05-31T00:32:22-05:00[0m* - Documentation Updates [33m

*2026-05-30T23:05:58-05:00[0m* - Documentation Updates [33m

*2026-05-30T21:17:21-05:00[0m* - Documentation Updates [33m

*2026-05-30T18:26:11-05:00[0m* - Various and sundry cleanup and fixes. [33m

*2026-05-30T14:04:02-05:00[0m* - Documentation Updates [33m

*2026-05-30T13:35:07-05:00[0m* - Corrected bug in Form command processing [33m

*2026-05-29T23:46:22-05:00[0m* - Documentation Updates [33m

*2026-05-29T20:52:04-05:00[0m* - Documentation Updates [33m

*2026-05-29T20:17:31-05:00[0m* - Menu application directory general cleanup. Added Main Menu selection to edit menu description files. [33m

*2026-05-29T15:38:44-05:00[0m* - Cleaned up lf "-D1" output. [33m

*2026-05-28T22:36:28-05:00[0m* - Documentation Updates [33m

*2026-05-28T22:21:42-05:00[0m* - Documentation Updates [33m

*2026-05-28T18:25:20-05:00[0m* - Testing the new help system and adding a few screenshots to the docs. [33m

*2026-05-28T00:39:26-05:00[0m* - Documentation Updates [33m

*2026-05-28T00:12:12-05:00[0m* - Applied changes to test program C-Menu/test/iso8601.c. [33m

*2026-05-27T23:50:23-05:00[0m* - Documentation Updates [33m

*2026-05-27T21:45:54-05:00[0m* - Documentation Updates [33m

*2026-05-27T21:19:03-05:00[0m* - Resolved lf date/time issues with Co-Pilot's help. The date/times at issue were the after and before date/times defining the range of files to be included in lf's output. I discovered this issue using the "-D18" option, which reveals the date/time information captured from the command line. The option processing code captured the date/times entered, but the verification date/times printed in init_find were off by by an hour for date/times in the CDT domain. The problem was that tm1_isdst must be initialized to -1 before calling mktime, which is used to convert the date/time information into a time_t value. By setting tm1_isdst to -1, mktime can correctly determine whether daylight saving time is in effect for the given date/time, which resolves the issue with the date/time being off by an hour. After making this change, the date is now correctly formatted and displayed in the application. This fix ensures that users will see the correct date information without any formatting errors. [33m

*2026-05-27T19:56:59-05:00[0m* - Documentation Updates [33m

*2026-05-27T19:51:29-05:00[0m* - Documentation Updates [33m

*2026-05-27T19:48:01-05:00[0m* - Bug fixes and documentation updates. [33m

*2026-05-27T11:28:01-05:00[0m* - I suspended organized testing during the last tranche of feature additions and improvements in the interest of expedient completion. As a result, I have a backlog of testing. I am adding more test cases to ensure that the code is robust and can handle a wide variety of operating conditions. This commit fixes several issues exposed by examining the output of lf using the "-D18" option to verify option processing. I will continue to test and fix any bugs that I find, and add more test cases to ensure that the code is robust and can handle a wide variety of inputs and scenarios. I am committed to ensuring that the code is of high quality and meets the needs of users, and I will continue to work diligently to achieve that goal. Thank you for your patience and support as I continue to improve the code and add more features. [33m

*2026-05-26T17:33:04-05:00[0m* - Clean-up following scan-build report. lf: Corrected thread number calculation. [33m

*2026-05-26T12:56:26-05:00[0m* - Switching back to alternate screen buffer after running nvim as a subprogram. This is to deal with screen buffer corruption when exiting C-Menu after running nvim as a subprogram. [33m

*2026-05-26T11:55:01-05:00[0m* - lf: fixed error in argument processing, which could cause regular expression failure and SIGSEGV (core dumped) [33m

*2026-05-25T22:30:18-05:00[0m* - Update C-Menu application directory [33m

*2026-05-25T21:42:45-05:00[0m* - lf: corrected issues with exclusion regex leading to SIGSEGV [33m

*2026-05-25T17:49:37-05:00[0m* - Documentation Updates [33m

*2026-05-25T16:04:43-05:00[0m* - Menu: Due to some menu's running out of key characters that make sense, I have removed the restriction on lower case letters and some special characters. The characters  "!" through "
" (0x21 - 0x7e) are now valid key characters. Menu uses a rudamentary algorithm to determine the key character for a menu item. It scans the menu text for the first character that is not a space, not already reserved, and in the valid key character range. If no such character is found, Menu will select the first unreserved letter in the valid range, even though it may not be in the menu text. You may specify any character by including it immediately (no space separation) after a dash ("-") as the first non-blank character in the the menu item text. Lower case ("q") is always reserved for the hidden "Quit", "Exit", "Return", etc. menu item. As an additional visual queue, key characters in the menu text will be displayed using "nt_hl_rev_fg" and "nt_hl_rev_bg" color pair. These colors are defined as six-digit hex RGB values in the C-Menu configuration (normally 
/menuapp/.minitrc). [33m

*2026-05-25T10:27:08-05:00[0m* - Added color definitions to specify reverse and highlight for Menu items. These are nt_fg, nt_bg, nt_hl_fg, nt_hl_bg, nt_hl_rev_fg, nt_hl_rev_bg, where nt is for normal text, hl is highlighted, rev is reverse, and fg for background and bg for background. [33m

*2026-05-22T13:44:36-05:00[0m* - lf: Continuing optimizations and enhancements [33m

*2026-05-20T20:24:01-05:00[0m* - lf: Continuing performance enhancements and adding tests [33m

*2026-05-19T23:41:23-05:00[0m* - lf: added several features and performance enhancements [33m

*2026-05-19T02:04:51-05:00[0m* - lf: continuing performance improvements, added features [33m

*2026-05-17T20:01:19-05:00[0m* - Documentation Updates [33m

*2026-05-17T18:54:22-05:00[0m* - Experimental feature additions and optimizations. [33m

*2026-05-16T14:36:38-05:00[0m* - Documentation Updates [33m

*2026-05-16T14:04:47-05:00[0m* - CHANGELOG updates [33m

*2026-05-16T13:45:35-05:00[0m* - lf: Attempts to integrate mmap into lf resulted in degraded performanc. As a result, the mmap implementation has been removed and the previous file handling method has been restored. This change aims to improve the overall performance of lf while maintaining its functionality. [33m

*2026-05-14T23:01:43-05:00[0m* - Documentation Update [33m

*2026-05-14T22:51:11-05:00[0m* - lf performance tuning [33m

*2026-05-14T14:36:56-05:00[0m* - lf corrected conditionals grouping errors in file type deselector [33m

*2026-05-13T23:25:03-05:00[0m* - Further lf optimizations and Documentation updates [33m

*2026-05-13T01:34:28-05:00[0m* - lf performance tuning [33m

*2026-05-12T13:22:03-05:00[0m* - Documentation Updates [33m

*2026-05-12T13:01:06-05:00[0m* - lf performance tuning. Set default number of threads to 6 if available. Added -T option to specify number of threads. Using more threads isn't always better, and can actually decrease performance, I have 16 threads available, but testing showed that 6 threads gave the best performance. Use the time command and pay most attention to real time, not user or system time. You can specify the number of threads you want to use with the -T option if you want to experiment with it. lf checks the number of threads available on the system on the system and limits the user setting accordingly. lf's -D (debug) option displays the number of threads being used. [33m

*2026-05-12T01:09:47-05:00[0m* - lf: include directory names by default [33m

*2026-05-12T01:07:11-05:00[0m* - lf: corrected error in finder loop [33m

*2026-05-11T23:51:01-05:00[0m* - Documentation Updates [33m

*2026-05-11T01:02:51-05:00[0m* - Documentation Updates [33m

*2026-05-11T00:51:37-05:00[0m* - lf default was to follow symbolic links to directories. New default is to not follow symbolic links to directories. The -L option has been added to follow symbolic links to directories. [33m

*2026-05-10T23:45:39-05:00[0m* - Documentation Updates [33m

*2026-05-10T17:28:09-05:00[0m* - lf: Changed default to exclude hidden files, removed -n option to exclude hidden files, and added the -H option to include hidden files. Updated the help file and screenshot accordingly. [33m

*2026-05-10T11:44:15-05:00[0m* - lf: tidying up the coded added for concurrent directory processing. Further benchmark testing indicates the concurrent directory processing provides the expected performance gains, and in all cases so far, C-Menu's lf finder is outperforming fd and find, and in some cases significantly so. [33m

*2026-05-10T00:13:48-05:00[0m* - Added concurrent directory processing to lf using a thread pool and a worker to manage thread queues. Threads are mutex protected to ensure safe access to shared resources. The implementation includes a thread pool for efficient task management and a worker to handle the distribution of tasks among threads. This enhancement allows for faster processing of directories by utilizing multiple threads concurrently, improving the overall performance of the lf application. The changes include modifications to the Makefile, the addition of thread management code in futil.c, and updates to the header file cm.h to support the new threading functionality. The documentation has also been updated to reflect these changes. [33m

*2026-05-08T08:27:52-05:00[0m* - Documentation Updates [33m

*2026-05-08T00:31:43-05:00[0m* - Documentation Updates [33m

*2026-05-07T17:43:37-05:00[0m* - Documentation Updates [33m

*2026-05-07T15:05:15-05:00[0m* - Identified a logic error in lf that was causing it to recurse into lf_process for regular files. This induced quite a performance penalty on lf because lf_process was being called for all files, regardless of whether they were directories or not. The fix was to rearrange the comparisons of the controlling if statement. This prevents the unnecessary recursion and allows lf to function correctly when processing regular files. [33m

*2026-05-07T00:01:50-05:00[0m* - Documentation Updates [33m

*2026-05-06T23:59:56-05:00[0m* - Documentation Updates [33m

*2026-05-06T23:33:43-05:00[0m* - Added -S (sort) option to lf. -Sr sorts in reverse order. [33m

*2026-05-06T12:22:08-05:00[0m* - Documentation Updates and minor bug fixes [33m

*2026-05-05T18:39:49-05:00[0m* - amort, the executable receiver for the Form example only had the format yyyy-mm-dd specified in strptime, and the default output of Form is unformatted without the "-". As a reault the amortization View output had incorrect dates. Added a second format, Yyyymmdd, to make it more robust and it's working now. [33m

*2026-05-05T18:19:27-05:00[0m* - Documentation Updates [33m

*2026-05-05T18:06:35-05:00[0m* - Reworked form_engine exec_receiver_cmd to make it more robust like pick. Added simple amortization program to demonstrate Form's -R execute_receiver_cmd functionality [33m

*2026-05-05T10:22:39-05:00[0m* - Form was failing to display field brackets. Assumed it was either an error in ncurses or a loose nut on my computer and rearranged placements of wnoutrefresh calls to manipulate screen updates in an order that worked. [33m

*2026-05-05T00:48:48-05:00[0m* - Documentation Updates [33m

*2026-05-04T20:59:35-05:00[0m* - Documentation Updates [33m

*2026-05-04T19:32:32-05:00[0m* - popups.c remove superfluous new_form [33m

*2026-05-04T18:51:33-05:00[0m* - Documentation Updates [33m

*2026-05-04T18:45:37-05:00[0m* - Documentation Updates [33m

*2026-05-04T17:22:00-05:00[0m* - Documentation Updates [33m

*2026-05-04T17:09:03-05:00[0m* - Documentation Updates [33m

*2026-05-04T17:04:54-05:00[0m* - form_fields.c pressing escape in field editor resulted in endless loop. Added validation for printable ASCII characters to prevent this. Documentation Updates [33m

*2026-05-03T17:49:43-05:00[0m* - Documentation Updates [33m

*2026-05-03T11:53:19-05:00[0m* - Documentation Updates [33m

*2026-05-03T10:55:25-05:00[0m* - Documentation Updates [33m

*2026-05-03T00:43:48-05:00[0m* - Documentation Updates [33m

*2026-05-03T00:36:47-05:00[0m* - pick_engine.c: remove toggle selection on enter key [33m

*2026-05-02T23:15:34-05:00[0m* - Documentation Updates [33m

*2026-05-02T23:14:15-05:00[0m* - Documentation Updates [33m

*2026-05-02T23:08:18-05:00[0m* - Documentation Updates [33m

*2026-05-02T22:58:53-05:00[0m* - Documentation Updates [33m

*2026-05-02T21:39:57-05:00[0m* - Documentation Updates [33m

*2026-05-02T15:27:56-05:00[0m* - Documentation Updates [33m

*2026-05-01T20:19:52-05:00[0m* - Documentation Updates [33m

*2026-05-01T19:06:41-05:00[0m* - Documentation Updates [33m

*2026-05-01T19:05:08-05:00[0m* - Documentation Updates [33m

*2026-05-01T14:00:49-05:00[0m* - Documentation Updates [33m

*2026-04-30T22:36:52-05:00[0m* - Documentation Updates [33m

*2026-04-30T15:14:08-05:00[0m* - Documentation Updates [33m

*2026-04-30T13:52:08-05:00[0m* - Added screenshots to Rustlings exercises and updated the exercises.md file to reflect the changes. The new screenshots show the output of the exercises after completing them, while the old screenshots have been removed as they were outdated. The exercises.md file has been modified to include the new screenshots and to provide a better visual representation of the exercises. This update will help users understand the expected output of the exercises and make it easier for them to follow along with the Rustlings course. [33m

*2026-04-30T11:50:08-05:00[0m* - Remove hard-coded paths from main.m [33m

*2026-04-30T10:56:50-05:00[0m* - Provide tilde expansion for external commands. [33m

*2026-04-29T20:58:43-05:00[0m* - Documentation Updates [33m

*2026-04-29T11:50:13-05:00[0m* - Documentation Updates [33m

*2026-04-28T10:03:14-05:00[0m* - Documentation Updates [33m

*2026-04-28T08:46:19-05:00[0m* - Added "Tab Pick" and "Tab Edit" to chyron. [33m

*2026-04-27T23:58:27-05:00[0m* - pick_engine: accept line editor field on enter key [33m

*2026-04-27T22:11:28-05:00[0m* - pick_engine: pick->tbl_pages calculation is correct in line editor main loop. Also fixed updating of pick->tbl_pages on info line. [33m

*2026-04-27T21:52:17-05:00[0m* - pick_engine: fixed pick->tbl_pages calculation [33m

*2026-04-27T16:59:55-05:00[0m* - Fixed fork_exec root shell. Fixed spurious mouse click in object selection window triggering selection of the first object in the list. [33m

*2026-04-27T12:30:40-05:00[0m* - pick_engine revert search term to previous state if change would result in match failure [33m

*2026-04-27T07:45:11-05:00[0m* - Removed spurious setting of pick->d_cnt in pick_engine.c, which was causing the pick engine to display only the first page of objects. [33m

*2026-04-26T20:06:43-05:00[0m* - Fix child processes inheriting stderr and corrupting ncurses display. [33m

*2026-04-26T17:57:35-05:00[0m* - Before all execvp calls, redirect stderr to /dev/null to prevent corruption of ncurses windows. [33m

*2026-04-26T16:19:33-05:00[0m* - Redirect child output to /dev/null after fork and before execvp to prevent the child from corrupting the ncurses display. Fixes a bug where the child process could write to the terminal and mess up the display of the parent process. [33m

*2026-04-26T13:08:07-05:00[0m* - pick_engine: minor cosmetic update [33m

*2026-04-26T12:33:55-05:00[0m* - pick_engine: track number of pages [33m

*2026-04-26T10:13:09-05:00[0m* - Pick Engine: Turn cursor off in inactive window [33m

*2026-04-25T20:54:11-05:00[0m* - lf: correcting issues with arguments [33m

*2026-04-25T20:02:21-05:00[0m* - if lf cannot validate the first positional argument as directory, but determines it is a valid regular expression, it should also check the second positional argument, and if it is a valid regular expression, reject the first positional argument. [33m

*2026-04-25T19:38:57-05:00[0m* - Fixed failure of lf to detect symlink to valid directory. [33m

*2026-04-25T16:28:57-05:00[0m* - Pick Engine: refinement of information display. [33m

*2026-04-25T15:55:03-05:00[0m* - Pick Engine: added line and page numbers to separator line. [33m

*2026-04-25T15:00:16-05:00[0m* - Pick Engine, window width error. [33m

*2026-04-25T08:34:46-05:00[0m* - Pick Engine: Accept mouse clicks from line editor window when object selector is active. [33m

*2026-04-25T08:10:28-05:00[0m* - Increase width of Pick Window to accommodate chyron. [33m

*2026-04-24T19:05:01-05:00[0m* - Pick Engine. Various improvements and bug fixes. [33m

*2026-04-24T10:54:24-05:00[0m* - Documentation Updates [33m

*2026-04-24T10:01:18-05:00[0m* - In menu.c, move _atexit() after curses initialization because _atexit() calls destroy_curses(). [33m

*2026-04-24T07:51:45-05:00[0m* - Pick Engine set y_offset to 0 when not used. [33m

*2026-04-24T07:13:10-05:00[0m* - Pick Engine line editor field positioning errors corrected. Added logic to make object selector and line editor cede control when a mouse click comes from other than their assigned window. [33m

*2026-04-23T22:15:56-05:00[0m* - Documentation Updates [33m

*2026-04-23T22:03:00-05:00[0m* - Documentation Updates [33m

*2026-04-23T20:37:41-05:00[0m* - Documentation Updates [33m

*2026-04-23T10:09:01-05:00[0m* - Pick_engine, fine tuning the odds and ends. [33m

*2026-04-23T09:28:41-05:00[0m* - Fixed bug in Pick Engine. Pressing the page down key when there is only one page failed to set in_key to 0 resulting in endless loop. [33m

*2026-04-22T14:16:57-05:00[0m* - Added wrefresh to line editor in Pick Engine. [33m

*2026-04-22T13:35:33-05:00[0m* - Documentation Update [33m

*2026-04-22T13:31:24-05:00[0m* - Adjusted Pick Engine box geometry to remove extra background line. [33m

*2026-04-22T10:51:17-05:00[0m* - Pick Engine redisplay window 2 after displaying help. [33m

*2026-04-22T06:31:43-05:00[0m* - Somehow lost wmouse_trafo() in dxwgetch, which caused the mouse to not work in dxwgetch. Reinstating it. [33m

*2026-04-21T21:22:02-05:00[0m* - Documentation Updates [33m

*2026-04-21T19:48:21-05:00[0m* - Continue fine tuning Pick Engine documentation and screenshots. [33m

*2026-04-21T16:33:35-05:00[0m* - Documentation Updates [33m

*2026-04-21T16:07:17-05:00[0m* - Continuing work on Pick Engine. [33m

*2026-04-21T15:38:29-05:00[0m* - Reworked Pick Engine logic to be more efficient and handle edge cases better. Updated the user interface to provide clearer feedback on pick results. Refactored code for improved readability and maintainability. Added comments to explain complex sections of the code. [33m

*2026-04-21T10:02:11-05:00[0m* - Update object cursor position. [33m

*2026-04-20T21:39:24-05:00[0m* - Still more tuning on Pick Engine. [33m

*2026-04-20T21:21:03-05:00[0m* - Continuing to fine tune the pick engine. [33m

*2026-04-20T17:17:32-05:00[0m* - Fine tuning Pick's line editor. [33m

*2026-04-20T08:49:42-05:00[0m* - Valgrind documentation update Merge: 1475850 5f7fa6d [33m

*2026-04-20T07:17:59-05:00[0m* - Merge pull request #4 from BillWaller/copilot/fix-valgrind-still-reachable Fix Valgrind still-reachable leaks: free new_init() allocations, suppress ncurses internals [33m

*2026-04-20T03:28:42Z[0m* - Fix Valgrind still-reachable leaks: free new_init allocs, add suppression file, Makefile target, and docs Agent-Logs-Url: https://github.com/BillWaller/C-Menu/sessions/79b9264f-614c-417f-b8d5-a1a6aab29c5f Co-authored-by: BillWaller <10166578+BillWaller@users.noreply.github.com> [33m

*2026-04-20T03:23:51Z[0m* - Initial plan [33m

*2026-04-19T22:07:29-05:00[0m* - Fixing valgrind errors. [33m

*2026-04-19T21:57:17-05:00[0m* - Chasing down valgrind errors and fixing them. [33m

*2026-04-19T20:15:55-05:00[0m* - Extensively reworked the Pick Engine to make it much more intuitive and easier to use. The new API is much more consistent and easier to understand, and the new implementation is much more efficient and easier to maintain. Specifically, a line editor has been added to the Pick Engine to allow users to enter search strings. The Pick Engine automatically filters the list of items displayed in real time, as you type. It's sort of like Telescope, except that it maintains distinct modes for the Picker and Editor to preserve the individual features and optimizations of each. As a result, Pick delivers a powerful multi-column, multi-page Picker and a full-function line editor. [33m

*2026-04-18T22:11:35-05:00[0m* - Fixed bug in mem.c which was causing view to fail when reading input from a command line pipe. [33m

*2026-04-17T20:43:42-05:00[0m* - Cleaning up possible memory leaks in lf.c and futil.c, and fixing a minor issue in the Makefile. [33m

*2026-04-17T15:06:30-05:00[0m* - Complex characters must be initialized with proper wide character strings and mbstate must be initialized to zeros. [33m

*2026-04-17T07:22:04-05:00[0m* - lf after and before date/time changes, and some refactoring to make the code more readable and maintainable. [33m

*2026-04-16T19:10:39-05:00[0m* - lf -i does not have an argument. [33m

*2026-04-16T19:02:24-05:00[0m* - Documentation Updates [33m

*2026-04-16T19:00:34-05:00[0m* - Documentation Updates [33m

*2026-04-16T18:26:38-05:00[0m* - Add -s, file_size, option to lf. For example, lf -t f -s 100k, will only list files greater than or equal to 100k. [33m

*2026-04-16T11:01:14-05:00[0m* - Added after and before date ranges to lf. [33m

*2026-04-14T15:31:36-05:00[0m* - Fixed bug in lf. Erroneously used continue where I should have suppressed. [33m

*2026-04-14T14:40:09-05:00[0m* - Added -u, --user option to lf. Usage is -u user_name. I am trying to keep lf as simple as possible, but this seems like a pretty important capability. Also changed some instances of "-N f" to "-Nf" in the code. This became an error when I added a boolean argument to the -N option without removing the space between N and f. [33m

*2026-04-14T11:23:17-05:00[0m* - Documentation Updates [33m

*2026-04-14T11:03:24-05:00[0m* - After adding OPTION_ARG_OPTIONAL to the "-N" option of view, argp interpreted the "f" of "-N f" as a separate argument, causing C-Menu to fail as it tried to open "f" as a file". Reading the documentation for argp, this is normal and expected behavior. The solution, as suggested by the documentation, is to eliminate the space after the -N option. The scripts will be changed and the documentation will clarify that the -N option should be used without a space when providing an argument. I also ran across a problem caused by popup_view using the function arguments for line, col, begy, and begx, when command line arguments were also specified. The desired behavior os to override the function arguments with the command line arguments when they are provided. I have also modified the code to check for the presence of command line arguments and use them instead of the function arguments when they are available. [33m

*2026-04-13T18:39:03-05:00[0m* - More minor fixes for View line numbering [33m

*2026-04-13T15:00:52-05:00[0m* - Fine tuning line numbering toggle [33m

*2026-04-12T20:11:26-05:00[0m* - Documentation Updates [33m

*2026-04-12T12:15:39-05:00[0m* - Fixed View trying to open line number window when line number flag set to false. [33m

*2026-04-12T06:35:51-05:00[0m* - More work on resizing View windows [33m

*2026-04-11T21:03:43-05:00[0m* - Documentation Updates [33m

*2026-04-11T20:52:57-05:00[0m* - Working on View window resizing. It's not perfect yet, but we are making progress. As always, if you have any suggestions or feedback, please let me know. I am open to any ideas that can help improve the resizing behavior and overall user experience. Thank you for your continued support and contributions to the project. Let's keep pushing forward and making this software better together! [33m

*2026-04-10T15:36:55-05:00[0m* - Expand view line number window to 8 characters wide. [33m

*2026-04-10T14:17:15-05:00[0m* - Expanded line number window to 7 digits. To accommodate more than 7 digits, we will need to allow scalinig of the line number window. [33m

*2026-04-10T13:45:35-05:00[0m* - Corrected line numbering issue in view when scrolling up and then down. [33m

*2026-04-10T11:05:18-05:00[0m* - Don't activate cursor after deleting a window [33m

*2026-04-10T11:00:49-05:00[0m* - Erase and refresh stdscr after execvp() [33m

*2026-04-10T10:52:16-05:00[0m* - Replace old submenu code with popup_menu() [33m

*2026-04-10T08:27:20-05:00[0m* - Correct color rendition in view [33m

*2026-04-10T08:03:10-05:00[0m* - set menu to init->menu to avoid SIGSEGV [33m

*2026-04-10T07:32:29-05:00[0m* - Set background to NORMAL when deleting windows [33m

*2026-04-10T05:43:44-05:00[0m* - Uncommented endwin() in destroy_curses() to prevent the terminal from being left in an unusable state if the program crashes. [33m

*2026-04-09T21:42:19-05:00[0m* - Documentation Updates [33m

*2026-04-09T19:44:49-05:00[0m* - Reset terminal on exit [33m

*2026-04-09T18:31:10-05:00[0m* - Add atexit and end_pgm functions to menu.c to ensure proper cleanup of resources when the program exits. This will help prevent memory leaks and ensure that any necessary finalization steps are performed before the program terminates. [33m

*2026-04-09T18:14:55-05:00[0m* - Taking care of valgrind issues. While I am working hard to chase down memory leaks, the fixes may have side-effects leading to other issues. Be assured, I am addressing all known issues as quickly as possible. [33m

*2026-04-09T15:20:01-05:00[0m* - Fixed fastbin chunk detected [33m

*2026-04-09T15:16:08-05:00[0m* - Valgrind fixes, and some other minor cleanups. See individual commits for details. [33m

*2026-04-09T11:31:35-05:00[0m* - Off by one free error in destroy_view. [33m

*2026-04-09T11:21:46-05:00[0m* - Fixed memory leak in view. [33m

*2026-04-08T23:49:19-05:00[0m* - Minor fixes [33m

*2026-04-08T16:43:56-05:00[0m* - Documentation Updates [33m

*2026-04-08T13:53:35-05:00[0m* - Removed -a (List all files, including hidden) option from lf and added -n (Don't list hidden files) to conform with find's default of listing all files. The -n option is equivalent to find . -not -name ".*" [33m

*2026-04-08T13:10:19-05:00[0m* - Changed lf default depth to unlimited, in conformance with find. lf did have a default depth of 3 because it was handy for initial testing. As it no longer serves a purpose, the default depth is 0, which means unlimited depth like find. My next quandry is whether to change the behavior of lf concerning hidden files. By default, lf ignores hidden files, while find includes them. I am leaning towards changing lf to include hidden files by default. [33m

*2026-04-08T10:22:38-05:00[0m* - Documentation Updates [33m

*2026-04-08T10:18:31-05:00[0m* - Documentation Updates [33m

*2026-04-08T08:22:56-05:00[0m* - Documentation Updates [33m

*2026-04-08T07:17:06-05:00[0m* - Fixed [33m

*2026-04-07T22:45:43-05:00[0m* - Fixed errors in fork_exec(). [33m

*2026-04-07T14:45:19-05:00[0m* - Documentation Updates [33m

*2026-04-07T14:13:22-05:00[0m* - Documentation Updates [33m

*2026-04-07T14:11:38-05:00[0m* - Documentation Updates [33m

*2026-04-07T14:10:10-05:00[0m* - Documentation Updates [33m

*2026-04-07T13:56:52-05:00[0m* - Documentation Updates [33m

*2026-04-07T13:32:16-05:00[0m* - Documentation Updates [33m

*2026-04-07T13:11:28-05:00[0m* - Documentation Updates [33m

*2026-04-07T10:30:02-05:00[0m* - Documentation Updates [33m

*2026-04-07T10:07:55-05:00[0m* - Documentation Updates [33m

*2026-04-07T09:05:27-05:00[0m* - Documentation Updates [33m

*2026-04-07T09:03:17-05:00[0m* - Documentation Updates [33m

*2026-04-07T08:48:48-05:00[0m* - Documentation Updates [33m

*2026-04-07T08:46:14-05:00[0m* - Documentation Updates [33m

*2026-04-07T08:45:04-05:00[0m* - Documentation Updates [33m

*2026-04-07T08:17:33-05:00[0m* - Documentation updates. [33m

*2026-04-07T08:14:14-05:00[0m* - Documentation Updates [33m

*2026-04-06T23:15:29-05:00[0m* - Documentation updates. [33m

*2026-04-06T22:56:32-05:00[0m* - Documentation Updates. [33m

*2026-04-06T22:54:15-05:00[0m* - Documentation Updates [33m

*2026-04-06T22:14:49-05:00[0m* - Documentation Updates [33m

*2026-04-06T21:36:45-05:00[0m* - Update Installation Guide [33m

*2026-04-06T20:50:43-05:00[0m* - Clean up script errors [33m

*2026-04-06T20:30:22-05:00[0m* - Resolving issues between NCurses and Neovim handling of alternate and main screens related to smcup and rmcup. This commit includes changes to the Makefile, menu_engine.c, and pick_engine.c to ensure compatibility and proper handling of screen states in both environments. [33m

*2026-04-06T16:02:05-05:00[0m* - Fixed several errors in scripts. [33m

*2026-04-06T15:27:18-05:00[0m* - Documentation Updates. [33m

*2026-04-06T15:25:23-05:00[0m* - Documentation updates. [33m

*2026-04-06T13:00:39-05:00[0m* - Reworked menu_engine logic, added c23 standard to .clangd, made SIGSEGV handler messages more informative. [33m

*2026-04-05T23:11:28-05:00[0m* - Fixed bug in waiting for process. Added confirmation of program execution. [33m

*2026-04-05T11:09:59-05:00[0m* - Clear screen before execvp. [33m

*2026-04-04T20:55:16-05:00[0m* - Cleaned up entry and exit for fork-exec executables and added checks for directories in workstation setup scripts. [33m

*2026-04-04T13:46:26-05:00[0m* - Clear before execvp. [33m

*2026-04-04T13:41:24-05:00[0m* - Clear artifacts from screen before and after fork_exec. [33m

*2026-04-04T09:43:01-05:00[0m* - Documentation Updates [33m

*2026-04-04T04:58:54-05:00[0m* - pick_engine - pick->obj_idx was being erroneously set to 0 in picker main loop. [33m

*2026-04-03T21:05:44-05:00[0m* - Tuning some of the fine details. [33m

*2026-04-03T15:15:08-05:00[0m* - When starting a full screen root shell from the main.m menu, the ncurses screen was not being cleared, and the root shell was being launched on top of the ncurses screen. This was because the clear() function was not being called before launching the root shell. To fix this issue, I added a call to clear() before launching the root shell in the main.m file. This ensures that the ncurses screen is cleared before the root shell is launched, providing a clean and clear interface for the user. [33m

*2026-04-03T12:00:28-05:00[0m* - Added a complete user session with Form to the User Guide. [33m

*2026-04-03T00:14:36-05:00[0m* - Update menuapp files. [33m

*2026-04-02T22:16:28-05:00[0m* - Catching the edge cases now. I would love to have some help locating bugs so 0.2.9 can be wrapped up soon. I have been testing the new features and they seem to be working well, but as the designer, I am functionally fixed. If you have some time, please try out the latest version and make a list of things that don't work they way they should. I will be working on the list and trying to resolve as many of the issues as I can. I was a lead developer for many years, so I an no newby to criticism. I know what kinds of bugs live in the shadows of every software project. No one shapes a software product more than the early adopters. Theirs is perhaps the greatest contribution to great software. [33m

*2026-04-02T15:30:12-05:00[0m* - Valgrinding and fixing some memory leaks and other minor issues. Also added some comments to the code for better readability. [33m

*2026-04-02T11:17:50-05:00[0m* - My tree-sitter parser for shell scripts isn't working properly, so i removed the "sh" entries from source queries. [33m

*2026-04-02T11:12:31-05:00[0m* - Fixed bug in form_engine related to F5 process. [33m

*2026-04-02T10:41:18-05:00[0m* - Documentation update [33m

*2026-04-02T10:31:29-05:00[0m* - Clean up screen artifacts after running various processes in C-Menu. [33m

*2026-04-02T09:20:13-05:00[0m* - Prevent nvim from leaving artifacts on screen. [33m

*2026-04-02T08:10:42-05:00[0m* - Added help files to Menu, Form, and Pick and cleaned up the code to display help. [33m

*2026-04-02T06:22:20-05:00[0m* - Included form help file and fixed bug in form_engine. [33m

*2026-04-02T04:32:51-05:00[0m* - Fixed bug in form engine help. [33m

*2026-04-01T22:06:10-05:00[0m* - Documentation updates. [33m

*2026-04-01T21:56:46-05:00[0m* - Documentation updates. [33m

*2026-04-01T21:32:26-05:00[0m* - Documentation updates. [33m

*2026-04-01T21:23:32-05:00[0m* - Documentation updates. [33m

*2026-04-01T21:21:31-05:00[0m* - Documentation Updates [33m

*2026-04-01T17:03:45-05:00[0m* - Tidy up the chyron a bit. [33m

*2026-04-01T15:09:50-05:00[0m* - Further testing proved that "wchar_t wstr[2] = {L'\0', L'\0'};" will work for "setcchar(&cc, wstr, WA_NORMAL, cpx, nullptr);", so re-reverted to wstr. [33m

*2026-04-01T14:55:23-05:00[0m* - Changing final setcchar(&cc, &wc, WA_NORMAL, cpx, nullptr) to setcchar(&cc, wstr, WA_NORMAL, cpx, nullptr) lost termination of cmplx_buf. Reverted back to scalar wc. [33m

*2026-04-01T14:03:13-05:00[0m* - Replace all ocrrurences of scalar wc with wstr [33m

*2026-04-01T13:52:18-05:00[0m* - Remove unnecessary code from whence [33m

*2026-04-01T11:21:52-05:00[0m* - pick_engine.c - highlight selection when toggled on with mouse [33m

*2026-04-01T07:47:12-05:00[0m* - Added wclrtoeol() at end of chyron [33m

*2026-04-01T07:43:02-05:00[0m* - Removed extra space at end of chyron. [33m

*2026-03-31T19:56:36-05:00[0m* - Added set_chyron_key_cp to insert/overwrite toggle switch in the field editor. [33m

*2026-03-31T19:35:38-05:00[0m* - Added color pair to to set_chyron_key so that certain keys can be highlighted. This is used as a hint to the user as to the next logical key to press or click with the mouse. [33m

*2026-03-31T13:04:36-05:00[0m* - Fixed bug in pick_engine related to argp processing. Essentially, argp provides all the non-option arguments to be captured in an array not including argv[0]. The getopt code returned an array of all arguments with an index to the first non-option argument. This caused the pick_engine code to fail when it tried to access the non-option arguments as it was accessing the wrong indices. The fix was to adjust the indices in pick_engine to account for the fact that argp does not include argv[0] in the array of non-option arguments. This should resolve the issue and allow pick_engine to function correctly with argp. [33m

*2026-03-30T18:39:10-05:00[0m* - More clean-up and documentation updates. [33m

*2026-03-30T17:45:07-05:00[0m* - Refactoring some function calls to use the new API, and removing some unused code. [33m

*2026-03-30T17:34:53-05:00[0m* - Resolved more issues with argp conversion. [33m

*2026-03-30T14:24:41-05:00[0m* - Incorporate argp into whence [33m

*2026-03-30T12:36:34-05:00[0m* - Fixed another error resulting from switching to argp from getopt. There is nothing wrong with the argp library. These issues are solely my fault for not testing the code more thoroughly after switching to argp. I will be more careful in the future. [33m

*2026-03-30T11:28:14-05:00[0m* - Fixed off-by-one error introduced by migration to argp in commit 9c8e5b1. The error caused the last argument to be ignored, which was the path to the form description file. This involved setting optind to state->next - 1 instead of state->next. [33m

*2026-03-29T23:56:02-05:00[0m* - Fixed bugs related to upgrading from getopt to argp. [33m

*2026-03-28T19:16:54-05:00[0m* - Switched from getopt to argp for better argument parsing and help message generation. This change improves the user experience by providing clearer usage instructions and more robust handling of command-line arguments. The new implementation also allows for easier maintenance and future enhancements to the argument parsing logic. [33m

*2026-03-28T06:55:01-05:00[0m* - Documentation Updates [33m

*2026-03-27T23:20:38-05:00[0m* - Documentation Updates [33m

*2026-03-27T23:13:53-05:00[0m* - Documentation Updates [33m

*2026-03-27T20:40:11-05:00[0m* - Delete C-Menu-0.2.9-Linux-x86_64.tar.xz Signed-off-by: Bill Waller <billxwaller@gmail.com> [33m

*2026-03-27T18:28:22-05:00[0m* - Add files via upload Signed-off-by: Bill Waller <billxwaller@gmail.com> [33m

*2026-03-27T18:16:34-05:00[0m* - Documentation Updates [33m

*2026-03-27T16:18:24-05:00[0m* - Testing revealed some off-by-one errors in view. Fixed all that I know about. [33m

*2026-03-26T19:42:30-05:00[0m* - Fixed menu calling popup_view instead of mview, which caused SIGSEGV on exit. [33m

*2026-03-26T18:17:48-05:00[0m* - Making the build a little cleaner [33m

*2026-03-26T18:00:46-05:00[0m* - view clrtobot at eod [33m

*2026-03-26T17:21:37-05:00[0m* - Restore cursor at end of program [33m

*2026-03-26T17:11:28-05:00[0m* - Cosmetic improvement to view [33m

*2026-03-26T17:05:30-05:00[0m* - Removed artifacts from bottom line of line number window [33m

*2026-03-26T15:13:23-05:00[0m* - Fixed several bugs arising from refactoring xwgetch() to expand its usefulness for managing wait states, synchronizing forked processes, and catching signals. [33m

*2026-03-26T00:04:43-05:00[0m* - Refresh pad after selecting -n line numbering from command line [33m

*2026-03-25T21:46:50-05:00[0m* - Refactor field input after xwgetch changes [33m

*2026-03-25T13:02:54-05:00[0m* - Remove mview and updated CMakeLists.txt and Makefile [33m

*2026-03-25T12:35:54-05:00[0m* - Removed unneeded source files menu.c, form.c, pick.c, view.c, and ckeys.c [33m

*2026-03-25T11:09:57-05:00[0m* - Combined menu, form, pick, view, and ckeys into a single executable, main, which substitutes for menu, form, pick, view, and ckeys when executed via symbolic links of those same names to main. Created a module, popups.c, that contains function calls to menu, form, pick, view, and ckeys, which will function as pop-ups and drop-downs within the main executable. [33m

*2026-03-24T22:59:29-05:00[0m* - Put #ifdefs in rsh to make SSH authentication and system logging optional. [33m

*2026-03-24T21:15:33-05:00[0m* - Changelog Updates [33m

*2026-03-24T20:37:31-05:00[0m* - RSH Activating SSH Authentication [33m

*2026-03-24T19:50:35-05:00[0m* - Added ssh authentication and system logging to RSH [33m

*2026-03-24T12:13:05-05:00[0m* - Documentation Updates [33m

*2026-03-24T11:04:45-05:00[0m* - Fix view display_page clearing last line. [33m

*2026-03-24T10:54:59-05:00[0m* - Get rid of flickering in view display_page by moving wclrtobot to after the last line is displayed. [33m

*2026-03-24T09:14:41-05:00[0m* - Documentation Updates [33m

*2026-03-24T05:38:50-05:00[0m* - Remove DEBUG from cm.h [33m

*2026-03-24T05:35:33-05:00[0m* - Fine tuning of wait timing [33m

*2026-03-23T21:20:31-05:00[0m* - Testing and debugging of the pick engine and the view initialization. [33m

*2026-03-23T19:44:43-05:00[0m* - Resolve timing issues with pick and view engines [33m

*2026-03-23T19:28:06-05:00[0m* - Synchronize forked processes [33m

*2026-03-23T18:03:28-05:00[0m* - Update to init_view waiting display [33m

*2026-03-23T17:55:27-05:00[0m* - Working with waiting pop-ups [33m

*2026-03-23T17:49:22-05:00[0m* - Further work on the wait displays [33m

*2026-03-23T16:22:28-05:00[0m* - Add -rdynamic option to debug builds [33m

*2026-03-23T16:04:39-05:00[0m* - Fine tuning the timeout and fixing a bug. [33m

*2026-03-23T15:02:13-05:00[0m* - Added pop-up wait message with countdown timer in view and pick while waiting for input from provider processes. Default timeout is 5 seconds. [33m

*2026-03-22T08:46:21-05:00[0m* - Documentation Updates [33m

*2026-03-21T22:54:39-05:00[0m* - Suppress "File of" if input is stdin [33m

*2026-03-21T15:01:00-05:00[0m* - Documentation Updates [33m

*2026-03-21T11:50:11-05:00[0m* - Documentation Updates [33m

*2026-03-21T00:11:08-05:00[0m* - Toggle line numbers with "-n" in view [33m

*2026-03-20T22:35:52-05:00[0m* - Documentation Updates [33m

*2026-03-20T22:10:32-05:00[0m* - Documentation Updates [33m

*2026-03-20T13:50:45-05:00[0m* - Documentation Updates [33m

*2026-03-20T09:06:11-05:00[0m* - Documentation Updates [33m

*2026-03-20T08:50:16-05:00[0m* - General clean-up of the code [33m

*2026-03-19T23:36:07-05:00[0m* - Fixed bug in xwgetch that caused it to miss mouse clicks. [33m

*2026-03-19T21:37:22-05:00[0m* - Fix bug in View vertical scrolling with mouse wheel [33m

*2026-03-19T21:19:15-05:00[0m* - Fixed mouse wheel vertical scrolling in view_engine. [33m

*2026-03-19T14:49:02-05:00[0m* - Documentation Updates [33m

*2026-03-19T14:41:26-05:00[0m* - Documentation Updates [33m

*2026-03-19T14:36:17-05:00[0m* - Documentation Updates [33m

*2026-03-19T14:32:05-05:00[0m* - Documentation Updates [33m

*2026-03-19T14:30:49-05:00[0m* - Documentation Updates [33m

*2026-03-19T14:21:47-05:00[0m* - Documentation Updates [33m

*2026-03-18T22:59:34-05:00[0m* - Fixed some logic errors in view related to page positioning. [33m

*2026-03-18T21:48:51-05:00[0m* - Fine tuning the paging logic [33m

*2026-03-18T18:20:38-05:00[0m* - Corrected Page Up logic in view [33m

*2026-03-18T14:24:44-05:00[0m* - fixed Pick failing to operate on the last item in the object list. This was caused by an off-by-one error in the loop that iterates through the objects. The loop condition was checking for `i < num_objects - 1` instead of `i < num_objects`, which caused it to skip the last object. This fix changes the loop condition to `i < num_objects`, allowing Pick to properly operate on all objects in the list, including the last one. [33m

*2026-03-18T13:39:29-05:00[0m* - Close line number window properly to clear artifact [33m

*2026-03-18T10:57:07-05:00[0m* - free data structures view->ln_tbl, view->ln_win [33m

*2026-03-17T23:02:47-05:00[0m* - Documentation Updates [33m

*2026-03-17T22:50:19-05:00[0m* - Added line number option (-N) to view command. [33m

*2026-03-14T11:32:38-05:00[0m* - lf enhanced to selectively list any combination of 8 file types. [33m

*2026-03-12T21:14:40-05:00[0m* - Documentation Updates [33m

*2026-03-12T20:57:41-05:00[0m* - Documentation Updates [33m

*2026-03-12T18:12:54-05:00[0m* - Documentation Updates [33m

*2026-03-12T18:10:16-05:00[0m* - Documentation Updates [33m

*2026-03-12T00:20:24-05:00[0m* - fixed bug in lf -a not showing hidden files [33m

*2026-03-10T17:23:07-05:00[0m* - Documentation Updates [33m

*2026-03-10T16:30:40-05:00[0m* - Documentation Updates [33m

*2026-03-10T16:29:27-05:00[0m* - Documentation Updates [33m

*2026-03-10T16:24:39-05:00[0m* - Documentation Updates [33m

*2026-03-10T14:37:53-05:00[0m* - Documentation Updates [33m

*2026-03-10T14:34:33-05:00[0m* - Documentation Updates [33m

*2026-03-10T09:30:41-05:00[0m* - Documentation Updates [33m

*2026-03-10T07:59:40-05:00[0m* - Prevent lf from following links [33m

*2026-03-09T17:55:46-05:00[0m* - Modified scripts to sort output [33m

*2026-03-09T17:47:22-05:00[0m* - Field editor refinements/cleanup [33m

*2026-03-09T12:00:35-05:00[0m* - Removed intentional segmentation fault used for testing. This function was designed to verify that the program would core dump instead of continuing to run after a SIGSEGV interrupt and it functioned correctly, breaking the program as designed. It is a testing artifact not intended for production code. [33m

*2026-03-09T11:55:25-05:00[0m* - Fixed bug in field editor insert mode. [33m

*2026-03-09T11:06:46-05:00[0m* - Modified lf to use current directory instead of complaining if first non-option argument is not a valid directory. [33m

*2026-03-08T22:16:45-05:00[0m* - Fixed problem with lf [33m

*2026-03-08T18:27:43-05:00[0m* - Hopefully fixed stubborn bug in lf [33m

*2026-03-08T16:06:42-05:00[0m* - Many bug fixes and improvements, including: integrating chyron into form and pick, fixing a long-standing bug in the form engine, improving the pick engine. Fixed another bug in lf. [33m

*2026-03-07T11:08:34-06:00[0m* - Added section on lf [33m

*2026-03-07T10:01:54-06:00[0m* - Documentation Updates [33m

*2026-03-07T09:14:31-06:00[0m* - Corrected Pick KEY_DOWN behavior [33m

*2026-03-06T18:58:15-06:00[0m* - Additions to Changelog [33m

*2026-03-06T18:15:43-06:00[0m* - Upgraded the menuapp and src code with various modifications, including changes to the sddm_chbg.sh script, main.m file, and several C source files. Additionally, a new file diag.m was added to the menuapp/msrc directory, and an old file xx was deleted. These changes aim to enhance the functionality and performance of the application while maintaining compatibility with existing features. The chyron functions, which have been updated to handle mouse events, will now provide a more interactive user experience. The modifications in the form_engine.c and related files will improve the overall efficiency of the form handling process,while the updates in the include files will ensure better code organization and maintainability. Overall, these changes contribute to a more robust and user-friendly application, aligning with the project's goals and objectives. [33m

*2026-03-06T12:16:41-06:00[0m* - Added incomplete message [33m

*2026-03-06T10:16:56-06:00[0m* - Documentation Updates [33m

*2026-03-05T20:22:02-06:00[0m* - Documentation Updates [33m

*2026-03-05T18:45:14-06:00[0m* - Bug fixes, exercises, scripts and screenshots [33m

*2026-03-05T14:02:06-06:00[0m* - Fixed bugs in menu and added exclusion regex to lf. [33m

*2026-03-03T21:16:58-06:00[0m* - Documentation Updates [33m

*2026-03-02T14:25:58-06:00[0m* - Replace NULL with nullptr [33m

*2026-03-01T19:58:26-06:00[0m* - Pick failing to draw box window if no title [33m

*2026-03-01T09:18:23-06:00[0m* - Documentation Updates [33m

*2026-03-01T09:16:23-06:00[0m* - Documentation Updates [33m

*2026-02-28T13:55:51-06:00[0m* - Documentation Updates [33m

*2026-02-28T13:53:30-06:00[0m* - Documentation Updates [33m

*2026-02-27T21:55:31-06:00[0m* - Perfect sub-window positioning [33m

*2026-02-27T21:52:15-06:00[0m* - Documentation Updates [33m

*2026-02-27T20:29:01-06:00[0m* - Code and Documentation inmprovements [33m

*2026-02-27T09:40:27-06:00[0m* - Use new options with lf [33m

*2026-02-27T09:33:18-06:00[0m* - Fixed another issue with lf [33m

*2026-02-27T08:50:16-06:00[0m* - Fixed lf (list files) [33m

*2026-02-26T19:57:38-06:00[0m* - Documentation Updates [33m

*2026-02-26T19:42:31-06:00[0m* - Documentation Updates [33m

*2026-02-26T11:05:52-06:00[0m* - Documentation Updates [33m

*2026-02-26T11:02:24-06:00[0m* - Documentation Updates [33m

*2026-02-25T23:47:54-06:00[0m* - Added Desktop startup files [33m

*2026-02-25T22:52:59-06:00[0m* - Correct transposition of iso8601 [33m

*2026-02-25T17:48:44-06:00[0m* - Documentation Updates [33m

*2026-02-24T20:44:48-06:00[0m* - Documentation Updates [33m

*2026-02-24T20:35:23-06:00[0m* - Documentation Updates [33m

*2026-02-24T14:34:09-06:00[0m* - Corrected search page-top, page-bottom positions [33m

*2026-02-24T12:59:37-06:00[0m* - Corrected issue with lf -d, (depth) [33m

*2026-02-24T12:15:49-06:00[0m* - Corrected problem with lf [33m

*2026-02-24T07:49:23-06:00[0m* - Move chktty to bin [33m

*2026-02-24T07:47:19-06:00[0m* - Documentation Updates [33m

*2026-02-23T16:03:24-06:00[0m* - Added message if tty directory not found [33m

*2026-02-23T15:50:01-06:00[0m* - View reverse search logic fixes [33m

*2026-02-22T16:39:57-06:00[0m* - Make include directives more portable for API use [33m

*2026-02-22T16:20:25-06:00[0m* - Correcting type-o [33m

*2026-02-22T16:07:16-06:00[0m* - More work on CMake build system [33m

*2026-02-22T12:41:43-06:00[0m* - Remove installed include and lib64 directories [33m

*2026-02-22T12:25:46-06:00[0m* - CMake - update installation logic [33m

*2026-02-22T12:02:07-06:00[0m* - Install CMenu.conf in ld.so.conf [33m

*2026-02-21T09:45:55-06:00[0m* - Documentation Updates [33m

*2026-02-21T09:43:29-06:00[0m* - Documentation Updates [33m

*2026-02-21T09:15:19-06:00[0m* - View Window Geometry [33m

*2026-02-20T14:48:25-06:00[0m* - Reworking View's search logic [33m

*2026-02-19T16:57:06-06:00[0m* - Fixed problem with multi-byte conv. mbrtowc() [33m

*2026-02-19T12:27:28-06:00[0m* - Documentation Updates [33m

*2026-02-19T11:44:46-06:00[0m* - Documentation Updates [33m

*2026-02-18T20:15:00-06:00[0m* - Added environment variable for static rsh build [33m

*2026-02-18T15:41:26-06:00[0m* - Added option to statically link rsh [33m

*2026-02-18T12:11:31-06:00[0m* - Corrected issue with View wrapping [33m

*2026-02-17T20:49:33-06:00[0m* - Add .bashrc to .gitignore [33m

*2026-02-17T20:35:02-06:00[0m* - Remove GH_TOKEN [33m

*2026-02-17T20:05:25-06:00[0m* - Deleted GH_TOKEN from .bashrc [33m

*2026-02-17T20:03:14-06:00[0m* - Logic correction in view write buffer to file [33m

*2026-02-16T20:20:44-06:00[0m* - CMake Build install directory [33m

*2026-02-16T20:01:20-06:00[0m* - Documentation Updates [33m

*2026-02-16T19:51:46-06:00[0m* - Added logic for editing current buffer [33m

*2026-02-15T19:26:51-06:00[0m* - Library versioning CMake and Makefile [33m

*2026-02-15T13:49:03-06:00[0m* - Correct Misspelling in menu.m [33m

*2026-02-15T11:38:43-06:00[0m* - Fix several programs that weren't expanding tilde [33m

*2026-02-15T10:01:33-06:00[0m* - Documentation Updates [33m

*2026-02-13T22:37:54-06:00[0m* - Documentation Updates [33m

*2026-02-13T21:40:17-06:00[0m* - Documentation Updates [33m

*2026-02-13T21:19:00-06:00[0m* - Documentation Updates [33m

*2026-02-13T21:17:27-06:00[0m* - Documentation Updates [33m

*2026-02-13T21:12:22-06:00[0m* - Documentation Updates [33m

*2026-02-12T23:07:57-06:00[0m* - Documentation Updates [33m

*2026-02-12T22:53:24-06:00[0m* - Documentation Updates [33m

*2026-02-12T19:27:52-06:00[0m* - Improving Help display [33m

*2026-02-12T17:09:50-06:00[0m* - Improved Signal Handling, Help mechanism [33m

*2026-02-11T18:53:32-06:00[0m* - Documentation Updates [33m

*2026-02-11T18:49:57-06:00[0m* - Documentation Updates [33m

*2026-02-11T18:39:58-06:00[0m* - Documentation Updates [33m

*2026-02-11T18:02:41-06:00[0m* - Documentation Updates [33m

*2026-02-11T18:07:44-06:00[0m* - Add files via upload Signed-off-by: Bill Waller <billxwaller@gmail.com> [33m

*2026-02-11T18:06:14-06:00[0m* - Add files via upload Signed-off-by: Bill Waller <billxwaller@gmail.com> [33m

*2026-02-11T15:38:26-06:00[0m* - Update C-Menu API link to use HTTPS Signed-off-by: Bill Waller <billxwaller@gmail.com> [33m

*2026-02-11T14:50:33-06:00[0m* - Documentation Updates [33m

*2026-02-10T22:34:27-06:00[0m* - Reorganize files, added doxygen API [33m

*2026-02-08T23:39:17-06:00[0m* - Documentation Updates [33m

*2026-02-06T23:51:51-06:00[0m* - Documentation Updates [33m

*2026-02-06T20:56:59-06:00[0m* - Documentation Updates [33m

*2026-02-06T15:11:12-06:00[0m* - Documentation Update [33m

*2026-02-06T14:58:50-06:00[0m* - Processing of mview parameters, Documentation [33m

*2026-02-05T18:06:14-06:00[0m* - Corrections to Makefile and CMake install paths [33m

*2026-02-05T09:44:19-06:00[0m* - Added dependencies to make install [33m

*2026-02-05T09:41:10-06:00[0m* - Misspelling in Makefile [33m

*2026-02-05T00:36:06-06:00[0m* - Mogrify Images [33m

*2026-02-05T00:27:31-06:00[0m* - Documentation Updates [33m

*2026-02-04T09:07:22-06:00[0m* - Documentation Updates [33m

*2026-02-03T21:30:17-06:00[0m* - Calculate Window Size if zero [33m

*2026-02-03T13:19:07-06:00[0m* - API placed in libcm.so [33m

*2026-01-31T19:57:53-06:00[0m* - Documentation Updates [33m

*2026-01-31T18:25:55-06:00[0m* - Documentation Updates [33m

*2026-01-31T16:45:04-06:00[0m* - View Engine Improvements and Bug Fixes [33m

*2026-01-31T09:37:11-06:00[0m* - Corrected errors in View Search Logic [33m

*2026-01-30T23:27:38-06:00[0m* - Refine horizontal scrolling behavior in view [33m

*2026-01-30T19:15:08-06:00[0m* - Modified Horizantal Scrolling default to pagewidth [33m

*2026-01-30T10:26:25-06:00[0m* - Documentation Updates [33m

*2026-01-29T23:48:30-06:00[0m* - Documentation Updates [33m

*2026-01-29T14:47:36-06:00[0m* - Documentation Updates [33m

*2026-01-28T14:11:55-06:00[0m* - Throw an error for bad ANSI escape sequences [33m

*2026-01-28T11:41:44-06:00[0m* - Documentation Updates [33m

*2026-01-28T10:35:06-06:00[0m* - Documentation Updates [33m

*2026-01-28T09:44:42-06:00[0m* - Documentation updates and bug fixes [33m

*2026-01-27T22:56:00-06:00[0m* - Corrected issue with italics attribute [33m

*2026-01-27T13:48:56-06:00[0m* - Remove unneeded debugging code [33m

*2026-01-27T13:13:28-06:00[0m* - Update terminfo documentation [33m

*2026-01-27T12:21:21-06:00[0m* - Tweaking Color Supprt [33m

*2026-01-26T14:44:53-06:00[0m* - Continuing work on view_engine.c [33m

*2026-01-26T13:53:55-06:00[0m* - Corrected issue with input line index [33m

*2026-01-26T13:24:49-06:00[0m* - view-engine fmt_line input handling optimization [33m

*2026-01-26T00:35:58-06:00[0m* - Signal Handler, Documentation Updates [33m

*2026-01-25T00:36:03-06:00[0m* - Delete Unneeded directory [33m

*2026-01-24T23:30:14-06:00[0m* - Documentation Updates [33m

*2026-01-24T22:27:44-06:00[0m* - Documentation Updates [33m

*2026-01-24T22:19:01-06:00[0m* - Documentation Updates [33m

*2026-01-24T20:44:42-06:00[0m* - Improve Signal Handling, Doc Updates [33m

*2026-01-24T10:45:44-06:00[0m* - Documentation Updates [33m

*2026-01-24T10:34:42-06:00[0m* - Documentation Updates [33m

*2026-01-24T10:29:03-06:00[0m* - Documentation Updates [33m

*2026-01-24T09:07:22-06:00[0m* - Corrected Gamma Calculation, added gray gamma [33m

*2026-01-23T22:48:59-06:00[0m* - Documentation Updates [33m

*2026-01-23T18:41:53-06:00[0m* - Documentation Updates [33m

*2026-01-23T18:35:33-06:00[0m* - Documentation Updates [33m

*2026-01-23T18:33:45-06:00[0m* - Documentation Updates [33m

*2026-01-23T13:20:07-06:00[0m* - Documentation Updates [33m

*2026-01-22T21:49:10-06:00[0m* - Documentation Updates [33m

*2026-01-22T18:44:17-06:00[0m* - Documentation Updates [33m

*2026-01-22T17:25:03-06:00[0m* - Documentation Updates [33m

*2026-01-22T17:19:10-06:00[0m* - Documentation Updates [33m

*2026-01-22T12:46:28-06:00[0m* - Documentation Updates [33m

*2026-01-22T12:43:46-06:00[0m* - Documentation Updates [33m

*2026-01-22T12:35:16-06:00[0m* - Documentation Updates [33m

*2026-01-22T00:54:41-06:00[0m* - Clean-up of obsolete test and work files [33m

*2026-01-21T23:45:27-06:00[0m* - Documentation Updates [33m

*2026-01-21T20:31:13-06:00[0m* - Documentation Updates [33m

*2026-01-21T18:12:53-06:00[0m* - Documentation Updates [33m

*2026-01-21T17:42:02-06:00[0m* - Corrected issues with ANSI to Complex Char [33m

*2026-01-21T12:02:35-06:00[0m* - Added Terminal Emulator Example Configurations [33m

*2026-01-21T11:55:01-06:00[0m* - Documentation Updates [33m

*2026-01-21T10:42:00-06:00[0m* - Corrected rgb_to_xterm256_idx, gcc -O2 issues [33m

*2026-01-20T21:23:30-06:00[0m* - Conflicting signatures for str_tok_r [33m

*2026-01-20T17:50:53-06:00[0m* - Update str_tok_r [33m

*2026-01-20T16:52:49-06:00[0m* - Documentation Updates [33m

*2026-01-20T12:37:38-06:00[0m* - Documentation Updates [33m

*2026-01-20T11:45:25-06:00[0m* - Documentation Updates [33m

*2026-01-19T20:34:55-06:00[0m* - Documentation Updates [33m

*2026-01-19T20:30:30-06:00[0m* - Documentation Updates [33m

*2026-01-19T20:24:24-06:00[0m* - Documentation Updates [33m

*2026-01-19T20:15:44-06:00[0m* - Documentation Updates [33m

*2026-01-19T17:11:25-06:00[0m* - Documentation Updates [33m

*2026-01-19T17:02:03-06:00[0m* - File handling and memory management issues. [33m

*2026-01-18T19:27:11-06:00[0m* - Documentation Updates [33m

*2026-01-18T18:15:59-06:00[0m* - Documentation Updates [33m

*2026-01-18T12:15:47-06:00[0m* - Documentation Updates [33m

*2026-01-18T11:42:56-06:00[0m* - Documentation Updates [33m

*2026-01-18T10:18:45-06:00[0m* - Documentation Updates [33m

*2026-01-17T23:05:46-06:00[0m* - Documentation Updates [33m

*2026-01-17T22:20:52-06:00[0m* - Documentation Updates [33m

*2026-01-17T22:12:13-06:00[0m* - Documentation Updates [33m

*2026-01-17T19:58:45-06:00[0m* - Documentation Updates [33m

*2026-01-17T12:43:19-06:00[0m* - Documentation Updates [33m

*2026-01-16T21:05:03-06:00[0m* - Documentation Updates [33m

*2026-01-16T21:00:56-06:00[0m* - Documentation Updates [33m

*2026-01-16T17:38:31-06:00[0m* - Documentation Updates [33m

*2026-01-16T17:13:13-06:00[0m* - Documentation Updates [33m

*2026-01-16T01:34:27-06:00[0m* - Documentation Updates [33m

*2026-01-16T00:48:47-06:00[0m* - stripansi - Handle \033[K, API.md Update Doc [33m

*2026-01-15T22:31:41-06:00[0m* - Documentation Updates [33m

*2026-01-15T20:28:49-06:00[0m* - Same as previous commit, except for boxwin [33m

*2026-01-15T19:53:36-06:00[0m* - View - enforce horizontal scrolling limit [33m

*2026-01-15T09:40:43-06:00[0m* - Fixed unable to read stdin in view (ssize_t issue) [33m

*2026-01-14T19:24:32-06:00[0m* - Documentation Updates [33m

*2026-01-14T12:11:44-06:00[0m* - Documentation updates. [33m

*2026-01-14T10:03:33-06:00[0m* - Data type corrections [33m

*2026-01-13T22:58:06-06:00[0m* - Corrections to data types in view [33m

*2026-01-13T21:35:31-06:00[0m* - View - interpret ANSI SGR \033[K correctly [33m

*2026-01-12T16:24:12-06:00[0m* - Documentation updates [33m

*2026-01-09T18:10:51-06:00[0m* - Documentation Updates [33m

*2026-01-09T13:27:19-06:00[0m* - Documentation Updates [33m

*2026-01-09T13:12:51-06:00[0m* - Documentation update [33m

*2026-01-09T12:36:50-06:00[0m* - Documentation Updates [33m

*2026-01-08T23:11:10-06:00[0m* - Added functionality to handle window resizing in the view engine. [33m

*2026-01-08T18:24:44-06:00[0m* - Resolve problems exposed by ccc-analyzer [33m

*2026-01-08T17:52:00-06:00[0m* - Documentation Updates [33m

*2026-01-08T01:07:22-06:00[0m* - Updates [33m

*2026-01-08T00:12:46-06:00[0m* - Documentation update [33m

*2026-01-07T23:18:10-06:00[0m* - Documentation update [33m

*2026-01-07T23:16:08-06:00[0m* - Documentation uppdate [33m

*2026-01-07T23:12:09-06:00[0m* - Documentation update [33m

*2026-01-07T23:06:22-06:00[0m* - Documentation update [33m

*2026-01-07T21:09:26-06:00[0m* - Documentation Updates [33m

*2026-01-07T08:35:43-06:00[0m* - Listed recent changes [33m

*2026-01-07T08:16:06-06:00[0m* - Movement keys [33m

*2026-01-06T23:09:33-06:00[0m* - Documentation updates Merge: 66704b9 0f77a44 [33m

*2026-01-06T22:26:19-06:00[0m* - Merge remote-tracking branch 'refs/remotes/origin/main' [33m

*2026-01-06T22:09:20-06:00[0m* - Clean-up [33m

*2026-01-06T22:07:31-06:00[0m* - Delete doc/extras.html [33m

*2026-01-06T21:56:33-06:00[0m* - Update image paths in extras.html [33m

*2026-01-06T21:52:46-06:00[0m* - Delete doc/screenshots [33m

*2026-01-06T21:41:15-06:00[0m* - Clean-up [33m

*2026-01-06T21:25:48-06:00[0m* - Continuing updates [33m

*2026-01-06T20:55:55-06:00[0m* - Added hjkl to motion keys, added Form entry to View [33m

*2026-01-05T20:09:07-06:00[0m* - Modified image directory in extras.html [33m

*2026-01-05T20:06:08-06:00[0m* - Fix image paths in extras.md Updated image paths in extras.md to remove '../' prefix. [33m

*2026-01-05T19:59:40-06:00[0m* - Move extras to root directory [33m

*2026-01-05T19:54:06-06:00[0m* - Updates for documentation and code comments [33m

*2026-01-04T19:58:12-06:00[0m* - Continuing clean-up [33m

*2026-01-04T11:14:22-06:00[0m* - Updates to CMake build [33m

*2026-01-04T09:03:57-06:00[0m* - Added extras.html file and updated documentation files [33m

*2026-01-03T21:12:52-06:00[0m* - Continuing clean-up [33m

*2026-01-02T18:18:10-06:00[0m* - Additions [33m

*2026-01-02T14:49:19-06:00[0m* - Title banner box view [33m

*2026-01-02T14:12:51-06:00[0m* - Next page test for EOD [33m

*2026-01-02T11:06:25-06:00[0m* - Mouse Support for Function Keys in Form_Engine [33m

*2026-01-02T09:07:40-06:00[0m* - Add -d (depth) option to lf [33m

*2026-01-01T21:52:14-06:00[0m* - Deal with superfluous escape character in input file [33m

*2026-01-01T13:47:37-06:00[0m* - Continuing clean-up [33m

*2026-01-01T08:17:06-06:00[0m* - Continuing clean-up [33m

*2025-12-31T22:25:56-06:00[0m* - Fixed issue with -S, -R, and -c options [33m

*2025-12-30T22:31:28-06:00[0m* - clean-up [33m

*2025-12-30T22:19:52-06:00[0m* - Continuing clean-up [33m

*2025-12-30T18:31:04-06:00[0m* - Clean-up [33m

*2025-12-30T17:03:30-06:00[0m* - Clean-up [33m

*2025-12-30T16:41:52-06:00[0m* - Continuing clean-up [33m

*2025-12-29T23:45:40-06:00[0m* - Continuing clean-up [33m

*2025-12-29T21:56:00-06:00[0m* - Fix typeo [33m

*2025-12-29T20:01:03-06:00[0m* - Typeo correction [33m

*2025-12-29T18:07:41-06:00[0m* - Added section on external executables [33m

*2025-12-29T17:01:05-06:00[0m* - Form logic corrections [33m

*2025-12-29T00:58:07-06:00[0m* - Update README and FAQ [33m

*2025-12-29T00:42:41-06:00[0m* - Add name known as [33m

*2025-12-29T00:38:21-06:00[0m* - Refine descriptions for receiver and provider [33m

*2025-12-29T00:33:22-06:00[0m* - L and C options reversed [33m

*2025-12-28T22:45:25-06:00[0m* - Continuing clean-up [33m

*2025-12-28T22:13:45-06:00[0m* - Continuing clean-up [33m

*2025-12-28T21:47:59-06:00[0m* - Continuing clean-up [33m

*2025-12-28T19:42:34-06:00[0m* - Continuing clean-up [33m

*2025-12-28T18:47:31-06:00[0m* - Continuing clean-up [33m

*2025-12-28T18:26:10-06:00[0m* - Continuing updates [33m

*2025-12-28T17:16:07-06:00[0m* - Added FAQ section to doc [33m

*2025-12-27T20:35:15-06:00[0m* - Additional extras [33m

*2025-12-27T19:56:27-06:00[0m* - Continuing clean-up [33m

*2025-12-27T18:38:42-06:00[0m* - Continui [33m

*2025-12-27T17:39:54-06:00[0m* - Continuing clean-up [33m

*2025-12-27T16:44:38-06:00[0m* - Continuing clean-up [33m

*2025-12-27T14:13:20-06:00[0m* - Continuing clean-up [33m

*2025-12-27T10:40:49-06:00[0m* - Updated Help Screen to reflect option changes [33m

*2025-12-27T10:12:14-06:00[0m* - Changed -c, and -S options, and added -R [33m

*2025-12-26T23:17:52-06:00[0m* - Continuing clean-up [33m

*2025-12-26T22:53:02-06:00[0m* - Continuing clean-up [33m

*2025-12-26T13:45:37-06:00[0m* - Back to some level of sanity [33m

*2025-12-25T22:32:35-06:00[0m* - Continuing updates [33m

*2025-12-25T21:40:06-06:00[0m* - This set of patches may be buggy. Substantial changes. [33m

*2025-12-25T18:26:53-06:00[0m* - Continuing clean-up [33m

*2025-12-25T16:58:28-06:00[0m* - View screen geometry [33m

*2025-12-25T16:48:46-06:00[0m* - View screen geometry corrected [33m

*2025-12-25T09:55:57-06:00[0m* - Can't use treesitter with main.m (no parser) [33m

*2025-12-24T22:21:45-06:00[0m* - Continuing clean-up [33m

*2025-12-24T22:19:47-06:00[0m* - Continuing clean-up [33m

*2025-12-23T22:41:15-06:00[0m* - Continuing clean-up [33m

*2025-12-23T22:39:24-06:00[0m* - Continuing clean-up [33m

*2025-12-22T22:25:20-06:00[0m* - Continuing clean-up [33m

*2025-12-21T23:08:58-06:00[0m* - Add extras.md to doc [33m

*2025-12-21T23:07:43-06:00[0m* - Add extras.md to doc [33m

*2025-12-21T23:05:48-06:00[0m* - Added extras directory [33m

*2025-12-21T22:42:38-06:00[0m* - Continuing clean-up [33m

*2025-12-21T19:04:38-06:00[0m* - Continuing clean-up [33m

*2025-12-21T16:00:36-06:00[0m* - Continuing clean-up [33m

*2025-12-21T14:57:37-06:00[0m* - Continuing clean-up [33m

*2025-12-20T22:10:07-06:00[0m* - Continuing clean-up [33m

*2025-12-20T22:08:33-06:00[0m* - Continuing clean-up [33m

*2025-12-20T21:23:31-06:00[0m* - Continuing clean-up [33m

*2025-12-19T10:22:22-06:00[0m* - fix view KEY_UP disabling itself by setting view->f_bod [33m

*2025-12-18T23:22:38-06:00[0m* - Clean-up [33m

*2025-12-18T22:49:58-06:00[0m* - Continuing clean-up [33m

*2025-12-17T18:29:35-06:00[0m* - Clean-up continues [33m

*2025-12-17T18:27:27-06:00[0m* - Clean-up continues [33m

*2025-12-17T10:04:38-06:00[0m* - Continuing clean-up [33m

*2025-12-17T07:19:03-06:00[0m* - Expand line buffer to accommodate 2048 byte lines [33m

*2025-12-16T23:15:28-06:00[0m* - Bug Fixes [33m

*2025-12-16T23:11:34-06:00[0m* - Bug fixes for pick [33m

*2025-12-16T20:43:48-06:00[0m* - Continuing the cleanup after refactoring. [33m

*2025-12-14T22:40:33-06:00[0m* - Continuing the clean-up after added features [33m

*2025-12-09T17:53:58-06:00[0m* - General Cleanup [33m

*2025-12-09T17:49:29-06:00[0m* - General Cleanup [33m

*2025-12-09T17:48:31-06:00[0m* - General Cleanup of Repository [33m

*2025-11-21T14:33:56-06:00[0m* - More updates [33m

*2025-11-21T13:59:58-06:00[0m* - C-Menu still needs lots of work [33m

*2025-11-06T21:04:58-06:00[0m* - Update distribution files [33m

*2025-11-04T18:14:50-06:00[0m* - Major revisions, but not complete yet. [33m

*2025-10-11T23:28:26-05:00[0m* - Add files via upload [33m

*2025-10-11T23:07:06-05:00[0m* - Initial commit 