#
# Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#


# ---- In-source guard ----

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
          FATAL_ERROR
          "In-source builds are not supported. "
          "Please read the BUILDING document before trying to build this project. "
          "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' first."
  )
endif()