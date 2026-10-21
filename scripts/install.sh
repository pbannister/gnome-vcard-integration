#!/bin/sh
#
# install.sh: install the products of gnome-vcard-integration from a GitHub release.
#
#   curl -fsSL https://raw.githubusercontent.com/pbannister/gnome-vcard-integration/master/scripts/install.sh | sh
#
# It fetches the release tarball for this machine, checks it against the release's
# SHA256SUMS, and installs into $PREFIX (default $HOME/.local).  Where no prebuilt
# build exists for this machine it builds from the source archive instead.  It then
# registers the vCard handler, so double-clicking a card opens the viewer and *.vcf
# files carry the project's icon; that step rewrites the user's MIME defaults and
# records what they were, so it can be declined with VCARD_INTEGRATION_NO_HANDLER=1
# and reversed with "vcard-integrate handler uninstall".
#
# Environment:
#   PREFIX                              where to install (default $HOME/.local)
#   VERSION                             a release tag, or "latest" (default)
#   REPOSITORY                          owner/repo to fetch from
#   VCARD_INTEGRATION_RELEASE_URL       base URL to fetch assets from, instead of GitHub
#   VCARD_INTEGRATION_TARBALL           install from this local tarball instead
#   VCARD_INTEGRATION_SKIP_VERIFY=1     install although SHA256SUMS is missing
#   VCARD_INTEGRATION_NO_BUILD=1        do not fall back to building from source
#   VCARD_INTEGRATION_NO_HANDLER=1      install the tools, register no handler
#   PROJECT_NAME                        asset-name piece (default gnome-vcard-integration)
#
# The TARBALL, RELEASE_URL, and SKIP_VERIFY names are accepted as aliases of the
# project-specific variables above, for the shared packaging test.
#
# This script is POSIX sh on purpose: it may be piped into any shell.
#
# See prompts/capabilities/04-release-and-publishing.md.
set -eu

REPOSITORY=${REPOSITORY:-pbannister/gnome-vcard-integration}
PREFIX=${PREFIX:-$HOME/.local}
VERSION=${VERSION:-latest}
PROGRAM_NAME=${PROJECT_NAME:-gnome-vcard-integration}
FILE_TARBALL_LOCAL=${VCARD_INTEGRATION_TARBALL:-${TARBALL:-}}
BASE_URL=${VCARD_INTEGRATION_RELEASE_URL:-${RELEASE_URL:-}}
SKIP_VERIFY=${VCARD_INTEGRATION_SKIP_VERIFY:-${SKIP_VERIFY:-0}}
NO_BUILD=${VCARD_INTEGRATION_NO_BUILD:-0}
NO_HANDLER=${VCARD_INTEGRATION_NO_HANDLER:-0}

say() {
    echo "install: $1"
}

fail() {
    echo "install: $1" >&2
    exit 1
}

have() {
    command -v "$1" > /dev/null 2>&1
}

# ---------------------------------------------------------------------------
# Platform

machine=$(uname -s -m 2>/dev/null || echo unknown)
case "$machine" in
    Linux\ x86_64 | Linux\ amd64)
        ARCHITECTURE=x86_64
        ;;
    Linux\ aarch64 | Linux\ arm64)
        ARCHITECTURE=aarch64
        ;;
    *)
        fail "no build is published for '$machine'; this project targets Linux on x86_64 and aarch64"
        ;;
esac
ASSET="$PROGRAM_NAME-linux-$ARCHITECTURE.tar.gz"

# ---------------------------------------------------------------------------
# Tools

if have curl; then
    fetch() { # <url> <destination>
        curl -fsSL --connect-timeout 20 --max-time 600 -o "$2" "$1"
    }
elif have wget; then
    fetch() { # <url> <destination>
        wget -q -T 20 -O "$2" "$1"
    }
else
    fail "curl or wget is needed to download anything"
fi

sha256_of() { # <file>
    if have sha256sum; then
        sha256sum "$1" | awk '{print $1}'
    elif have shasum; then
        shasum -a 256 "$1" | awk '{print $1}'
    else
        echo ""
    fi
}

if [ -z "$BASE_URL" ]; then
    if [ "latest" = "$VERSION" ]; then
        BASE_URL="https://github.com/$REPOSITORY/releases/latest/download"
    else
        BASE_URL="https://github.com/$REPOSITORY/releases/download/$VERSION"
    fi
fi

DIRECTORY_WORK=$(mktemp -d)
cleanup() {
    rm -rf "$DIRECTORY_WORK"
}
trap cleanup EXIT HUP INT TERM

# ---------------------------------------------------------------------------
# Fetch and verify

verify_tarball() { # <tarball> <sums-file>
    file_tarball=$1
    file_sums=$2
    expected=$(awk -v name="$(basename "$file_tarball")" '$2 == name { print $1 }' "$file_sums")
    if [ -z "$expected" ]; then
        # A sums file made with a bare name and one made with a path both work.
        expected=$(awk -v name="$(basename "$file_tarball")" 'index($2, name) > 0 { print $1 }' \
            "$file_sums" | head -1)
    fi
    if [ -z "$expected" ]; then
        fail "$(basename "$file_sums") has no digest for $(basename "$file_tarball")"
    fi
    actual=$(sha256_of "$file_tarball")
    if [ -z "$actual" ]; then
        say "no sha256sum tool is available, so the digest could not be checked"
        return 0
    fi
    if [ "$expected" != "$actual" ]; then
        fail "the download does not match SHA256SUMS: expected $expected but got $actual"
    fi
    say "the download matches its published digest"
}

install_tarball() { # <tarball>
    file_tarball=$1
    mkdir -p "$PREFIX"
    # The tarball holds bin/ and share/, which is the layout $PREFIX is meant to
    # have; extracting it there is the install.
    tar --extract --file "$file_tarball" --directory "$PREFIX" --no-same-owner
    chmod 755 "$PREFIX/bin"/* 2>/dev/null || true
    say "installed into $PREFIX"
}

# ---------------------------------------------------------------------------
# Build from source, when no prebuilt build matches this machine

build_from_source() {
    for tool_name in cmake c++ tar; do
        if ! have "$tool_name"; then
            fail "no prebuilt build for linux-$ARCHITECTURE, and building needs $tool_name; install it and try again, or set VCARD_INTEGRATION_RELEASE_URL to a mirror"
        fi
    done
    if [ "latest" = "$VERSION" ]; then
        source_url="https://github.com/$REPOSITORY/archive/refs/heads/master.tar.gz"
    else
        source_url="https://github.com/$REPOSITORY/archive/refs/tags/$VERSION.tar.gz"
    fi
    say "no prebuilt build for linux-$ARCHITECTURE: building from $source_url"
    fetch "$source_url" "$DIRECTORY_WORK/source.tar.gz" \
        || fail "could not download the source archive"
    tar --extract --file "$DIRECTORY_WORK/source.tar.gz" --directory "$DIRECTORY_WORK"
    DIRECTORY_SOURCE=$(find "$DIRECTORY_WORK" -maxdepth 1 -type d -name "$PROGRAM_NAME-*" | head -1)
    if [ -z "$DIRECTORY_SOURCE" ]; then
        fail "the source archive is not shaped as expected"
    fi
    say "building; this needs CMake and a C++ compiler, and GTK4 for the window"
    (
        cd "$DIRECTORY_SOURCE"
        sh scripts/program-build.sh
        PREFIX="$PREFIX" sh scripts/program-install.sh
    ) || fail "the build failed"
}

# ---------------------------------------------------------------------------
# Go

if [ -n "$FILE_TARBALL_LOCAL" ]; then
    if [ ! -f "$FILE_TARBALL_LOCAL" ]; then
        fail "$FILE_TARBALL_LOCAL is not there"
    fi
    say "installing from $FILE_TARBALL_LOCAL"
    FILE_SUMS_LOCAL="$(dirname "$FILE_TARBALL_LOCAL")/SHA256SUMS"
    if [ -f "$FILE_SUMS_LOCAL" ]; then
        verify_tarball "$FILE_TARBALL_LOCAL" "$FILE_SUMS_LOCAL"
    elif [ "1" = "$SKIP_VERIFY" ]; then
        say "SHA256SUMS is unavailable, and verification was skipped on request"
    else
        fail "SHA256SUMS is unavailable next to $FILE_TARBALL_LOCAL, so the tarball cannot be checked; set VCARD_INTEGRATION_SKIP_VERIFY=1 to install anyway"
    fi
    install_tarball "$FILE_TARBALL_LOCAL"
else
    FILE_TARBALL="$DIRECTORY_WORK/$ASSET"
    if fetch "$BASE_URL/$ASSET" "$FILE_TARBALL" 2>/dev/null; then
        if fetch "$BASE_URL/SHA256SUMS" "$DIRECTORY_WORK/SHA256SUMS" 2>/dev/null; then
            verify_tarball "$FILE_TARBALL" "$DIRECTORY_WORK/SHA256SUMS"
        elif [ "1" = "$SKIP_VERIFY" ]; then
            say "SHA256SUMS could not be fetched, and verification was skipped on request"
        else
            fail "SHA256SUMS could not be fetched, so the download cannot be checked; set VCARD_INTEGRATION_SKIP_VERIFY=1 to install anyway"
        fi
        install_tarball "$FILE_TARBALL"
    elif [ "1" = "$NO_BUILD" ]; then
        fail "there is no $ASSET in $VERSION; nothing was installed"
    else
        build_from_source
    fi
fi

# ---------------------------------------------------------------------------
# Register the handler
#
# The handler is what makes double-clicking a card open the viewer, and what
# gives vCard files their icon.  It rewrites the user's MIME defaults and records
# the previous ones, which is why it stays a step of its own -- but a user who has
# just installed these tools has asked for the tool, so it runs here, says what it
# did, and says how to undo it.  A failure is reported rather than fatal: the tools
# are installed either way, and the command to finish the job is printed.

if [ "1" = "$NO_HANDLER" ]; then
    say "the vCard handler was not registered, on request (VCARD_INTEGRATION_NO_HANDLER=1)"
elif [ -x "$PREFIX/bin/vcard-integrate" ]; then
    if "$PREFIX/bin/vcard-integrate" handler install > "$DIRECTORY_WORK/handler.txt" 2>&1; then
        say "the vCard handler is now installed for text/vcard and text/x-vcard"
    else
        say "the vCard handler could not be registered; the tool said:"
        cat "$DIRECTORY_WORK/handler.txt" 2>/dev/null || true
        say "run it yourself with: $PREFIX/bin/vcard-integrate handler install"
    fi
fi

# ---------------------------------------------------------------------------
# Report

VERSION_INSTALLED="unknown"
if [ -f "$PREFIX/VERSION" ]; then
    VERSION_INSTALLED=$(cat "$PREFIX/VERSION")
fi

echo
say "installed version $VERSION_INSTALLED into $PREFIX"
for program_name in vcard-inspect vcard-render vcard-integrate vcard-viewer; do
    if [ -x "$PREFIX/bin/$program_name" ]; then
        echo "  $PREFIX/bin/$program_name"
    fi
done
if [ -f "$PREFIX/share/icons/hicolor/scalable/apps/vcard-handler.svg" ]; then
    echo "  $PREFIX/share/icons/hicolor/scalable/apps/vcard-handler.svg"
fi
if [ -d "$PREFIX/bin" ]; then
    say "add $PREFIX/bin to PATH if it is not already there"
fi
exit 0
