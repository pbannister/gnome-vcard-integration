# Task: Add the release installer and the Installing section

## TASK-DESCRIPTION
- Create: `prompts/features/07-release-installer.md`
- Modify: `prompts/features/00-features.md`
- Modify: `scripts/install.sh`
- Modify: `scripts/release-package.sh`
- Modify: `tests/11-release-install.sh`
- Modify: `README.md`
- Modify: `prompts/tasks/00-tasks.md`
- Modify: `TODO.md`

The release installer follows the AppImage project: one command fetches the
release tarball for this machine, verifies it against `SHA256SUMS`, installs it
into `$PREFIX`, and then registers the vCard handler. It supports a pinned
`VERSION`, a local `VCARD_INTEGRATION_TARBALL`, the
`VCARD_INTEGRATION_SKIP_VERIFY` override, a source-archive build when no
prebuilt asset matches the machine, and `VCARD_INTEGRATION_NO_HANDLER=1` to
skip the registration.

`README.md` gains an `Installing` section with the one-line command, the
`PREFIX` and `VERSION` controls, the handler step and its reversal, the
`make install` path for a checkout, and `make release-publish`. The generated
release notes describe the same behavior.

## TASK-OUTPUT
Produce these complete files in this order, then the `VERIFICATION:` line:
1. `prompts/features/07-release-installer.md`
2. `prompts/features/00-features.md`
3. `scripts/install.sh`
4. `scripts/release-package.sh`
5. `tests/11-release-install.sh`
6. `README.md`
7. `prompts/tasks/00-tasks.md`
8. `TODO.md`

## TASK-CONTEXT
<note>
The reference is `gnome-appimage-integration/README.md` and its
`scripts/install.sh`.  This project's handler is registered with
`vcard-integrate handler install` and reversed with
`vcard-integrate handler uninstall`.  A handler failure during the release
install is reported rather than fatal, because the tools are installed either
way; `make install` is the path that fails closed.
</note>
<constraint>
The installer must verify the download and must not prompt.  The handler step
must be skippable and reversible.
</constraint>

## TASK-FILES

| Operation | Path |
|---|---|
| create | `prompts/features/07-release-installer.md` |
| modify | `prompts/features/00-features.md` |
| modify | `scripts/install.sh` |
| modify | `scripts/release-package.sh` |
| modify | `tests/11-release-install.sh` |
| modify | `README.md` |
| modify | `prompts/tasks/00-tasks.md` |
| modify | `TODO.md` |

## TASK-VERIFY
- Run: `make test` from the repository root.
- Expected: exit status 0, with `11-release-install` reporting `ok` for the local, HTTP, handler, and opt-out cases.
- Run: `VERSION=v0.1.0 PREFIX=/tmp/vcard-prefix HOME=/tmp/vcard-home XDG_DATA_HOME=/tmp/vcard-home/.local/share XDG_CONFIG_HOME=/tmp/vcard-home/.config sh scripts/install.sh`.
- Expected: the download matches its digest, the programs install, and `handler status` reports both types with `vcard-handler.desktop`.

## TASK-FEATURES
- `prompts/features/07-release-installer.md`

## TASK-ACCEPTANCE
- `RELEASE-INSTALLER-R001`
- `RELEASE-INSTALLER-R002`
- `RELEASE-INSTALLER-R003`
- `RELEASE-INSTALLER-R004`
- `RELEASE-INSTALLER-R005`
- `RELEASE-INSTALLER-R006`
- `RELEASE-INSTALLER-R007`
- `RELEASE-INSTALLER-R008`

OUTPUT: the complete files in the order listed, ending with the `VERIFICATION:` line.
