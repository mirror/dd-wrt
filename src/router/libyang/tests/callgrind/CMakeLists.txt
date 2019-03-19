cmake_minimum_required(VERSION 2.8.12)

# Callgrind tests
add_executable(validate validate.c)
target_link_libraries(validate yang)

add_executable(list_manipulation list_manipulation.c)
target_link_libraries(list_manipulation yang)

add_executable(create_data create_data.c)
target_link_libraries(create_data yang)

set(CALLGRIND_EXEC valgrind --tool=callgrind --instr-atstart=no)
add_custom_target(callgrind
    COMMAND ${CALLGRIND_EXEC} ./validate all-validation.yang all-validation.xml
    COMMAND ${CALLGRIND_EXEC} ./validate ietf-interfaces.yang iana-if-type.yang ietf-ip.yang ietf-interfaces.xml
    COMMAND ${CALLGRIND_EXEC} ./validate lists.yang lists.xml
    COMMAND ${CALLGRIND_EXEC} ./validate xpath.yang xpath.xml
    COMMAND ${CALLGRIND_EXEC} ./list_manipulation
    COMMAND ${CALLGRIND_EXEC} ./create_data
    DEPENDS validate list_manipulation create_data
    VERBATIM
)

add_custom_target(callgrind_clear
    COMMAND rm -f ./callgrind.out.*
)
