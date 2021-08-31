#!/bin/bash

# @file cstr.sh
# @author Adam Piecek <piecek@cesnet.cz>
# @brief Helper script for creating tests that converts yang and c strings.
#
# Copyright (c) 2020 CESNET, z.s.p.o.
#
# This source code is licensed under BSD 3-Clause License (the "License").
# You may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://opensource.org/licenses/BSD-3-Clause

#---------- Global variables ----------

script_name="$(basename $0)"

# -i <FILE>
target_file=""
target_text=""

# -l NUM
linenum_flag=""
linenum_default=1
linenum=$linenum_default

# -v
verbose=""

# -y <EXE>
yanglint_flag=""
yanglint_exe="yanglint -f yang"
yanglint_run=""

# <FILE>
input_text=""
input_file=""
input_ext=""

tmpfile=""

#---------- Basic functions ----------

function usage(){
echo "\
Usage:
$script_name [<INPUT>] [OPTION]... 

Examples:
From the \"test.c\" file, c-string is converted from line 20 to a yang schema.
$script_name test.c -l 20
The yang scheme is converted from the \"a.yang\" file to c-string and inserted into the \"test.c\" file on line 30.
$script_name a.yang -i test.c -l 30

Note:
If <INPUT> missing, read standard input. Press ctrl-d to end the entry.
The c-string is statement starting with character (\") and ending with string (\";).
This script creates a temporary file in the \"/tmp\" directory with the file name <yang_module_name>.yang,
which it eventually deletes when it finishes its run.
"
echo "\
OPTION:
-c, --combinations # Print a help table with all important parameter combinations and a usage comment.
#
-i, --insert=FILE # Insert c-string to the FILE.
                  # Option is valid only if <INPUT> is in .yang format and option \"-l\" is set.
                  # Warning: make sure you have a file backed up so you can undo the change.
                  # Don't forget to reload the file in your editor to see the changes.
#
-l, --line=NUM # If <INPUT> is in .c format: find c-string on the NUM line.
               # If <INPUT> is in .yang format: insert c-string on the NUM-th line.
               # If <INPUT> is from standard input, then the parameter is always $linenum_default
               # regardless of the value of the --line parameter.
#
-v, --verbose # Print debug messages.
#
-y, --yanglint=EXEC # Run yang schema formatting by \"EXEC\".
                    # Default value is \"./$yanglint_exe </tmp/TEMPORARY_FILE>\",
                    # but if the local directory does not contain yanglint,
                    # then yanglint is taken from PATH.
                    # Note that parameters must also be entered, eg \"pyang -f tree\".
" | column -t -s "#"
}

function combinations(){
echo "\
Abbreviations: c -> .c format file, y -> .yang format file
All important combinations of parameters are: ( -y, -v parameters are ommitted)
"
echo "\
<c> -i -l # Not allowed.
<c> -i # Not allowed.
<c> -l # Find c-string on line -l in file/stdin <c>, convert it to .yang and print the result.
<c> # Get c-string on line $linenum_default in file/stdin <c>, convert it to .yang and print the result.
<y> -i -l # Get yang schema from file/stdin <y>, convert it to .c format and insert result to the -i file on line -l.
<y> -i # Get yang schema from file/stdin <y>, convert it to .c format and insert result to the -i file on line $linenum_default.
<y> -l # Not allowed.
<y> # Get yang schema from file/stdin <y>, convert it to .c format and print the result.
" | column -t -s "#"
}

function print_verbose()
{
    if [ -n "$verbose" ] && [ -n "$1" ]; then echo "Verbose: $1" ; fi
}

function exit_if_error()
{
    if [ $? != 0 ]; then
        if [ -n "$1" ]; then
            echo "$1" >&2
        fi
        exit 1
    fi
}

function print_error_then_exit()
{
    echo "$1" >&2
    exit 1
}

function fill_input() {
    # check if argument is empty
    if  [ -z "$1" ]; then
        # read from stdin
        while IFS= read -r line; do
            if [ -z $input_text ] ; then
                input_text="$line"
            else
                input_text="$input_text\n$line"
            fi
        done
        # substitute string \n with newline
        input_text="$(echo -e "$input_text")"
        print_verbose "Text is loaded from stdin"
    else
        # check if file exists and is a regular file
        if [ -f "$1" ]; then 
            # <INPUT> is readable file
            input_file="$1"
            if [ -r $input_file ]; then
                # read from file is possible
                input_text=$(cat "$input_file")
                print_verbose "Text is loaded from \"$input_file\" file"
            else
                print_error_then_exit "Error: cannot read \"$input_file\""
            fi
        else
            print_error_then_exit "Error: file \"$1\" cannot open"
        fi
    fi
    # set input_text
    # set input_file if file name was entered
}


#---------- Getopt ----------

# options may be followed by one colon to indicate they have a required argument
options=$(getopt -o ci:hl:vy: -l combinations,insert,help,line:,verbose,yanglint: --name "$0" -- "$@")
exit_if_error "Failed to parse options...exiting."

eval set -- "$options"

# extract options and their arguments into variables.
while true ; do
    case "$1" in
    -c | --combinations )
        combinations 
        exit 0
        ;;
    -i | --insert )
        target_file="$2"
        shift 2
        ;;
    -h | --help )
        usage
        exit 0
        ;;
    -l | --line )
        linenum_flag="true"
        linenum="$2"
        shift 2
        ;;
    -v | --verbose )
        verbose="true"
        shift
        ;;
    -y | --yanglint)
        yanglint_flag="true"
        yanglint_exe="$2"
        shift 2
        ;;
    -- )
        # set input_text
        # set input_file if file name was entered
        fill_input $2
        break
        ;;
    -* )
        usage
        print_error_then_exit "$0: error - unrecognized option $1"
        ;;
    *)
        print_error_then_exit "Internal error!"
        ;;
  esac
done

#---------- Functions for checking parameters ----------

function get_one_line()
{
    local text_with_more_lines="$1"
    local linenum="$2"
    echo "$(echo "$text_with_more_lines" | sed "${linenum}q;d")"
}

function recognize_format()
{
    local text="$1"
    local linenum="$2"
    local line=$(get_one_line "$text" "$linenum")
    local matched_chars=$(expr match "$line" "^\s*\"")
    if [ "$matched_chars" == "0" ]; then
        echo "yang"
    else
        echo "c"
    fi
}

#---------- Check parameters ----------

# check -y
exe_name=$(echo "$yanglint_exe" | awk '{print $1;}')
if [ -n "$yanglint_flag" ]; then
    # try user's exe
    command -v "$exe_name" > /dev/null 2>&1
    if [ $? != 0 ] ; then
        command -v "./$exe_name" > /dev/null 2>&1
        exit_if_error "Error: cannot find exe \"$exe_name\""
        yanglint_run="./$yanglint_exe"
    else
        yanglint_run="$yanglint_exe"
    fi
    print_verbose "Using user's EXE \"$exe_name\""
else
    # try in exe current directory
    command -v "./$exe_name" > /dev/null 2>&1
    if [ $? == 0 ] ; then
        print_verbose "Using default \"$exe_name\" in current directory"
        yanglint_run="./$yanglint_exe"
    else 
        # try PATH's exe
        command -v "$exe_name" > /dev/null 2>&1
        exit_if_error "Error: \"$exe_name\" wasn't found in the current directory nor is installed"
        print_verbose "Using default \"$exe_name\" from PATH"
        yanglint_run="$yanglint_exe"
    fi
fi
# yanglint_run must be set
yanglint_run="$yanglint_run"" " # add space due to input filename

# check <INPUT>
# expected that input_text has string
if [ -n "$input_file" ]; then
    # get suffix of the <INPUT> file
    input_ext=${input_file##*.}
else 
    # <INPUT> is from stdin
    input_ext=$(recognize_format "$input_text" "1")
fi
print_verbose "<INPUT> is in \"$input_ext\" format"
# input_ext must be set

# check -i
if [ -n "$target_file" ]; then 
    # if target_file is writeable
    if [ -w $target_file ]; then
        print_verbose "target_file $target_file is writeable"
    else 
        print_error_then_exit "Error: cannot insert text to file \"$target_file\""
    fi
    # if <INPUT> is yang then -l must be set
    if [ "$input_ext" == "yang" ] && [ -n "$linenum_flag" ]; then
        print_verbose "-i option is valid"
    else
        print_error_then_exit "Error: Option -i is valid only if <INPUT> is in .yang format and option \"-l\" is set."
    fi
    target_text=$(cat "$target_file")
fi
# target_text must be set

# check -l
if [ -n "$linenum_flag" ]; then

    if [ -z "$input_file" ]; then
        # reading <INPUT> from stdin
        print_verbose "-l option is ignored because <INPUT> is from stdin."
        linenum=$linenum_default
    else
        if [ "$linenum" -lt "0" ]; then
            print_error_then_exit "Error: only positive numbers in --line option are valid"
        fi
        if [ "$input_ext" == "yang" ]; then
            if [ -z "$target_file" ]; then
                print_error_then_exit "Error: Option -l with <INPUT> format yang is valid only if option -i is set too."
            fi
            text4linenum="$target_text"
        else
            text4linenum="$input_text"
        fi
        # check if linenum is not too big
        lines_count=$(echo "$text4linenum" | wc -l)
        if [ "$linenum" -gt "$lines_count" ]; then
            print_error_then_exit "Error: number in --line option is too big"
        fi
        print_verbose "-l option is valid"
    fi
else 
    print_verbose "-l option is not set"
    # rest of restrictions must be checked in option -i
fi

#---------- Formatting text ----------

# warning: do not call this function in subshell $(formatting_yang_text)
function formatting_yang_text()
{
    # parameters: modify global variable input_text, read yanglint_run, read tmpfile
    echo "$input_text" > "$tmpfile" 
    # check if <INPUT> is valid yang file, store only stderr to variable
    yanglint_output=$(eval ""$yanglint_run" "$tmpfile"" 2>&1) # do not add local
    local yanglint_retval="$?"
    if [ "$yanglint_retval" != "0" ]; then
        print_verbose "$yanglint_output"
    fi
    $(exit $yanglint_retval)
    exit_if_error "Error: yang-schema in is not valid."
    input_text="$yanglint_output"
}

#---------- Main functions ----------

# called from main run
function cstring2yang()
{
    local ret
    local input_text="$1"
    local linenum="$2"
    # extract statement from c language file from specific line
    ret=$(echo "$input_text" |
        awk -v linenum="$linenum" '
            NR >= linenum   {lineflag=1; print}
            /;\s*$/         {if(lineflag == 1) exit;}
        ')
    # substitute special characters - for example \"
    ret=$(printf "$ret")
    # remove everything before first " and remove last "
    ret=$(echo "$ret" | grep -oP '"\K.*' | sed 's/"\s*$//')
    # but last line is not right due to "; at the end of line
    # so get last line and remove ";
    lastline=$(echo "$ret" | tail -n1 | sed -n 's/";\s*$//p')
    # get everything before last line
    ret=$(echo "$ret" | head -n -1)
    # concatenate result
    ret=$ret$lastline
    echo "$ret"
}

# called from main run
function yang2cstring()
{
    local ret
    local input_text="$1"
    # backslashing character "
    ret=${input_text//\"/\\\"}
    ret=$(echo "$ret" | awk -v s="\"" -v e="\\\n\"" '{print s$0e}')
    ret="$ret"";"
    echo "$ret"
}

# called from --Create temporary file--
function get_yang_module_name()
{
    local text="$1"
    local linenum="$2"
    local input_ext="$3"
    if [ "$input_ext" == "yang" ]; then
        # module name search on line 1 in .yang file
        linenum=1
    fi
    # else get module name on line $linenum in .c file
    echo "$(echo "$text" | awk -v linenum="$linenum" 'NR >= linenum' | grep -oP -m 1 "\s*module\s+\K\S+")"
}

# called from tabs2spaces_in_line and insert_indentation
function number2spaces()
{
    local number="$1"
    # return string containing <number> spaces
    echo "$(printf ' %.0s' $(seq 1 $number))"
}

# called from count_indentation_in_line function
function tabs2spaces_in_line()
{
    local text="$1"
    local linenum="$2"
    local tabinspaces="$(number2spaces 4)" # 1 tab = 4 spaces
    local line="$(get_one_line "$text" "$linenum")"
    echo "$(echo "$line" | sed "s/\t/$tabinspaces/")"
}

# called from main run
function count_indentation_in_line()
{
    local text="$1"
    local linenum="$2"
    local line="$(tabs2spaces_in_line "$1" "$2")"
    echo "$(expr match "$line" "^ *")"
}

# called from main run
function insert_indentation()
{
    local text="$1"
    local number_of_spaces="$2" # from count_indentation_in_line
    local spaces="$(number2spaces "$number_of_spaces")"
    echo "$(echo "$text" | sed -e "s/^/$spaces/")"
}

# called from main run
function insert_text2file()
{
    local text="$1"
    local linenum="$2"
    local filename="$3"

    linenum=$((linenum + 1))
    awk -i inplace -v text="$text" -v linenum="$linenum" 'NR == linenum {print text} 1' $filename
}

#---------- Create temporary file ----------

module_name=$(get_yang_module_name "$input_text" "$linenum" "$input_ext")
if [ -z "$module_name" ]; then
    print_error_then_exit "Error: module name not found"
fi
tmpfile="/tmp/""$module_name"".yang"
touch "$tmpfile"
exit_if_error "Error: error while creating temporary file"
# delete temporary file after script end
trap 'rm -f -- "$tmpfile"' INT TERM HUP EXIT
exit_if_error "Error: trap return error"

#---------- Main run ----------

# print new line for clarity
if [ -z "$input_file" ] && [ -z "$target_file" ]; then
    echo ""
fi

if [ "$input_ext" == "yang" ]; then
    if [ -z "$target_file" ]; then
        # Options: (<y> -l, <y>, <y-stdin> -l, <y-stdin>)
        print_verbose "Print c-string to output"
        formatting_yang_text
        echo "$(yang2cstring "$input_text")"
    else
        # Options: (<y-stdin> -i -l, <y> -i -l, <y> -i)
        print_verbose "Insert c-string to target_file"

        # formatting and converting
        formatting_yang_text
        inserted_text="$(yang2cstring "$input_text")"
        # add extra backslash
        inserted_text=${inserted_text//\\/\\\\}

        # indentation
        indentation="$(count_indentation_in_line "$target_text" "$linenum")"
        print_verbose "indentation is: $indentation"
        inserted_text="$(insert_indentation "$inserted_text" "$indentation")"

        # inserting to file
        insert_text2file "$inserted_text" "$linenum" "$target_file"
        echo "Done"
    fi
elif [ "$input_ext" == "c" ] || [ "$input_ext" == "h" ]; then
    # Options: (<c-stdin> -l, <c-stdin>, <c> -l, <c>)
    print_verbose "Print yang to output or from file <c> print yang to output"
    output="$(cstring2yang "$input_text" "$linenum")"
    # "input_text" is input and output parameter for formatting_yang_text
    input_text="$output"
    formatting_yang_text
    echo "$input_text"
else
    print_error_then_exit "Error: format \"$input_ext\" is not supported"
fi

exit 0
