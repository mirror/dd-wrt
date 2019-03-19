set(JAVA_SWIG_BINDING libyangJava)
find_package(Java REQUIRED)
find_package(JNI REQUIRED)

include_directories(${JNI_INCLUDE_DIRS})
message(STATUS "JNI_INCLUDE_DIRS: ${JNI_INCLUDE_DIRS}")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_SWIG_FLAGS "-c++")
set(CMAKE_SWIG_FLAGS "-I${PROJECT_SOURCE_DIR}")
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/org/cesnet")

set_source_files_properties(${JAVA_SWIG_BINDING}.i PROPERTIES CPLUSPLUS ON PREFIX "" SWIG_FLAGS "-package;org.cesnet")

if(${CMAKE_VERSION} VERSION_LESS "3.8.0")
    swig_add_module(${JAVA_SWIG_BINDING} java ${JAVA_SWIG_BINDING}.i)
else()
    swig_add_library(${JAVA_SWIG_BINDING} LANGUAGE java SOURCES ${JAVA_SWIG_BINDING}.i)
endif()

set_target_properties(${SWIG_MODULE_${JAVA_SWIG_BINDING}_REAL_NAME} PROPERTIES PREFIX "")
swig_link_libraries(${JAVA_SWIG_BINDING} ${JNI_LIBRARIES} libyang-cpp)

add_custom_command(TARGET ${JAVA_SWIG_BINDING} POST_BUILD COMMAND "${Java_JAVAC_EXECUTABLE}" *.java -d . COMMAND "${Java_JAR_EXECUTABLE}" -cvf libyang.jar org)
set(NATIVE_JAR "${CMAKE_CURRENT_BINARY_DIR}/libyang.jar")

# install so file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${JAVA_SWIG_BINDING}.so" DESTINATION "${CMAKE_INSTALL_LIBDIR}")

file(COPY "examples" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
add_custom_command(TARGET ${JAVA_SWIG_BINDING} POST_BUILD
        COMMAND ${Java_JAVAC_EXECUTABLE} -cp .:${NATIVE_JAR} ${CMAKE_SWIG_OUTDIR}/examples/Context.java
        COMMAND ${Java_JAVAC_EXECUTABLE} -cp .:${NATIVE_JAR} ${CMAKE_SWIG_OUTDIR}/examples/ProcessTree.java
        COMMAND ${Java_JAVAC_EXECUTABLE} -cp .:${NATIVE_JAR} ${CMAKE_SWIG_OUTDIR}/examples/Xpath.java
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

if (ENABLE_BUILD_TESTS)

    file(COPY "tests" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    set(JAVA_TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/tests)
    set(JUNIT_JAR ${JAVA_TEST_DIR}/junit-4.12.jar:${JAVA_TEST_DIR}/hamcrest-core-1.3.jar)
    set(JAVA_TEST_SRC
            LibyangTest
            TreeDataTest
            TreeSchemaTest)
    configure_file("${JAVA_TEST_DIR}/Constants.java.in" "${JAVA_TEST_DIR}/Constants.java" ESCAPE_QUOTES @ONLY)

    foreach(JTEST ${JAVA_TEST_SRC})
        add_custom_command(TARGET ${JAVA_SWIG_BINDING} POST_BUILD
                COMMAND ${Java_JAVAC_EXECUTABLE} -cp ${NATIVE_JAR}:${JUNIT_JAR}:${JAVA_TEST_DIR} ${JAVA_TEST_DIR}/${JTEST}.java
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
     endforeach()

    add_subdirectory(tests)
endif()