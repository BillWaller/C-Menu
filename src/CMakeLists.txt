cmake_minimum_required(VERSION 4.1)

set(RSH_LD $ENV{RSH_LD})

project(CMenu
    VERSION 0.2.9
    LANGUAGES C
)
set(CMAKE_C_FLAGS "-O2 -I../include -Wall -Wextra")
set(CMAKE_INSTALL_REMOVE_ENVIRONMENT_RPATH TRUE)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type (e.g. ebug, 
    Release)" FORCE)
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/menuapp" CACHE PATH "Install path 
    prefix" FORCE)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

configure_file(
    "${PROJECT_SOURCE_DIR}/include/version.h.in"
    "${PROJECT_SOURCE_DIR}/include/version.h"
)

add_library(libcm SHARED dwin.c exec.c futil.c scriou.c sig.c)
set_target_properties(
    libcm PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

set(LIBS ncursesw tinfo m)

set(COMMON_SRCS
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
add_library(CMenu OBJECT ${COMMON_SRCS})

add_executable(menu menu.c menu_engine.c parse_menu_desc.c)
target_sources(menu PRIVATE $<TARGET_OBJECTS:CMenu>)
target_link_libraries(menu libcm ${LIBS})

add_executable(form form.c)
target_sources(form PRIVATE $<TARGET_OBJECTS:CMenu>)
target_link_libraries(form libcm ${LIBS})

add_executable(pick pick.c)
target_sources(pick PRIVATE $<TARGET_OBJECTS:CMenu>)
target_link_libraries(pick libcm ${LIBS})

add_executable(view view.c)
target_sources(view PRIVATE $<TARGET_OBJECTS:CMenu>)
target_link_libraries(view libcm ${LIBS})

add_executable(ckeys ckeys.c)
target_sources(ckeys PRIVATE $<TARGET_OBJECTS:CMenu>)
target_link_libraries(ckeys libcm ${LIBS})

add_executable(enterchr enterchr.c)
target_link_libraries(enterchr libcm ${LIBS})

add_executable(enterstr enterstr.c)
target_link_libraries(enterstr libcm ${LIBS})

add_executable(iloan iloan.c)
target_link_libraries(iloan libcm ${LIBS})

add_executable(lf lf.c)
target_link_libraries(lf libcm ${LIBS})

add_executable(optsp optsp.c)
target_sources(optsp PRIVATE $<TARGET_OBJECTS:CMenu>)
target_link_libraries(optsp libcm ${LIBS})

add_executable(rsh rsh.c)
#[[ Remove the Comment lines to enable static linking of rsh
set(RSH_LD -static)
]]
if (RSH_LD STREQUAL "-static")
    set_target_properties(rsh PROPERTIES LINK_FLAGS_RELEASE -s)
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
target_link_libraries(stripansi libcm ${LIBS})

add_executable(whence whence.c)
target_link_libraries(whence libcm ${LIBS})

install(
  TARGETS menu
          form
          pick
          view
          ckeys
          enterchr
          enterstr
          iloan
          lf
          optsp
          stripansi
          whence
  RUNTIME DESTINATION bin)

target_include_directories(libcm PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    $<INSTALL_INTERFACE:include>
)

install(TARGETS libcm
    DESTINATION lib64
    EXPORT libcm_export
)

install(EXPORT libcm_export
    DESTINATION lib64/cmake/libcm
    FILE libcmConfig.cmake
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
