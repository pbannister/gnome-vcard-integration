#!/bin/sh
#
# Fixture privacy guard.
#
# The project's privacy boundary says that a real vCard must never be committed:
# a real card belongs to a person.  The synthetic fixtures under
# dataflow.in/vcard/ are the only cards that may be tracked, and each must be
# reviewed when it is added or changed.
#
# This test enforces the allowlist by name.  It does not classify content: a
# real card that overwrites an allowlisted filename still passes, so the
# reviewer must read a changed fixture.  The allowlist is the gate against a
# new, unreviewed file being added to the tree.
set -eu

REPOSITORY_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
DIRECTORY_FIXTURES="$REPOSITORY_ROOT/dataflow.in/vcard"

ALLOWED_FIXTURES='incomplete.vcf invalid.vcf legacy.vcf multiple.vcf simple.vcf soft-break.vcf'

fail() {
    echo "04-fixture-privacy: $1" >&2
    exit 1
}

for file_fixture in "$DIRECTORY_FIXTURES"/*.vcf; do
    [ -f "$file_fixture" ] || continue
    name_fixture=$(basename "$file_fixture")
    case " $ALLOWED_FIXTURES " in
        *" $name_fixture "*)
            ;;
        *)
            fail "unreviewed vCard fixture: $name_fixture; use synthetic data and add its name to ALLOWED_FIXTURES"
            ;;
    esac
done

for name_fixture in $ALLOWED_FIXTURES; do
    [ -f "$DIRECTORY_FIXTURES/$name_fixture" ] \
        || fail "missing reviewed fixture: $name_fixture"
done

echo '04-fixture-privacy: ok'
exit 0
