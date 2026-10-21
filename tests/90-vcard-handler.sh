#!/bin/sh
#
# vCard handler MIME-definition test (prompts/features/05-vcard-handler.md).
#
# Tier: tool-gated (cmake, a C++ compiler, xdg-mime).  Runs a real install and
# uninstall inside a throwaway HOME, so the assertions are about the files the
# desktop reads and not about this machine:
#
#   * the MIME definition names the card type, its alias, its globs, and this icon;
#   * the handler entry, the icons, and the record land at their fixed paths;
#   * both types now open with this handler;
#   * a definition the project did not write is backed up and restored;
#   * one the project created is removed again;
#   * the record is what makes an uninstall possible, so its absence is refused.
set -eu

DIRECTORY_SCRIPT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$DIRECTORY_SCRIPT/lib/test_helpers.sh"

NAME_TEST=$(basename "$0" .sh)
skip_unless_tool cmake
skip_unless_tool xdg-mime
build_program

PROGRAM_INTEGRATE="$DIRECTORY_BUILD/vcard-integrate"

DIRECTORY_TEST=$(mktemp -d)
trap 'rm -rf "$DIRECTORY_TEST"' EXIT HUP INT TERM

# run_sandbox: run a command against a throwaway desktop, with no display so the
# handler entry point never tries to open a window.
run_sandbox() {
    directory_home=$1
    shift
    env -u DISPLAY -u WAYLAND_DISPLAY \
        HOME="$directory_home" \
        XDG_DATA_HOME="$directory_home/.local/share" \
        XDG_CONFIG_HOME="$directory_home/.config" \
        XDG_DATA_DIRS="$DIRECTORY_TEST/share" \
        "$@"
}

TYPE_CARD='text/vcard'
TYPE_LEGACY='text/x-vcard'

fail() {
    fail_test "$NAME_TEST" "$1"
}

# --- a fresh install ---------------------------------------------------------
DIRECTORY_HOME="$DIRECTORY_TEST/home-fresh"
mkdir -p "$DIRECTORY_HOME/.local/share" "$DIRECTORY_HOME/.config" "$DIRECTORY_TEST/share"

FILE_DEFINITION="$DIRECTORY_HOME/.local/share/mime/packages/vcard.xml"
FILE_ENTRY="$DIRECTORY_HOME/.local/share/applications/vcard-handler.desktop"
FILE_ICON="$DIRECTORY_HOME/.local/share/icons/hicolor/scalable/apps/vcard-handler.svg"
FILE_TYPE_ICON="$DIRECTORY_HOME/.local/share/icons/hicolor/scalable/mimetypes/text-vcard.svg"
FILE_TYPE_ICON_LEGACY="$DIRECTORY_HOME/.local/share/icons/hicolor/scalable/mimetypes/text-x-vcard.svg"
FILE_MANIFEST="$DIRECTORY_HOME/.local/share/gnome-vcard-integration/vcard-handler.manifest"

run_sandbox "$DIRECTORY_HOME" "$PROGRAM_INTEGRATE" handler install >/dev/null \
    || fail 'handler install failed'

for file_expected in "$FILE_DEFINITION" "$FILE_ENTRY" "$FILE_ICON" "$FILE_TYPE_ICON" \
                    "$FILE_TYPE_ICON_LEGACY" "$FILE_MANIFEST"; do
    [ -f "$file_expected" ] || fail "handler install did not write $file_expected"
done

grep -q 'type="text/vcard"' "$FILE_DEFINITION" \
    || fail 'the MIME definition does not name the card type'
grep -q 'alias type="text/x-vcard"' "$FILE_DEFINITION" \
    || fail 'the MIME definition does not alias the legacy type'
grep -q 'glob pattern="\*.vcf"' "$FILE_DEFINITION" \
    || fail 'the MIME definition does not glob *.vcf'
grep -q 'glob pattern="\*.vcard"' "$FILE_DEFINITION" \
    || fail 'the MIME definition does not glob *.vcard'
grep -q 'value="BEGIN:VCARD"' "$FILE_DEFINITION" \
    || fail 'the MIME definition has no magic test'
grep -q 'generic-icon name="vcard-handler"' "$FILE_DEFINITION" \
    || fail 'the MIME definition does not name this icon'

grep -q 'Exec=.* handle %f' "$FILE_ENTRY" || fail 'the entry has no handle Exec line'
grep -q 'MimeType=text/vcard;text/x-vcard;' "$FILE_ENTRY" \
    || fail 'the entry does not name both types'
grep -q 'StartupWMClass=vcard-viewer' "$FILE_ENTRY" || fail 'the entry has no WM class'
grep -q 'X-Integrated-By=gnome-vcard-integration' "$FILE_ENTRY" \
    || fail 'the entry does not say who wrote it'

grep -q '^mime_created=1$' "$FILE_MANIFEST" || fail 'the record does not claim the definition'
grep -q '^mime_package=' "$FILE_MANIFEST" || fail 'the record does not name the definition'
grep -q "^previous_default=${TYPE_CARD}	" "$FILE_MANIFEST" \
    || fail 'the record does not remember the previous card handler'
grep -q "^previous_default=${TYPE_LEGACY}	" "$FILE_MANIFEST" \
    || fail 'the record does not remember the previous legacy handler'

# Both types now resolve to this handler.
for type_name in "$TYPE_CARD" "$TYPE_LEGACY"; do
    default_now=$(run_sandbox "$DIRECTORY_HOME" xdg-mime query default "$type_name")
    [ "$default_now" = 'vcard-handler.desktop' ] \
        || fail "the default for $type_name is $default_now"
done

# Status names the same facts.
FILE_STATUS="$DIRECTORY_TEST/status.txt"
run_sandbox "$DIRECTORY_HOME" "$PROGRAM_INTEGRATE" handler status > "$FILE_STATUS"
grep -q "^${TYPE_CARD}: vcard-handler.desktop$" "$FILE_STATUS" \
    || fail 'handler status did not report the card default'
grep -q "^${TYPE_LEGACY}: vcard-handler.desktop$" "$FILE_STATUS" \
    || fail 'handler status did not report the legacy default'
grep -q 'mime definition: present (installed by this project)' "$FILE_STATUS" \
    || fail 'handler status did not report the definition as ours'
grep -q 'type icons: text/vcard present, text/x-vcard present' "$FILE_STATUS" \
    || fail 'handler status did not report both type icons'

# Installing again is harmless and keeps ownership of the definition.
run_sandbox "$DIRECTORY_HOME" "$PROGRAM_INTEGRATE" handler install >/dev/null \
    || fail 'a second handler install failed'
grep -q '^mime_created=1$' "$FILE_MANIFEST" \
    || fail 'a second install lost ownership of the definition'

# --- a definition the project did not write is restored ----------------------
DIRECTORY_OTHER="$DIRECTORY_TEST/home-existing"
mkdir -p "$DIRECTORY_OTHER/.local/share/mime/packages" "$DIRECTORY_OTHER/.config"
FILE_OTHER_DEFINITION="$DIRECTORY_OTHER/.local/share/mime/packages/vcard.xml"
cat > "$FILE_OTHER_DEFINITION" <<'OTHER_DEFINITION'
<?xml version="1.0" encoding="UTF-8"?>
<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
  <mime-type type="text/vcard">
    <comment>Somebody else's definition</comment>
    <glob pattern="*.vcf"/>
  </mime-type>
</mime-info>
OTHER_DEFINITION
cp "$FILE_OTHER_DEFINITION" "$DIRECTORY_TEST/other-original.xml"

run_sandbox "$DIRECTORY_OTHER" "$PROGRAM_INTEGRATE" handler install >/dev/null \
    || fail 'handler install over an existing definition failed'

FILE_OTHER_MANIFEST="$DIRECTORY_OTHER/.local/share/gnome-vcard-integration/vcard-handler.manifest"
[ -f "$DIRECTORY_OTHER/.local/share/gnome-vcard-integration/backup/vcard.xml" ] \
    || fail 'the existing definition was not backed up'
if grep -q '^mime_created=1$' "$FILE_OTHER_MANIFEST"; then
    fail 'a displaced definition was recorded as created by this project'
fi

run_sandbox "$DIRECTORY_OTHER" "$PROGRAM_INTEGRATE" handler uninstall >/dev/null \
    || fail 'handler uninstall over a displaced definition failed'
cmp -s "$DIRECTORY_TEST/other-original.xml" "$FILE_OTHER_DEFINITION" \
    || fail 'the displaced definition was not restored byte for byte'

# --- a fresh install uninstalls cleanly --------------------------------------
run_sandbox "$DIRECTORY_HOME" "$PROGRAM_INTEGRATE" handler uninstall >/dev/null \
    || fail 'handler uninstall failed'

for file_gone in "$FILE_ENTRY" "$FILE_MANIFEST" "$FILE_ICON" "$FILE_TYPE_ICON" \
                 "$FILE_TYPE_ICON_LEGACY" "$FILE_DEFINITION"; do
    if [ -e "$file_gone" ]; then
        fail "handler uninstall left $file_gone"
    fi
done

# --- uninstall without a record is refused -----------------------------------
DIRECTORY_NONE="$DIRECTORY_TEST/home-none"
mkdir -p "$DIRECTORY_NONE/.local/share" "$DIRECTORY_NONE/.config"
if run_sandbox "$DIRECTORY_NONE" "$PROGRAM_INTEGRATE" handler uninstall >/dev/null 2>&1; then
    fail 'handler uninstall succeeded with no record'
fi

pass_test "$NAME_TEST"
