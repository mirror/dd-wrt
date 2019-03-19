foreach(JTEST ${JAVA_TEST_SRC})

    add_test(NAME java_${JTEST}
            COMMAND ${Java_JAVA_EXECUTABLE}  -Djava.library.path=${CMAKE_SWIG_OUTDIR} -cp ${JAVA_TEST_DIR}:${NATIVE_JAR}:${JUNIT_JAR} ${JTEST}
            WORKING_DIRECTORY ${JAVA_TEST_DIR})

endforeach()