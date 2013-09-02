#!/bin/sh
#
# find test/*.cl | xargs leak-check.sh bin/webcl-validator

# Location of validator binary.
VALIDATOR="$1"
shift 1
# Location of test files.
TEST_FILES="$@"

for i in $TEST_FILES ; do
    valgrind --leak-check=full --track-origins=yes --log-file=$i.log $VALIDATOR $i
done
