# Feature: Release Installer

## Purpose

A release must be installable with one command: a person should be able to
fetch the build for their machine, have it verified, install it, and open a card
without assembling the pieces by hand.

## Requirements

- `RELEASE-INSTALLER-R001` — `scripts/install.sh` must be POSIX `sh`, must not prompt, and must be safe to pipe from `curl`.
- `RELEASE-INSTALLER-R002` — The installer must fetch `<project>-linux-<arch>.tar.gz` for the machine from the release, check it against the release's `SHA256SUMS`, and refuse when verification is impossible.
- `RELEASE-INSTALLER-R003` — The installer must install into `$PREFIX` (default `$HOME/.local`); `VERSION` must select a release tag (default `latest`), `REPOSITORY` must override the owner/repo, and `VCARD_INTEGRATION_RELEASE_URL` must override the asset base URL.
- `RELEASE-INSTALLER-R004` — `VCARD_INTEGRATION_TARBALL` must install a local tarball after checking its sibling `SHA256SUMS`; `VCARD_INTEGRATION_SKIP_VERIFY=1` must permit an install when the checksum is unavailable.
- `RELEASE-INSTALLER-R005` — When no prebuilt asset exists for the machine, the installer must build from the source archive unless `VCARD_INTEGRATION_NO_BUILD=1`.
- `RELEASE-INSTALLER-R006` — After installing, the installer must run the installed `vcard-integrate handler install` unless `VCARD_INTEGRATION_NO_HANDLER=1`; a handler failure must be reported and must leave the tools installed.
- `RELEASE-INSTALLER-R007` — The installer must report the installed version, each installed program's path, and the installed icon's path.
- `RELEASE-INSTALLER-R008` — `README.md` must document the one-line install, `PREFIX`, `VERSION`, the handler step and its opt-out, and `make release-publish`.

## Behavior

- Running the one-line install on x86_64 Linux fetches the release tarball, verifies the digest, installs the programs, and registers the vCard handler.
- `PREFIX=/opt/vcard` installs there instead; `VERSION=v0.1.0` pins that release.
- `VCARD_INTEGRATION_NO_HANDLER=1` installs the programs and registers nothing.
- A local tarball with no sibling `SHA256SUMS` is refused unless `VCARD_INTEGRATION_SKIP_VERIFY=1` is set.
- On a machine with no matching prebuilt asset, the installer fetches the source archive and builds it.

## Dependencies

- `05-vcard-handler.md`
