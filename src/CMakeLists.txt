cmake_minimum_required(VERSION 4.1)
set(CMAKE_C_FLAGS "-O0 -g3 -Wno-unused-parameter \
    -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR")

project(cmenu C)

add_library(
  cmenu
  fckeys.c
  fmenu.c
  rmenu.c
  mmenu.c
  fpaint.c
  fpick.c
  fview.c
  log.c
  mpick.c
  mview.c
  init.c
  futil.c
  fields.c
  scriou.c
  dwin.c
  log.c)

set(LIBS ncursesw tinfo config)

add_executable(menu menu.c)
target_link_libraries(menu cmenu ${LIBS})

add_executable(paint paint.c)
target_link_libraries(paint cmenu ${LIBS})

add_executable(ckeys ckeys.c)
target_link_libraries(ckeys cmenu ${LIBS})

add_executable(paintfile paintfile.c)
target_link_libraries(paintfile cmenu ${LIBS})

add_executable(cpick cpick.c)
target_link_libraries(cpick cmenu ${LIBS})

add_executable(view view.c)
target_link_libraries(view cmenu ${LIBS})

add_executable(dgraph dgraph.c)
target_link_libraries(dgraph cmenu ${LIBS} m)

add_executable(whence whence.c)
target_link_libraries(whence)

add_executable(iloan iloan.c)
target_link_libraries(iloan m)
