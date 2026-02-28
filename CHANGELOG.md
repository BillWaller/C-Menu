# Changelog

## [Released] - 2026-02-04

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
*   Prevent segmentation fault upon `close()` ([#28](link-to-pr))

## [0.2.8]- 2026-02-04
### Added
*   Initial release of the project ([`a1b2c3d`](link-to-commit))

: https://github.com/BillWAller/C-Menu/releases/tag/C-Menu-0.2.8
: https://github.com/BillWaller/C-Menu/releases/tag/C-Menu-0.2.8

