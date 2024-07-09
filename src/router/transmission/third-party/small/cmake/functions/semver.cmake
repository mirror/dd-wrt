#
# Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#

# Parse a strict semver version string into its components
# This function expects a valid semver string as input and
# it will not make any expensive effort (use regex) to throw
# errors when the input is invalid.
function(semver_split version_string output_prefix)
    # Handle empty string
    if (NOT version_string)
        return()
    endif ()

    # Identify position of core version and tags
    string(FIND ${version_string} "-" prerelease_begin)
    string(FIND ${version_string} "+" metadata_begin)
    if (prerelease_begin EQUAL -1 AND metadata_begin EQUAL -1)
        set(${output_prefix}_CORE ${version_string})
        set(${output_prefix}_PRERELEASE "")
        set(${output_prefix}_METADATA "")
    elseif (prerelease_begin EQUAL -1)
        string(SUBSTRING ${version_string} 0 ${metadata_begin} "${output_prefix}_CORE")
        set(${output_prefix}_PRERELEASE "")
        math(EXPR metadata_id_begin "${metadata_begin} + 1")
        string(SUBSTRING ${version_string} ${metadata_id_begin} -1 ${output_prefix}_METADATA)
    elseif (metadata_begin EQUAL -1)
        string(SUBSTRING ${version_string} 0 ${prerelease_begin} ${output_prefix}_CORE)
        math(EXPR prerelease_id_begin "${prerelease_begin} + 1")
        string(SUBSTRING ${version_string} ${prerelease_id_begin} -1 ${output_prefix}_PRERELEASE)
        set(${output_prefix}_METADATA "")
    else ()
        string(SUBSTRING ${version_string} 0 ${prerelease_begin} ${output_prefix}_CORE)
        math(EXPR prerelease_id_begin "${prerelease_begin} + 1")
        math(EXPR prerelease_id_length "${metadata_begin} - ${prerelease_begin} - 1")
        string(SUBSTRING ${version_string} ${prerelease_id_begin} ${prerelease_id_length} ${output_prefix}_PRERELEASE)
        math(EXPR metadata_id_begin "${metadata_begin} + 1")
        string(SUBSTRING ${version_string} ${metadata_id_begin} -1 ${output_prefix}_METADATA)
    endif ()

    # Split the core version
    # Major
    string(FIND ${${output_prefix}_CORE} "." major_end)
    string(SUBSTRING ${${output_prefix}_CORE} 0 ${major_end} ${output_prefix}_MAJOR)
    # Minor
    math(EXPR minor_begin "${major_end} + 1")
    string(SUBSTRING ${${output_prefix}_CORE} ${minor_begin} -1 ${output_prefix}_MINOR_AND_PATCH)
    string(FIND ${${output_prefix}_MINOR_AND_PATCH} "." minor_end)
    string(SUBSTRING ${${output_prefix}_MINOR_AND_PATCH} 0 ${minor_end} ${output_prefix}_MINOR)
    # Patch
    math(EXPR patch_begin "${minor_end} + 1")
    string(SUBSTRING ${${output_prefix}_MINOR_AND_PATCH} ${patch_begin} -1 ${output_prefix}_PATCH)

    # Return
    set(${output_prefix}_CORE ${${output_prefix}_CORE} PARENT_SCOPE)
    set(${output_prefix}_MAJOR ${${output_prefix}_MAJOR} PARENT_SCOPE)
    set(${output_prefix}_MINOR ${${output_prefix}_MINOR} PARENT_SCOPE)
    set(${output_prefix}_PATCH ${${output_prefix}_PATCH} PARENT_SCOPE)
    set(${output_prefix}_PRERELEASE ${${output_prefix}_PRERELEASE} PARENT_SCOPE)
    set(${output_prefix}_METADATA ${${output_prefix}_METADATA} PARENT_SCOPE)
endfunction()

# Compare two semver versions >
function(semver_greater version_a version_b output)
    semver_split(${version_a} a)
    semver_split(${version_b} b)
    if (a_MAJOR GREATER b_MAJOR)
        set(${output} TRUE PARENT_SCOPE)
    elseif (b_MAJOR GREATER a_MAJOR)
        set(${output} FALSE PARENT_SCOPE)
    elseif (a_MINOR GREATER b_MINOR)
        set(${output} TRUE PARENT_SCOPE)
    elseif (b_MINOR GREATER a_MINOR)
        set(${output} FALSE PARENT_SCOPE)
    elseif (a_PATCH GREATER b_PATCH)
        set(${output} TRUE PARENT_SCOPE)
    elseif (b_PATCH GREATER a_PATCH)
        set(${output} FALSE PARENT_SCOPE)
    elseif (a_RELEASE GREATER b_RELEASE)
        set(${output} TRUE PARENT_SCOPE)
    elseif (b_RELEASE GREATER a_RELEASE)
        set(${output} FALSE PARENT_SCOPE)
    else ()
        set(${output} FALSE PARENT_SCOPE)
    endif ()
endfunction()

# Compare two semver versions <
function(semver_less version_a version_b output)
    semver_greater(${version_b} ${version_a} ${output})
    set(${output} ${${output}} PARENT_SCOPE)
endfunction()

# Compare two semver versions >=
function(semver_greater_equal version_a version_b output)
    semver_equal(${version_a} ${version_b} ${output})
    if (${output})
        set(${output} ${${output}} PARENT_SCOPE)
        return()
    endif ()
    semver_greater(${version_a} ${version_b} ${output})
    set(${output} ${${output}} PARENT_SCOPE)
endfunction()

# Compare two semver versions <=
function(semver_less_equal version_a version_b output)
    semver_equal(${version_a} ${version_b} ${output})
    if (${output})
        set(${output} ${${output}} PARENT_SCOPE)
        return()
    endif ()
    semver_less(${version_a} ${version_b} ${output})
    set(${output} ${${output}} PARENT_SCOPE)
endfunction()

# Compare two semver versions ==
function(semver_equal version_a version_b output)
    # Attempt to match without parsing
    if (version_a STREQUAL version_b)
        set(${output} TRUE PARENT_SCOPE)
        return()
    endif()
    # Most general case
    semver_split(${version_a} a)
    semver_split(${version_b} b)
    if (NOT a_MAJOR EQUAL b_MAJOR)
        set(${output} FALSE PARENT_SCOPE)
    elseif (NOT a_MINOR EQUAL b_MINOR)
        set(${output} FALSE PARENT_SCOPE)
    elseif (NOT a_PATCH EQUAL b_PATCH)
        set(${output} FALSE PARENT_SCOPE)
    elseif (NOT a_PRERELEASE EQUAL b_PRERELEASE)
        set(${output} FALSE PARENT_SCOPE)
    else ()
        # The metadata should be discard in semver string comparisons
        # so we end here
        set(${output} TRUE PARENT_SCOPE)
    endif ()
endfunction()

# Compare two semver versions and check if if they are semver compatible
function(semver_compatible version_a version_b output)
    semver_split(${version_a} a)
    semver_split(${version_b} b)
    if (NOT a_MAJOR EQUAL b_MAJOR)
        set(${output} FALSE PARENT_SCOPE)
    elseif (NOT a_MAJOR EQUAL 0)
        set(${output} TRUE PARENT_SCOPE)
    else ()
        if (NOT a_MINOR EQUAL b_MINOR)
            set(${output} FALSE PARENT_SCOPE)
        elseif (NOT a_MINOR EQUAL 0)
            set(${output} TRUE PARENT_SCOPE)
        else ()
            if (a_PATCH EQUAL b_PATCH)
                set(${output} TRUE PARENT_SCOPE)
            else ()
                set(${output} FALSE PARENT_SCOPE)
            endif ()
        endif ()
    endif ()
endfunction()

# Parse a strict *canonical* semver range string
# Only canonical operators and a few expansions are supported here.
# - No whitespaces allowed between operators in comparators.
# - Only strict semver versions are supported after comparators.
# This function splits but does not validate strings.
function(semver_range_split version_range_string output_prefix)
    if (NOT version_range_string)
        set(version_range_string "*")
    endif ()

    # Create list with all original comparators
    string(REPLACE " " ";" original_comparators "${version_range_string}")

    # Expand non-canonical operators
    foreach (original_comparator ${original_comparators})
        # Expand "*"
        string(FIND ${original_comparator} "*" star_begin)
        if (star_begin EQUAL 0)
            list(APPEND comparators ">=0.0.0-0")
            continue()
        endif ()

        # Expand "-"
        string(FIND ${original_comparator} " - " range_sep_begin)
        if (NOT range_sep_begin EQUAL -1)
            string(SUBSTRING ${original_comparator} 0 ${range_sep_begin} comparator_min_version)
            string(SUBSTRING ${original_comparator} ${range_sep_begin} -1 comparator_max_version)
            list(APPEND comparators ">=${comparator_min_version}")
            list(APPEND comparators "<=${comparator_max_version}")
            continue()
        endif ()

        # Expand "^" (any compatible version)
        string(FIND ${original_comparator} "^" caret_begin)
        if (caret_begin EQUAL 0)
            string(SUBSTRING ${original_comparator} 1 -1 comparator_min_version)
            semver_split(${comparator_min_version} comparator)
            if (NOT comparator_MAJOR EQUAL 0)
                math(EXPR comparator_next_major "${comparator_MAJOR} + 1")
                list(APPEND comparators ">=${comparator_min_version}")
                list(APPEND comparators "<${comparator_next_major}.0.0-0")
            elseif (NOT comparator_MINOR EQUAL 0)
                math(EXPR comparator_next_minor "${comparator_MINOR} + 1")
                list(APPEND comparators ">=${comparator_min_version}")
                list(APPEND comparators "<0.${comparator_next_minor}.0-0")
            else ()
                math(EXPR comparator_next_patch "${comparator_PATCH} + 1")
                list(APPEND comparators ">=${comparator_min_version}")
                list(APPEND comparators "<0.0.${comparator_next_patch}-0")
            endif()
            continue()
        endif ()

        # Expand "~" (accept patches)
        string(FIND ${original_comparator} "~" tilde_begin)
        if (tilde_begin EQUAL 0)
            string(SUBSTRING ${original_comparator} 1 -1 comparator_min_version)
            semver_split(${comparator_min_version} comparator)

            math(EXPR comparator_next_minor "${comparator_MINOR} + 1")
            list(APPEND comparators ">=${comparator_min_version}")
            list(APPEND comparators "<${comparator_MAJOR}.${comparator_next_minor}.0-0")
            continue()
        endif ()

        # Expand "=" (exactly one version)
        string(FIND ${original_comparator} "=" equal_begin)
        if (equal_begin EQUAL 0)
            string(SUBSTRING ${original_comparator} 1 -1 comparator_minmax_version)
            list(APPEND comparators ">=${comparator_minmax_version}")
            list(APPEND comparators "<=${comparator_minmax_version}")
            continue()
        endif ()

        # Append if not empty
        if (original_comparator)
            list(APPEND comparators ${original_comparator})
        endif ()
    endforeach ()

    # Iterate canonical comparators and update max and min requirements
    foreach (comparator ${comparators})
        # Check if it's increasing
        string(FIND ${comparator} ">" increase_begin)
        if (increase_begin EQUAL 0)
            # Check if it's inclusive
            string(FIND ${comparator} ">=" inclusive_increase_begin)
            # Get version substring
            if (inclusive_increase_begin EQUAL 0)
                string(SUBSTRING ${comparator} 2 -1 comparator_version)
                set(comparator_inclusive true)
            else ()
                string(SUBSTRING ${comparator} 1 -1 comparator_version)
                set(comparator_inclusive false)
            endif ()
            # Update min version
            if (NOT RANGE_MIN_VERSION)
                set(RANGE_MIN_VERSION ${comparator_version})
                set(RANGE_MIN_INCLUSIVE ${comparator_inclusive})
            else ()
                semver_greater(${comparator_version} ${RANGE_MIN_VERSION} gt)
                if (gt)
                    set(RANGE_MIN_VERSION ${comparator_version})
                    set(RANGE_MIN_INCLUSIVE ${comparator_inclusive})
                elseif (NOT ${comparator_inclusive})
                    semver_equal(${comparator_version} ${RANGE_MIN_VERSION} eq)
                    if (eq)
                        set(RANGE_MIN_INCLUSIVE FALSE)
                    endif ()
                endif ()
            endif ()
            continue()
        endif ()

        # Check if it's decreasing
        string(FIND ${comparator} "<" decrease_begin)
        if (decrease_begin EQUAL 0)
            # Check if it's inclusive
            string(FIND ${comparator} "<=" inclusive_decrease_begin)
            # Get version substring
            if (inclusive_decrease_begin EQUAL 0)
                string(SUBSTRING ${comparator} 2 -1 comparator_version)
                set(comparator_inclusive true)
            else ()
                string(SUBSTRING ${comparator} 1 -1 comparator_version)
                set(comparator_inclusive false)
            endif ()
            # Update max version
            if (NOT RANGE_MAX_VERSION)
                set(RANGE_MAX_VERSION ${comparator_version})
                set(RANGE_MAX_INCLUSIVE ${comparator_inclusive})
            else ()
                semver_less(${comparator_version} ${RANGE_MAX_VERSION} lt)
                if (lt)
                    set(RANGE_MAX_VERSION ${comparator_version})
                    set(RANGE_MAX_INCLUSIVE ${comparator_inclusive})
                elseif (NOT ${comparator_inclusive})
                    semver_equal(${comparator_version} ${RANGE_MAX_VERSION} eq)
                    if (eq)
                        set(RANGE_MAX_INCLUSIVE FALSE)
                    endif ()
                endif ()
            endif ()
            continue()
        endif ()

        # Handle the case with no canonical comparator ("=" should have been replaced at this point)
        # Update min version
        if (NOT RANGE_MIN_VERSION)
            set(RANGE_MIN_VERSION ${comparator})
            set(RANGE_MIN_INCLUSIVE TRUE)
        else ()
            semver_greater(${comparator} ${RANGE_MIN_VERSION} gt)
            if (gt)
                set(RANGE_MIN_VERSION ${comparator})
                set(RANGE_MIN_INCLUSIVE TRUE)
            endif ()
        endif ()
        # Update max version
        if (NOT RANGE_MAX_VERSION)
            set(RANGE_MAX_VERSION ${comparator})
            set(RANGE_MAX_INCLUSIVE TRUE)
        else ()
            semver_less(${comparator} ${RANGE_MAX_VERSION} lt)
            if (lt)
                set(RANGE_MAX_VERSION ${comparator})
                set(RANGE_MAX_INCLUSIVE TRUE)
            endif ()
        endif ()
    endforeach ()

    # Return values
    set(${output_prefix}_MIN_VERSION ${RANGE_MIN_VERSION} PARENT_SCOPE)
    set(${output_prefix}_MIN_INCLUSIVE ${RANGE_MIN_INCLUSIVE} PARENT_SCOPE)
    set(${output_prefix}_MAX_VERSION ${RANGE_MAX_VERSION} PARENT_SCOPE)
    set(${output_prefix}_MAX_INCLUSIVE ${RANGE_MAX_INCLUSIVE} PARENT_SCOPE)
endfunction()

# Check if semver version is compatible with a semver range
function(semver_range_compatible version range output)
    semver_range_split(${range} range)
    if (range_MIN_VERSION)
        if (range_MIN_INCLUSIVE)
            semver_greater_equal(${version} ${range_MIN_VERSION} ok)
        else ()
            semver_greater(${version} ${range_MIN_VERSION} ok)
        endif ()
        if (NOT ok)
            set(${output} FALSE PARENT_SCOPE)
            return()
        endif ()
    endif ()
    if (range_MAX_VERSION)
        if (range_MAX_INCLUSIVE)
            semver_less_equal(${version} ${range_MAX_VERSION} ok)
        else ()
            semver_less(${version} ${range_MAX_VERSION} ok)
        endif ()
        if (NOT ok)
            set(${output} FALSE PARENT_SCOPE)
            return()
        endif ()
    endif ()
    set(${output} TRUE PARENT_SCOPE)
endfunction()

# Split semver requirements into a list of semver ranges
function(semver_requirements_split version_requirements_string output)
    string(REPLACE "||" ";" list_result "${version_requirements_string}")
    set(${output} ${list_result} PARENT_SCOPE)
endfunction()

# Check if semver version is compatible with a semver range
function(semver_requirements_compatible version requirements output)
    semver_requirements_split(${requirements} ranges)
    foreach (range ${ranges})
        semver_range_compatible(${version} ${range} ok)
        if (NOT ok)
            set(${output} FALSE PARENT_SCOPE)
            return()
        endif ()
    endforeach ()
    set(${output} TRUE PARENT_SCOPE)
endfunction()

# Print a simple message or throw an error comparing the version found with the version requirements
# These messages are important because we cannot only rely on find_package to give us good messages
# regarding versions for most packages yet.
# - CMake has no support for semver versions
# - Many/most packages don't define FooVersion.cmake while many simply define it wrong (mismatching CMakeLists.txt)
# - Throwing an error here would just make most projects impossible so we just let the user deal with it
function(version_requirement_message PACKAGE)
    # Parse arguments
    set(options REQUIRED QUIET EXACT)
    set(oneValueArgs VERSION_FOUND VERSION_LOCK VERSION_REQUIREMENTS PREFIX_HINT)
    set(multiValueArgs)
    cmake_parse_arguments(VERSION_REQUIREMENTS_MESSAGE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # If package was found in a prefix hint
    if (EXISTS ${VERSION_REQUIREMENTS_MESSAGE_PREFIX_HINT})
        # Use the version checked from the manifest instead of the config/module script version
        get_filename_component(VERSION_FROM_PREFIX ${VERSION_REQUIREMENTS_MESSAGE_PREFIX_HINT} NAME)
        # Override version we consider to be found
        set(VERSION_REQUIREMENTS_MESSAGE_VERSION_FOUND ${VERSION_FROM_PREFIX})
    endif ()

    # Check if anything was found at all
    if (NOT VERSION_REQUIREMENTS_MESSAGE_VERSION_FOUND AND VERSION_REQUIREMENTS_MESSAGE_EXACT)
        if (VERSION_REQUIREMENTS_MESSAGE_REQUIRED)
            message(FATAL_ERROR "${PACKAGE} is required. Version requirements ${VERSION_REQUIREMENTS_MESSAGE_VERSION_REQUIREMENTS}.")
        elseif (NOT VERSION_REQUIREMENTS_MESSAGE_QUIET)
            message(WARNING "${PACKAGE} not found. Version requirements ${VERSION_REQUIREMENTS_MESSAGE_VERSION_REQUIREMENTS}.")
        endif()
    endif()

    # Compare the version we found with the lock version we give us a cheap message
    # semver_equal(${VERSION_REQUIREMENTS_MESSAGE_VERSION_FOUND} ${VERSION_REQUIREMENTS_MESSAGE_VERSION_LOCK} same_as_lock)
    set(same_as_lock false)
    if (same_as_lock)
        message(STATUS "${PACKAGE} (${VERSION_REQUIREMENTS_MESSAGE_VERSION_FOUND}) found - Lock version ${VERSION_REQUIREMENTS_MESSAGE_VERSION_LOCK} - Requirements ${VERSION_REQUIREMENTS_MESSAGE_VERSION_REQUIREMENTS}")
    else ()
        # If the version is not the same as the lock version
        # Check if it's compatible with the requirements
        semver_requirements_compatible(${VERSION_REQUIREMENTS_MESSAGE_VERSION_FOUND} ${VERSION_REQUIREMENTS_MESSAGE_VERSION_REQUIREMENTS} req_compatible)
        if (req_compatible)
            message(STATUS "${PACKAGE} (${VERSION_REQUIREMENTS_MESSAGE_VERSION_FOUND}) found - Compatible with requirements ${VERSION_REQUIREMENTS_MESSAGE_VERSION_REQUIREMENTS}")
        else ()
            # Else, check if it's at least compatible with the lock
            semver_compatible(${VERSION_REQUIREMENTS_MESSAGE_VERSION_FOUND} ${VERSION_REQUIREMENTS_MESSAGE_VERSION_LOCK} lock_compatible)
            if (lock_compatible)
                semver_greater_equal(${VERSION_REQUIREMENTS_MESSAGE_VERSION_FOUND} ${VERSION_REQUIREMENTS_MESSAGE_VERSION_LOCK} ge)
                if (ge)
                    # Greater and compatible
                    message(WARNING "${PACKAGE} (${VERSION_REQUIREMENTS_MESSAGE_VERSION_FOUND}) found - Compatible with lock version ${VERSION_REQUIREMENTS_MESSAGE_VERSION_LOCK} - Incompatible with requirements ${VERSION_REQUIREMENTS_MESSAGE_VERSION_REQUIREMENTS}")
                else()
                    # Compatible but not greater
                    message(WARNING "${PACKAGE} (${VERSION_REQUIREMENTS_MESSAGE_VERSION_FOUND}) found - Version is less than lock version ${VERSION_REQUIREMENTS_MESSAGE_VERSION_LOCK} but incompatible with requirements ${VERSION_REQUIREMENTS_MESSAGE_VERSION_REQUIREMENTS} - Required features might be missing - Also incompatible with requirements ${VERSION_REQUIREMENTS_MESSAGE_VERSION_REQUIREMENTS}")
                endif()
            else ()
                # Incompatible version found
                if (VERSION_REQUIREMENTS_MESSAGE_REQUIRED AND VERSION_REQUIREMENTS_MESSAGE_EXACT)
                    message(FATAL_ERROR "${PACKAGE} (${VERSION_REQUIREMENTS_MESSAGE_VERSION_FOUND}) found - Incompatible with requirements ${VERSION_REQUIREMENTS_MESSAGE_VERSION_REQUIREMENTS}")
                else()
                    message(WARNING "${PACKAGE} (${VERSION_REQUIREMENTS_MESSAGE_VERSION_FOUND}) found - Incompatible with requirements ${VERSION_REQUIREMENTS_MESSAGE_VERSION_REQUIREMENTS}")
                endif()
            endif()
        endif ()
    endif ()
endfunction()