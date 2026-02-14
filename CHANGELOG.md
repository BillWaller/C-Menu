# C-Menu Changelog

## [Released] - 2026-02-04

[0.2.8] - 2026-02-04

- 
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

