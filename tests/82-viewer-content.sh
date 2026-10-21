#!/bin/sh
#
# Viewer content test (prompts/features/06-command-line-tools.md).
#
# Tier: tool-gated (cmake and a C++ compiler).  Builds the viewer content test
# and checks the Discovered command/output capture and the About identity
# without needing a display.
set -eu

DIRECTORY_SCRIPT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$DIRECTORY_SCRIPT/lib/test_helpers.sh"

NAME_TEST=$(basename "$0" .sh)
skip_unless_tool cmake
build_program

if [ ! -x "$DIRECTORY_BUILD/viewer-content-test" ]; then
    fail_test "$NAME_TEST" 'the build produced no viewer-content-test'
fi

"$DIRECTORY_BUILD/viewer-content-test" "$DIRECTORY_FIXTURES"
pass_test "$NAME_TEST"
