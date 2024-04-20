#! /bin/sh

LIST_BASE="List of the loaded models:
    i ietf-yang-metadata@2016-08-05
    I yang@2021-04-07
    i ietf-inet-types@2013-07-15
    i ietf-yang-types@2013-07-15"

testListEmptyContext() {
  output=`${YANGLINT} -l`
  assertEquals "Unexpected list of modules in empty context." "${LIST_BASE}" "${output}"
}

testListAllImplemented() {
  LIST_BASE_ALLIMPLEMENTED="List of the loaded models:
    I ietf-yang-metadata@2016-08-05
    I yang@2021-04-07
    I ietf-inet-types@2013-07-15
    I ietf-yang-types@2013-07-15"
  output=`${YANGLINT} -lii`
  assertEquals "Unexpected list of modules in empty context with -ii." "${LIST_BASE_ALLIMPLEMENTED}" "${output}"
}

. shunit2
