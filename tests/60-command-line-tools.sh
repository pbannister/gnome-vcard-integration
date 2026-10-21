#!/bin/sh
#
# Command-line tools test (prompts/features/06-command-line-tools.md).
#
# Tier: tool-gated (cmake, a C++ compiler, python3).  Exercises the three public
# programs the way a person or a script uses them: the report, the JSON form,
# the property list, the renderer, and the exit codes, including an incomplete
# card whose warnings are a finding rather than a failure to read.
set -eu

DIRECTORY_SCRIPT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$DIRECTORY_SCRIPT/lib/test_helpers.sh"

NAME_TEST=$(basename "$0" .sh)
skip_unless_tool cmake
skip_unless_tool python3
build_program

DIRECTORY_TEMPORARY=$(mktemp -d)
trap 'rm -rf "$DIRECTORY_TEMPORARY"' EXIT HUP INT TERM

FILE_SIMPLE="$DIRECTORY_FIXTURES/simple.vcf"
FILE_MULTIPLE="$DIRECTORY_FIXTURES/multiple.vcf"
FILE_INCOMPLETE="$DIRECTORY_FIXTURES/incomplete.vcf"
FILE_INVALID="$DIRECTORY_FIXTURES/invalid.vcf"

PROGRAM_INSPECT="$DIRECTORY_BUILD/vcard-inspect"
PROGRAM_RENDER="$DIRECTORY_BUILD/vcard-render"
PROGRAM_INTEGRATE="$DIRECTORY_BUILD/vcard-integrate"

fail() {
    fail_test "$NAME_TEST" "$1"
}

# --- version and help --------------------------------------------------------
for program in "$PROGRAM_INSPECT" "$PROGRAM_RENDER" "$PROGRAM_INTEGRATE"; do
    if ! "$program" --version | grep -q '^vcard-'; then
        fail "$program --version did not name the program"
    fi
    "$program" --help >/dev/null || fail "$program --help failed"
done

# --- a whole card is reported ------------------------------------------------
FILE_REPORT="$DIRECTORY_TEMPORARY/report.txt"
"$PROGRAM_INSPECT" "$FILE_SIMPLE" > "$FILE_REPORT"

for expected in \
    'cards: 1' \
    'version: 4.0' \
    'formatted name: Ada Lovelace' \
    'organization: Analytical Engine Society' \
    'email: ada@example.invalid (WORK, preferred)' \
    'telephone: +44 20 0000 0000 (VOICE, WORK, preferred)' \
    'address: 12 Example Street, London, Greater London, SW1A 1AA, United Kingdom (WORK)' \
    'url: https://example.invalid/ada' \
    'photo: image/png (68 bytes, embedded)' \
    'properties: 17'; do
    grep -qF "$expected" "$FILE_REPORT" || fail "the report is missing: $expected"
done

# --- the property list names the encoded lines ------------------------------
FILE_PROPERTIES="$DIRECTORY_TEMPORARY/properties.txt"
"$PROGRAM_INSPECT" --properties "$FILE_SIMPLE" > "$FILE_PROPERTIES"
for expected in \
    'property TEL;TYPE=voice,work;PREF=1' \
    'property PHOTO;MEDIATYPE=image/png;ENCODING=b' \
    'value: <binary: 68 bytes, image/png>'; do
    grep -qF "$expected" "$FILE_PROPERTIES" || fail "the property list is missing: $expected"
done

# --- the JSON form agrees with the report ------------------------------------
FILE_JSON="$DIRECTORY_TEMPORARY/report.json"
"$PROGRAM_INSPECT" --json "$FILE_SIMPLE" > "$FILE_JSON"

python3 - "$FILE_JSON" <<'PYTHON_CHECK' || fail 'the JSON report is wrong'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as handle:
    report = json.load(handle)

assert report["sizeBytes"] > 0
assert report["warnings"] == []
assert len(report["cards"]) == 1
card = report["cards"][0]
assert card["version"] == "4.0"
assert card["formattedName"] == "Ada Lovelace"
assert card["organization"] == "Analytical Engine Society"
assert card["organizationUnit"] == "Mathematics"
assert card["nameComponents"][0] == "Lovelace"
assert card["telephones"][0]["value"] == "+44 20 0000 0000"
assert card["telephones"][0]["types"] == ["VOICE", "WORK"]
assert card["telephones"][0]["preferred"] is True
assert card["emails"][0]["value"] == "ada@example.invalid"
assert card["addresses"][0]["components"]["street"] == "12 Example Street"
assert card["addresses"][0]["components"]["country"] == "United Kingdom"
assert card["photo"]["embedded"] is True
assert card["photo"]["mediaType"] == "image/png"
assert card["photo"]["sizeBytes"] == 68
assert any(prop["name"] == "PHOTO" for prop in card["properties"])
assert card["warnings"] == []
PYTHON_CHECK

# --- an incomplete card is a finding, not a failure --------------------------
"$PROGRAM_INSPECT" "$FILE_INCOMPLETE" > "$DIRECTORY_TEMPORARY/incomplete.txt" \
    || fail 'vcard-inspect refused a readable but incomplete card'
grep -q 'warning: VERSION is missing' "$DIRECTORY_TEMPORARY/incomplete.txt" \
    || fail 'the incomplete card report does not name the missing VERSION'
grep -q 'warning: FN is missing' "$DIRECTORY_TEMPORARY/incomplete.txt" \
    || fail 'the incomplete card report does not name the missing FN'

"$PROGRAM_INSPECT" --json "$FILE_INCOMPLETE" > "$DIRECTORY_TEMPORARY/incomplete.json"
python3 - "$DIRECTORY_TEMPORARY/incomplete.json" <<'PYTHON_CHECK' || fail 'the incomplete card JSON is wrong'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as handle:
    report = json.load(handle)

card = report["cards"][0]
assert card["formattedName"] == "Person Example"
assert len(card["warnings"]) >= 2, card["warnings"]
PYTHON_CHECK

# --- a file that is not a vCard is refused ----------------------------------
if "$PROGRAM_INSPECT" "$FILE_INVALID" >/dev/null 2>&1; then
    fail 'vcard-inspect accepted a file that is not a vCard'
fi
if ! "$PROGRAM_INSPECT" "$FILE_INVALID" 2>&1 | grep -q '^vcard-inspect:'; then
    fail 'the refusal does not name the program'
fi

# --- a multi-card file is reported card by card -----------------------------
"$PROGRAM_INSPECT" "$FILE_MULTIPLE" > "$DIRECTORY_TEMPORARY/multiple.txt"
grep -q 'cards: 2' "$DIRECTORY_TEMPORARY/multiple.txt" \
    || fail 'the multi-card report does not count both cards'
grep -q 'formatted name: Grace Hopper' "$DIRECTORY_TEMPORARY/multiple.txt" \
    || fail 'the multi-card report is missing the first contact'
grep -q 'formatted name: Katherine Johnson' "$DIRECTORY_TEMPORARY/multiple.txt" \
    || fail 'the multi-card report is missing the second contact'

# --- the card is drawn ------------------------------------------------------
"$PROGRAM_RENDER" "$FILE_SIMPLE" > "$DIRECTORY_TEMPORARY/card.svg"
for expected in '<svg' 'Ada Lovelace' 'mailto:ada@example.invalid' \
    'tel:+44200000000' 'https://example.invalid/ada' 'data:image/png;base64,'; do
    grep -qF "$expected" "$DIRECTORY_TEMPORARY/card.svg" \
        || fail "the rendered card is missing: $expected"
done

"$PROGRAM_RENDER" --no-photo "$FILE_SIMPLE" > "$DIRECTORY_TEMPORARY/no-photo.svg"
if grep -qF 'data:image/png;base64,' "$DIRECTORY_TEMPORARY/no-photo.svg"; then
    fail '--no-photo still embedded the photograph'
fi

FILE_CARD="$DIRECTORY_TEMPORARY/out.svg"
"$PROGRAM_RENDER" --output "$FILE_CARD" "$FILE_SIMPLE"
[ -s "$FILE_CARD" ] || fail '--output wrote no file'

"$PROGRAM_RENDER" --index 2 "$FILE_MULTIPLE" > "$DIRECTORY_TEMPORARY/second.svg"
grep -qF 'Katherine Johnson' "$DIRECTORY_TEMPORARY/second.svg" \
    || fail '--index 2 did not draw the second contact'

if "$PROGRAM_RENDER" --index 3 "$FILE_MULTIPLE" >/dev/null 2>&1; then
    fail 'vcard-render accepted an index outside the file'
fi

# --- usage errors are exit code 2 -------------------------------------------
if "$PROGRAM_INSPECT" >/dev/null 2>&1; then
    fail 'vcard-inspect accepted no file argument'
fi
if "$PROGRAM_INSPECT" --nonsense "$FILE_SIMPLE" >/dev/null 2>&1; then
    fail 'vcard-inspect accepted an unknown option'
fi
if "$PROGRAM_INSPECT" "$FILE_SIMPLE" "$FILE_INCOMPLETE" >/dev/null 2>&1; then
    fail 'vcard-inspect accepted two files'
fi
if "$PROGRAM_RENDER" --index nope "$FILE_SIMPLE" >/dev/null 2>&1; then
    fail 'vcard-render accepted a non-numeric index'
fi
if "$PROGRAM_RENDER" --output >/dev/null 2>&1; then
    fail 'vcard-render accepted --output without a file'
fi
if "$PROGRAM_INTEGRATE" nonsense >/dev/null 2>&1; then
    fail 'vcard-integrate accepted an unknown command'
fi
if "$PROGRAM_INTEGRATE" handler nonsense >/dev/null 2>&1; then
    fail 'vcard-integrate accepted an unknown handler subcommand'
fi
if "$PROGRAM_INTEGRATE" handle >/dev/null 2>&1; then
    fail 'vcard-integrate handle accepted no file'
fi

# --- the handler entry point works without a display ------------------------
FILE_HANDLE="$DIRECTORY_TEMPORARY/handle.txt"
env -u DISPLAY -u WAYLAND_DISPLAY "$PROGRAM_INTEGRATE" handle "$FILE_SIMPLE" > "$FILE_HANDLE"
grep -q 'vcard-inspect' "$FILE_HANDLE" \
    || fail 'the handler entry point printed no equivalent command'

# --- a missing path is refused ----------------------------------------------
if "$PROGRAM_INSPECT" "$DIRECTORY_TEMPORARY/absent.vcf" >/dev/null 2>&1; then
    fail 'vcard-inspect accepted a path that does not exist'
fi
if "$PROGRAM_INTEGRATE" handle "$DIRECTORY_TEMPORARY/absent.vcf" >/dev/null 2>&1; then
    fail 'vcard-integrate handle accepted a path that does not exist'
fi

pass_test "$NAME_TEST"
