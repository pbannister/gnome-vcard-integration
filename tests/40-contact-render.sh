#!/bin/sh
#
# Contact renderer test (prompts/features/04-contact-renderer.md).
#
# Tier: tool-gated (cmake and a C++ compiler).  Builds the renderer test and
# checks the SVG it draws, including the escaping and the link policy.
set -eu

DIRECTORY_SCRIPT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$DIRECTORY_SCRIPT/lib/test_helpers.sh"

NAME_TEST=$(basename "$0" .sh)
skip_unless_tool cmake
build_program

if [ ! -x "$DIRECTORY_BUILD/contact-render-test" ]; then
    fail_test "$NAME_TEST" 'the build produced no contact-render-test'
fi

"$DIRECTORY_BUILD/contact-render-test" "$DIRECTORY_FIXTURES"
pass_test "$NAME_TEST"
