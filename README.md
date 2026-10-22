# GNOME VCARD Integration

This project handles vCard files, usually named with the `.vcf` extension, on the
GNOME desktop.

A vCard is a small text file that carries a contact: a name, telephone numbers,
email addresses, postal addresses, a photograph, notes, and whatever else the
writer decided to include.
The format has three generations in the wild — 2.1, 3.0, and 4.0 — and the older
ones use quoted-printable encoding, bare type parameters, and folded lines, so a
reader that only understands the newest form loses most real contact files.
A stock desktop can show nothing useful for one, so a downloaded card is an
opaque file.

This project reads a card, reports what it contains, draws it as a contact card,
and registers itself as the application that opens the vCard types.

- `vcard-inspect` reads a file and prints the contact, its properties, its
  warnings, and the raw property list, or the same facts as JSON.
- `vcard-render` draws a contact as a standalone SVG card, carrying the name,
  organisation, telephone numbers, email addresses, postal address, links, note,
  and the embedded photo.
- `vcard-integrate handler install` associates `text/vcard` and `text/x-vcard`
  with this project, gives the file type its icon, and remembers what opened
  those types before, so the change can be reversed.
- `vcard-viewer` is the GTK4 window a double-clicked card opens in.

Everything the desktop does is also a command line.

**Status: phase 4 — complete.** The reader, the renderer, the handler, the
tools, and the viewer are built and tested; the page set is published at
<https://labs.bannister.us/projects/gnome-vcard-integration/>, and release
`v0.1.1` (the latest) is at
<https://github.com/pbannister/gnome-vcard-integration/releases/latest>.
The parser reads vCard 2.1, 3.0, and 4.0, unfolding long lines, decoding
quoted-printable and base64 values, and accepting both the 2.1 bare type
parameters and the later `TYPE=` form. The GTK4 viewer is built and exercised
here under Xvfb, and is skipped where the GTK4 development files are absent.

- Source: <https://github.com/pbannister/gnome-vcard-integration>
- Release: <https://github.com/pbannister/gnome-vcard-integration/releases/latest>
- Pages: <https://labs.bannister.us/projects/gnome-vcard-integration/>

## Installing

Easy to grab a release off Github with a one script install:

```sh
curl -fsSL https://raw.githubusercontent.com/pbannister/gnome-vcard-integration/master/scripts/install.sh | sh
```

The script fetches the release tarball for this machine, checks it against the
release's `SHA256SUMS`, and installs into `$HOME/.local`. `PREFIX` installs
elsewhere, `VERSION` pins a release tag, and where no prebuilt build exists for
the machine it builds from the source archive instead.

It then registers the vCard handler, so double-clicking a `.vcf` opens the
viewer and vCard files carry the project's icon:

```sh
vcard-integrate handler install     # what the installer runs
vcard-integrate handler uninstall   # undo it, restoring the previous handler
```

That step is the one part of the install that changes how the desktop behaves:
it rewrites the user's MIME defaults for `text/vcard` and `text/x-vcard`,
records what they were, and installs a definition of those types when the
machine has none. Set `VCARD_INTEGRATION_NO_HANDLER=1` to install the tools and
register nothing; `handler install` and `handler uninstall` remain available at
any time. A registration that cannot happen is reported, and the installed
tools stay in place.

For a checkout, `make install` installs the built programs into `$HOME/.local`
(`PREFIX` overrides) and registers the handler in the same way;
`VCARD_INTEGRATION_NO_HANDLER=1` skips that step. Unlike the release script, a
`make install` that cannot register the handler exits nonzero, so a scripted
install never reports success with the association missing.

The install writes only under `$XDG_DATA_HOME`, `$XDG_CONFIG_HOME`, and the
temporary directory, and never needs root.

`scripts/release-package.sh` makes the assets a release publishes, and
`make release-publish` (with the GitHub CLI authenticated) tags the commit and
publishes them.

## Commands

| Command | Purpose |
| ------- | ------- |
| `vcard-inspect FILE.vcf` | report the cards, the contact facts, the encoded properties, and any warning |
| `vcard-inspect --json FILE.vcf` | print the same facts as one JSON document |
| `vcard-inspect --properties FILE.vcf` | also list every property with its group, parameters, and value |
| `vcard-render FILE.vcf` | write the first contact as an SVG card to standard output |
| `vcard-render --index N FILE.vcf` | draw the Nth contact in a multi-card file, counting from 1 |
| `vcard-render --no-photo FILE.vcf` | draw the card without the embedded photograph |
| `vcard-render --output FILE FILE.vcf` | write the SVG to a file |
| `vcard-integrate handler status` | report each type's default, the entry, the definition, the type icons, and the record |
| `vcard-integrate handler install` | write the MIME definition, the handler entry, the icons, and the record, and set both types to this handler |
| `vcard-integrate handler uninstall` | restore the recorded previous handlers and remove what this project installed |
| `vcard-integrate handle FILE.vcf` | the desktop entry point: open the card in the viewer, or print the equivalent commands |
| `vcard-viewer FILE.vcf` | the GTK4 tabbed window: the card, the exact `vcard-inspect` command and output, and the project's name, version, and repository |
| `vcard-inspect --version` | print the build-time version |

`vcard-inspect` exits zero on a file it could read, including one with warnings:
a missing `FN`, an unknown `VERSION`, or a malformed property is a finding about
the file, and the report says so in words and in `warnings`.
A nonzero exit means the file could not be read as vCard at all, so a script can
tell "this card is incomplete" from "this is not a card".

## What the handler writes

| Path under `$XDG_DATA_HOME` | What it is |
| --------------------------- | ---------- |
| `mime/packages/vcard.xml` | the definition of `text/vcard` and its `text/x-vcard` alias, with the globs, the magic test, and this project's icon |
| `applications/vcard-handler.desktop` | the handler entry, whose `Exec` is `<tool> handle %f` |
| `icons/hicolor/scalable/apps/vcard-handler.svg` | the handler's icon |
| `icons/hicolor/scalable/mimetypes/text-vcard.svg` | the file icon for a card |
| `icons/hicolor/scalable/mimetypes/text-x-vcard.svg` | the file icon for the legacy type |
| `gnome-vcard-integration/vcard-handler.manifest` | the record: what was written, what was displaced, and what opened each type before |

The record is what makes an uninstall possible; without it, `handler uninstall`
refuses rather than guessing.
A definition at `mime/packages/vcard.xml` that this project did not write is
backed up first and restored byte for byte, because a distribution may already
define the type and its definition is not this project's to lose.

## Top-Level Map

- `README.md` is the project overview.
- `AGENTS.md` is the coding-agent entry point; it points at `prompts/` and defines no rule.
- `TODO.md` tracks pending and completed project tasks.
- `PHASES.md` records the current phase and its milestones.
- `prompts/` contains LLM interaction rules, common requirements, feature requirements, task definitions, episode work orders, and the capability conventions (`prompts/capabilities/`) that are loaded only when a task touches one.
- `documents/` contains human-consumption documents: the interaction pattern, worked examples, and the format research.
- `records/` contains version-controlled outcome, incident, and handoff records.
- `tools/` contains tool-specific rules.
- `sources/` contains the C++ reader, the renderer, the integration, and the command-line tools.
- `scripts/` contains project scripts.
- `tests/` contains tests and validation code.
- `dataflow.in/` contains input data, including the synthetic card fixtures.
- `dataflow.out/` contains generated data output and the build tree (not version-controlled).
- `logs/` contains generated logs (not version-controlled).
- `site.in/` contains static-site input.
- `site.out/` contains generated static-site output (not version-controlled).
- `Makefile` drives the build (`make build`), the tests (`make test`), the pages (`make site`), the install (`make install`), and cleanup (`make clean`).

## Building and Testing

- `make build` configures and compiles the reader, the renderer, the integration, and the tools into `dataflow.out/build/`.
- `make install` installs `vcard-inspect`, `vcard-render`, and `vcard-integrate` (and `vcard-viewer` where GTK4 is present) into `$HOME/.local/bin` (`PREFIX` overrides), with the icon, and registers the handler; `VCARD_INTEGRATION_NO_HANDLER=1` skips the registration.
- `make test` runs every test in `tests/` and writes a timestamped log to `logs/`.
- `make status` reports the phase, the next open TODO item, the last test run, and the working-tree state in one screen.
- `make context` measures the always-loaded context (the entry point plus the mandatory baseline) against the budget `prompts/02-workflow.md` declares.
- `make maturity` scores the machine-checkable subset of the prompt-maturity rubric.
- `make site` builds the published page set, then runs the sanitization gate over `site.out/` (`make check` runs the gate alone).
- `make release` builds, stages the programs into `dataflow.out/build/bin/`, checks that the version names the current commit, and writes the release assets into `dataflow.out/release/`.
- `make release-publish` builds from the commit being released, tags it, and creates or updates the GitHub release with the tarball and its checksum; it needs the GitHub CLI. Without `TAG`, the tag is `v<date>-<branch>` (for example `v2026-10-22-master`); a tag already on the commit is reused, and an explicit `TAG` overrides it. A generated tag that already points at another commit is refused, so a second release on the same branch and day needs an explicit `TAG`.
- The default release title leads with the date, then the project name, because GitHub's release list truncates long titles: `2026-10-22 — GNOME VCARD Integration (master-c388c1e)`.
- `make clean` removes generated output.

The C++ build uses CMake with the highest warning level and treats warnings as
errors.

The tests follow the skeleton's reserved bands: `04` for the fixture privacy
guard, `14` for the context budget, `15` for the vCard reader, `40` for the
renderer, `60` for the command-line tools, `82` for the viewer's Discovered and
About content, and `90`–`95` for the handler install and uninstall and the
viewer.
The C++ unit tests (`tests/*_test.cpp`, `tests/test_harness.h`) are built by
`sources/CMakeLists.txt` and run by a shell test; `tests/lib/test_helpers.sh`
carries `build_program` and the fixture paths.
The fixtures under `dataflow.in/vcard/` are synthetic on purpose: a real vCard
belongs to a person, so every value in them is invented.
The handler test points `HOME` and the XDG paths at a throwaway directory, so it
exercises `xdg-mime`, `update-mime-database`, and the icon cache without
touching this machine's own desktop.

## Source Layout

- `sources/vcard/` contains the property model, the reader, and the decoders for folding, quoted-printable, base64, and escaped text.
- `sources/render/` contains the SVG contact-card renderer.
- `sources/integration/` contains the MIME-definition and handler-database integration.
- `sources/util/` contains file, process, and base64 helpers.
- `sources/tools/` contains the command-line front ends, the GTK4 viewer, its non-GTK tab content, and the icon.
- `sources/version/` contains the build-time version reporting.
- `sources/CMakeLists.txt` defines the targets; the build tree is written to `dataflow.out/build/`.

The graphical viewer is C++ with GTK4.
The build needs the GTK4 development files (`libgtk-4-dev` for Debian and
Ubuntu, `gtk4-devel` for Fedora); without them the viewer target is skipped and
`vcard-integrate handle` prints the equivalent command-line commands instead.
`tests/95-viewer.sh` opens the window on a private Xvfb display and checks the
window's title and WM class, so the viewer is covered by `make test` wherever
GTK4 and Xvfb are present, and skips cleanly where they are not.
Nothing in the viewer needs `librsvg`: an embedded PNG or JPEG photo is shown
through `GdkTexture`, and the only SVG the project writes is the rendered card
file.

## Research Findings

The format research the reader implements is recorded in `documents/`.

- `documents/10-vcard-format.md` — the vCard generations, the line grammar, folding, parameters, escaping, quoted-printable and base64 values, the structured `N` and `ADR` values, dates, and what this project does and does not yet do with them.
- `documents/11-mime-type-association.md` — how the desktop resolves a file type to an application, what shared-mime-info already defines for vCard, and exactly what this project writes to become the handler.
- `documents/12-vcard-knowns-and-unknowns.md` — what is documented, observed, and only conventional about the format, what is unknown, and which decisions this project made where the format is silent.

## For Human Contributors

Human contributors should begin by reading:

- `prompts/README.md`
- `documents/README.md`

The prompt corpus is pinned, not copied: `rules.lock` records the
`00-project-rules` version and a digest per vendored file, and
`scripts/rules-check.sh` fails when one is edited. Customise by extending — a
project-specific rule goes in a local prompt file numbered after the shared ones —
rather than by editing shared text, which the check treats as drift.
`tests/09-prompt-contract.sh` validates the corpus itself.
`PHASES.md` needs no exception: it is declared in `prompts/10-project.md`, which
is how a project adds a root file without touching a shared rule.

## For the LLM

The mandatory baseline is defined in `prompts/02-workflow.md` section 1, and its
size is measured by `make context` against the budget that same section
declares. It is, in order:

1. `prompts/01-contract.md`
2. `prompts/02-workflow.md`
3. `prompts/03-conventions.md`
4. `prompts/common/02-universal-rules.md`
5. `prompts/flavors/01-semantic-sort-naming.md`
6. `AGENTS.md` (the entry point, read first)

- These define the interaction rules, the workflow, the formatting conventions, and the global rules.
- The LLM must follow the workflow defined in `prompts/02-workflow.md` for every task.
- A capability convention under `prompts/capabilities/` is loaded only for a task that touches that capability, so it stays out of the always-loaded context.
- This project targets C++, so `prompts/flavors/02-cpp-conventions.md` applies to source work.
- `prompts/common/03-glossary.md` is a reference: load it when a term's meaning decides the work.

## Project Scope

The project reads one file format and integrates it into the desktop.

1. Read a vCard file: its cards, its lines, its folded continuations, its parameters, and its encoded values, across versions 2.1, 3.0, and 4.0.
2. Decode a property: quoted-printable, base64, the `\n`-style escapes, and the structured `N`, `ADR`, and `ORG` values.
3. Draw the contact as a standalone SVG card, with a photo when one is embedded and links only for `mailto:`, `tel:`, `http:`, and `https:`.
4. Register `text/vcard` and `text/x-vcard` with the desktop, and reverse that registration from a record.

The project never modifies a vCard, and it never requires root.
It reads a card's contents in order to display them; it does not transmit them
anywhere, and it does not fetch a photo that is a remote URI.

## Project Settings

- Automatic task commits: enabled (see `prompts/02-workflow.md` §7.1).
- Privacy boundary: a real vCard belongs to a person and carries a name, telephone numbers, an address, an email address, and sometimes a photograph. No real vCard, and no value taken from one, is committed to this repository; the fixtures under `dataflow.in/vcard/` are synthetic and exist so the tests have a card that is structurally real and nobody's. A new fixture must be reviewed as synthetic and added to the allowlist in `tests/04-fixture-privacy.sh`; the test fails on an unreviewed file.
- Live-state tests: none. Every test runs against fixtures and a throwaway `HOME`, so `make test` touches neither the real desktop nor the network.

## Project Pages (publishing conventions)

- A project derived from the skeleton publishes the standard page set
  (status, dashboard, condensed todo/prompts/documents) per
  `prompts/features/02-project-pages.md` and
  `documents/06-project-pages.md`.
- `make site` builds the full set (`scripts/site-build.sh` +
  `scripts/site-condense.sh`).
- Publishing goes through the homelab project (homelab-publish): the project
  registers once via `pages_source` and does not push to the web server
  itself — `make deploy` is retired.
- The project is registered in the homelab's `sources/projects.yaml` as
  `gnome-vcard-integration`, with `pages_source` pointing at this repository's
  `site.out/`; the pages are live at
  <https://labs.bannister.us/projects/gnome-vcard-integration/>.

## Canonical Files

The following filenames are canonical and must not be renamed or duplicated without an explicit task:

- `README.md`
- `AGENTS.md`
- `TODO.md`
- `PHASES.md`
- `Makefile`
- `package.json`
- `.gitignore`

Tool-specific root files (for example `.aider.conf.yml`, `.aiderignore`) are permitted when a file under `tools/` declares them; see `prompts/01-contract.md` section 7.
