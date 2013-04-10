#!/bin/bash

set -e
set -u

declare -a flags=( ${@} )


declare -i flagscount=${#flags[*]}
declare -i i=0
declare regex='^[[:space:]]*-D(.+)[[:space:]]*$'
declare result=""

while [[ ${i} -lt ${flagscount} ]]; do
  if [[ -n "$(echo "${flags[i]}" | grep -E "${regex}")" ]]; then
    result="${result}$(echo "${flags[i]}" | sed -r "s/${regex}/ \1/g")"
  fi
  i+=1
done

echo "${result}"
