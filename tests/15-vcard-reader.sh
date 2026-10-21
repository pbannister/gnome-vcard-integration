#!/bin/sh
#
# vCard reader test (prompts/features/03-vcard-reader.md).
#
# Tier: tool-gated (cmake and a C++ compiler).  Builds the reader test and runs
# it against the synthetic fixtures in dataflow.in/vcard/.
set -eu

DIRECTORY_SCRIPT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$DIRECTORY_SCRIPT/lib/test_helpers.sh"

NAME_TEST=$(basename "$0" .sh)
skip_unless_tool cmake
build_program

if [ ! -x "$DIRECTORY_BUILD/vcard-reader-test" ]; then
    fail_test "$NAME_TEST" 'the build produced no vcard-reader-test'
fi

"$DIRECTORY_BUILD/vcard-reader-test" "$DIRECTORY_FIXTURES"
pass_test "$NAME_TEST"
