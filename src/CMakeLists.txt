cmake_minimum_required(VERSION 4.1)

set (CMAKE_C_STANDARD 23)
SET (CMAKE_C_STANDARD_REQUIRED ON)

set(UI "UAL_UI" CACHE STRING "UI Backend selection: UAL_UI, NOTCURSES_UI, or
NCURSES_UI")
set_property(CACHE UI PROPERTY STRINGS UAL_UI NOTCURSES_UI NCURSES_UI)

find_package(PkgConfig REQUIRED)

if(UI STREQUAL "NOTCURSES_UI")
    pkg_check_modules(UI_DEPS REQUIRED notcurses)
else()
    pkg_check_modules(UI_DEPS REQUIRED panelw ncursesw)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE
      "Release"
      CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()

# Apply the -s (strip all symbols) flag specifically to the Release
# configuration
add_link_options($<$<CONFIG:RELEASE>:-s>)

set(CMAKE_INSTALL_PREFIX
    "$ENV{HOME}/menuapp"
    CACHE PATH "Install path prefix" FORCE)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

project(
  CMenu
  VERSION 0.2.9
  LANGUAGES C)


set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib64")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_REMOVE_ENVIRONMENT_RPATH TRUE)
include(GNUInstallDirs)

configure_file("${PROJECT_SOURCE_DIR}/include/version.h.in"
               "${PROJECT_SOURCE_DIR}/include/version.h")

install(FILES "${CMAKE_BINARY_DIR}/compile_commands.json"
        DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories("${PROJECT_SOURCE_DIR}/include"
                    "${PROJECT_SOURCE_DIR}/ui")

set(LIBS ncursesw panelw tinfow m)
add_library(cm SHARED dwin.c exec.c futil.c scriou.c sig.c
    ui/ui_ncurses.c ui/ui_ncurses_draw.c ui/ui_ncurses_input.c)
set_target_properties(cm PROPERTIES VERSION ${PROJECT_VERSION}
                                  SOVERSION ${PROJECT_VERSION_MAJOR})

# Apply pkg-config compile flags and link libraries automatically
target_link_libraries(cm PUBLIC ${UI_DEPS_LIBRARIES})
target_compile_options(cm PRIVATE ${UI_DEPS_CFLAGS_OTHER})

set(COMMON_SRCS
    cm_fields.c
    curskeys.c
    fields.c
    form_engine.c
    init_view.c
    menu_engine.c
    parse_menu_desc.c
    pick_engine.c
    popups.c
    view_engine.c
    init.c
    mem.c)
add_library(CMenu OBJECT ${COMMON_SRCS})

add_executable(menu menu.c)
set_target_properties(menu PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
target_sources(menu PRIVATE $<TARGET_OBJECTS:CMenu>)
target_link_libraries(menu cm ${LIBS})

# 4. Conditional UI Code Injection
# Handles adding conditional UI sources and compile flags
if(UI STREQUAL "UAL_UI")
    target_sources(cm PRIVATE
        ui/ui_ncurses.c
        ui/ui_ncurses_draw.c
        ui/ui_ncurses_input.c
        ui/ui_layout.c
    )
    set(CMAKE_C_FLAGS "-Wall -Wextra -DUAL_UI")
    target_compile_definitions(cm PUBLIC -DUAL_UI -DNCURSES_UI
        -DUAL_LEGACY_COMPAT)
    target_compile_definitions(CMenu PUBLIC -DUAL_UI -DNCURSES_UI
        -DUAL_LEGACY_COMPAT)
    target_compile_definitions(menu PUBLIC -DUAL_UI -DNCURSES_UI
        -DUAL_LEGACY_COMPAT)

elseif(UI STREQUAL "NOTCURSES_UI")
    target_sources(cm PRIVATE
        ui/ui_notcurses.c
        ui/ui_notcurses_draw.c
        ui/ui_notcurses_input.c
        ui/ui_layout.c
    )
    set(CMAKE_C_FLAGS "-Wall -Wextra" -DNOTCURSES_UI)
    target_compile_definitions(cm PUBLIC -DNOTCURSES_UI)
    target_compile_definitions(CMenu PUBLIC -DNOTCURSES_UI)
    target_compile_definitions(menu PUBLIC -DNOTCURSES_UI)
    target_link_libraries(cm PUBLIC tinfo)
endif()



add_executable(enterchr enterchr.c)
set_target_properties(enterchr PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
target_link_libraries(enterchr cm ${LIBS})

add_executable(enterstr enterstr.c)
set_target_properties(enterstr PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
target_link_libraries(enterstr cm ${LIBS})

add_executable(amort amort.c)
set_target_properties(amort PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
target_link_libraries(amort cm ${LIBS})

add_executable(iloan iloan.c)
set_target_properties(iloan PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
target_link_libraries(iloan cm ${LIBS})

add_executable(lf lf.c)
set_target_properties(lf PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
target_link_libraries(lf cm ${LIBS} -pthread)

add_executable(detach detach.c)
set_target_properties(detach PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
target_link_libraries(detach cm ${LIBS})

add_executable(rsh rsh.c)
target_link_libraries(rsh)

add_executable(rsh_static rsh.c)
target_link_options(rsh_static PRIVATE -static)

add_executable(rsh_pam rsh.c)
target_compile_definitions(rsh_pam PRIVATE -DRSH_PAM)
target_link_libraries(rsh_pam pam pam_misc)

install(
  TARGETS rsh rsh_static rsh_pam
  RUNTIME
    DESTINATION bin
    PERMISSIONS
      OWNER_READ
      OWNER_WRITE
      OWNER_EXECUTE
      GROUP_EXECUTE
      WORLD_EXECUTE
      SETUID
      SETGID)

add_executable(stripansi stripansi.c)
set_target_properties(stripansi PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
target_link_libraries(stripansi cm ${LIBS})

add_executable(whence whence.c)
set_target_properties(whence PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
target_link_libraries(whence cm ${LIBS})

install(
  TARGETS menu
          detach
          enterchr
          enterstr
          amort
          iloan
          lf
          stripansi
          whence
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(
  CODE "
  set(APP_PATH \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/menu\")
  set(SYMLINK_PATH \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin\")
  execute_process(COMMAND \${CMAKE_COMMAND} -E create_symlink \${APP_PATH}
\${SYMLINK_PATH}/form)
  execute_process(COMMAND \${CMAKE_COMMAND} -E create_symlink \${APP_PATH}
\${SYMLINK_PATH}/pick)
  execute_process(COMMAND \${CMAKE_COMMAND} -E create_symlink \${APP_PATH}
\${SYMLINK_PATH}/view)
  execute_process(COMMAND \${CMAKE_COMMAND} -E create_symlink \${APP_PATH}
\${SYMLINK_PATH}/ckeys)
")

target_include_directories(
  cm PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

install(
  TARGETS cm
  DESTINATION lib64
  EXPORT cm_export)

install(
  EXPORT cm_export
  DESTINATION lib64/cmake/cm
  FILE cmConfig.cmake)

file(GLOB HEADERS "include/*.h")

install(FILES ${HEADERS} DESTINATION include)

include(GNUInstallDirs)

set(LD_CONF_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.conf")

file(WRITE "${LD_CONF_FILE}" "${CMAKE_INSTALL_FULL_LIBDIR}\n")

if(UNIX AND NOT APPLE)
    install(
    FILES "${LD_CONF_FILE}"
    DESTINATION "/etc/ld.so.conf.d"
    COMPONENT Runtime)
    install(
    CODE "
        if(NOT \"\$ENV{DESTDIR}\")
            execute_process(COMMAND ldconfig)
        endif()
    "
    COMPONENT Runtime)
endif()
