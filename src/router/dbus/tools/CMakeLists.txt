add_definitions("-DDBUS_COMPILATION")

set(dbus_send_SOURCES
    dbus-print-message.c
    dbus-print-message.h
    dbus-send.c
    tool-common.c
    tool-common.h
)

set(dbus_monitor_SOURCES
    dbus-monitor.c
    dbus-print-message.c
    dbus-print-message.h
    tool-common.c
    tool-common.h
)

set(dbus_test_tool_SOURCES
    dbus-echo.c
    dbus-spam.c
    tool-common.c
    tool-common.h
    test-tool.c
    test-tool.h
)

set(dbus_update_activation_environment_SOURCES
    dbus-update-activation-environment.c
    tool-common.c
    tool-common.h
)

if(WIN32)
set(dbus_launch_SOURCES
    dbus-launch-win.c
)
else(WIN32)
set(dbus_launch_SOURCES
    dbus-launch.c
    tool-common.c
    tool-common.h
)
endif()

if(DBUS_BUILD_X11)
set(dbus_launch_SOURCES
     ${dbus_launch_SOURCES}
     dbus-launch-x11.c
)
endif()

set(dbus_cleanup_sockets_SOURCES
    dbus-cleanup-sockets.c
)

set(dbus_run_session_SOURCES
    dbus-run-session.c
)

if(WIN32)
    add_executable_version_info(dbus_send_SOURCES "dbus-send")
endif()

add_executable(dbus-send ${dbus_send_SOURCES})
target_link_libraries(dbus-send ${DBUS_LIBRARIES})
install(TARGETS dbus-send ${INSTALL_TARGETS_DEFAULT_ARGS})

if(WIN32)
    add_executable_version_info(dbus_test_tool_SOURCES "dbus-test-tool")
endif()

add_executable(dbus-test-tool ${dbus_test_tool_SOURCES})
target_link_libraries(dbus-test-tool ${DBUS_LIBRARIES})
install(TARGETS dbus-test-tool ${INSTALL_TARGETS_DEFAULT_ARGS})

if(WIN32)
    add_executable_version_info(dbus_update_activation_environment_SOURCES "dbus-update-activation-environment")
endif()
if(WIN32 AND NOT MSVC)
    add_uac_manifest(dbus_update_activation_environment_SOURCES)
endif()
add_executable(dbus-update-activation-environment ${dbus_update_activation_environment_SOURCES})
target_link_libraries(dbus-update-activation-environment ${DBUS_LIBRARIES})
install(TARGETS dbus-update-activation-environment ${INSTALL_TARGETS_DEFAULT_ARGS})

if(WIN32)
    add_executable_version_info(dbus_launch_SOURCES "dbus-launch")
endif()

add_executable(dbus-launch ${dbus_launch_SOURCES})
target_link_libraries(dbus-launch ${DBUS_LIBRARIES})
if(DBUS_BUILD_X11)
    target_link_libraries(dbus-launch ${X11_LIBRARIES} )
endif()
install(TARGETS dbus-launch ${INSTALL_TARGETS_DEFAULT_ARGS})

if(WIN32)
    add_executable_version_info(dbus_launch_SOURCES "dbus-monitor")
endif()

add_executable(dbus-monitor ${dbus_monitor_SOURCES})
target_link_libraries(dbus-monitor ${DBUS_LIBRARIES})
install(TARGETS dbus-monitor ${INSTALL_TARGETS_DEFAULT_ARGS})

add_executable(dbus-run-session ${dbus_run_session_SOURCES})
target_link_libraries(dbus-run-session ${DBUS_INTERNAL_LIBRARIES})
install(TARGETS dbus-run-session ${INSTALL_TARGETS_DEFAULT_ARGS})

# create the /var/lib/dbus directory for dbus-uuidgen
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/dbus)

set(EXAMPLES_SCRIPTS
    GetAllMatchRules.py
)

install(FILES ${EXAMPLES_SCRIPTS} DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/dbus/examples)
