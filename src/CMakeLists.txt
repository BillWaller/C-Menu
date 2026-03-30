cmake_minimum_required(VERSION 4.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(RSH_LD $ENV{RSH_LD})
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/menuapp" CACHE PATH "Install path prefix"
FORCE)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
project(CMenu
    VERSION 0.2.9
    LANGUAGES C
)
set(CMAKE_C_FLAGS "-Wall -Wextra")

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib64")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_REMOVE_ENVIRONMENT_RPATH TRUE)
include(GNUInstallDirs)

configure_file(
    "${PROJECT_SOURCE_DIR}/include/version.h.in"
    "${PROJECT_SOURCE_DIR}/include/version.h"
)

install(FILES "${CMAKE_BINARY_DIR}/compile_commands.json" DESTINATION
"${CMAKE_CURRENT_SOURCE_DIR}")

add_library(cm SHARED dwin.c exec.c futil.c scriou.c sig.c)

set_target_properties(
    cm PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

set(LIBS ncursesw tinfo m)

set(COMMON_SRCS
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
target_sources(menu PRIVATE $<TARGET_OBJECTS:CMenu>)
target_link_libraries(menu cm ${LIBS})

add_executable(enterchr enterchr.c)
target_link_libraries(enterchr cm ${LIBS})

add_executable(enterstr enterstr.c)
target_link_libraries(enterstr cm ${LIBS})

add_executable(iloan iloan.c)
target_link_libraries(iloan cm ${LIBS})

add_executable(lf lf.c)
target_link_libraries(lf cm ${LIBS})

add_executable(rsh rsh.c)
#[[ Remove the Comment lines to enable static linking of rsh
set(RSH_LD -static)
]]
if(RSH_LD STREQUAL "-static")
    #     set_target_properties(rsh PROPERTIES LINK_FLAGS_RELEASE -s)
endif()
target_link_libraries(rsh ${RSH_LD})
install(
  TARGETS rsh
  RUNTIME DESTINATION bin
    PERMISSIONS
      OWNER_READ
      OWNER_WRITE
      OWNER_EXECUTE
      GROUP_EXECUTE
      WORLD_EXECUTE
      SETUID
      SETGID)

add_executable(stripansi stripansi.c)
target_link_libraries(stripansi cm ${LIBS})

add_executable(whence whence.c)
target_link_libraries(whence cm ${LIBS})

install(
  TARGETS menu
          enterchr
          enterstr
          iloan
          lf
          stripansi
          whence
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(CODE "
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

target_include_directories(cm PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS cm
    DESTINATION lib64
    EXPORT cm_export
)

install(EXPORT cm_export
    DESTINATION lib64/cmake/cm
    FILE cmConfig.cmake
)

file(GLOB HEADERS "include/*.h")

install(FILES ${HEADERS} DESTINATION include)

include(GNUInstallDirs)

set(LD_CONF_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.conf")

file(WRITE "${LD_CONF_FILE}" "${CMAKE_INSTALL_FULL_LIBDIR}\n")

if(UNIX AND NOT APPLE)
    install(FILES "${LD_CONF_FILE}"
            DESTINATION "/etc/ld.so.conf.d"
            COMPONENT Runtime)
    install(CODE "
        if(NOT \"\$ENV{DESTDIR}\")
            execute_process(COMMAND ldconfig)
        endif()
    " COMPONENT Runtime)
endif()
