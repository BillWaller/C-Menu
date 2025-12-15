cmake_minimum_required(VERSION 4.1)
set(CMAKE_C_FLAGS "-O0 -g3 -Wno-unused-parameter \
    -Wno-missing-field-initializers -Wno-unused-function -DNCURSES_WIDECHAR")

project(cmenu C)

add_library(
  cmenu
  curskeys.c
  dwin.c
  exec.c
  form_engine.c
  fields.c
  futil.c
  init.c
  init_view.c
  mem.c
  menu_engine.c
  mview.c
  opts.c
  parse_menu_desc.c
  pick_engine.c
  scriou.c
  sig.c
  view_engine.c)
set(LIBS ncursesw tinfo m)

add_executable(ckeys ckeys.c)
target_link_libraries(ckeys cmenu ${LIBS})
install(TARGETS ckeys DESTINATION bin)

add_executable(form form.c)
target_link_libraries(form cmenu ${LIBS})
install(TARGETS form DESTINATION bin)

add_executable(enterchr enterchr.c)
target_link_libraries(enterchr cmenu ${LIBS})
install(TARGETS enterchr DESTINATION bin)

add_executable(enterstr enterstr.c)
target_link_libraries(enterstr cmenu ${LIBS})
install(TARGETS enterstr DESTINATION bin)

add_executable(iloan iloan.c)
target_link_libraries(iloan m)
install(TARGETS iloan DESTINATION bin)

add_executable(menu menu.c)
target_link_libraries(menu cmenu ${LIBS})
install(TARGETS menu DESTINATION bin)

add_executable(pick pick.c)
target_link_libraries(pick cmenu ${LIBS})
install(TARGETS pick DESTINATION bin)

add_executable(rsh rsh.c)
target_link_libraries(rsh cmenu ${LIBS})
install(
  TARGETS rsh
  DESTINATION bin
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
              SETUID)
add_executable(view view.c)
target_link_libraries(view cmenu ${LIBS})
install(TARGETS view DESTINATION bin)

add_executable(whence whence.c)
target_link_libraries(whence)
install(TARGETS whence DESTINATION bin)
