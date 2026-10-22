#!/bin/sh
#
# project_helpers.sh: this project's additions to tests/lib/test_helpers.sh.
#
# tests/lib/test_helpers.sh is vendored and must not be edited; it sources this
# file when present. These additions were previously inside the vendored file,
# which is why they would have been lost at the next upgrade -- silently, after a
# vendor run that reported success.
set -eu

REPOSITORY_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
DIRECTORY_BUILD="$REPOSITORY_ROOT/dataflow.out/build"
DIRECTORY_FIXTURES="$REPOSITORY_ROOT/dataflow.in/vcard"

# build_program: configure and build the C++ reader, renderer, integration, and
# tools.  A test that needs the tools calls this rather than assuming an earlier
# test built them, because each test must also pass when it is run on its own.
build_program() {
    sh "$REPOSITORY_ROOT/scripts/program-build.sh" >/dev/null
}
