# TODO

## Open Questions

* [ ] decode the remaining legacy charset labels, chiefly `WINDOWS-1252` and
      `CP1252`.
      — The reader transcodes `UTF-8`, `US-ASCII`, and `ISO-8859-1`, which covers
      almost every 2.1 file; a Western European file labelled CP1252 with a
      curly quote in it is currently shown as a replacement character.
* [ ] show every contact in a multi-card file in the GTK4 viewer.
      — `vcard-render --index N` and `vcard-inspect` handle every card, but the
      viewer opens the first one; a bundled address book therefore shows one
      person.
* [ ] turn the `CATEGORIES`, `TZ`, `GEO`, and `LANG` properties into visible
      card sections.
      — They are parsed, reported, and kept in the model, but the SVG card
      currently draws only the name, organisation, title, telephones, emails,
      addresses, URLs, note, and photo.
* [ ] add an `audit` command that reports a handler record which disagrees with
      the files on disk, as the AppImage project's `audit` does.
* [ ] add a Nautilus thumbnailer for `text/vcard` that runs `vcard-render`.
* [ ] verify the reader against exports from several real producers
      (Thunderbird, Google Contacts, Apple Contacts, Evolution) before the next
      release.
      — The fixtures are synthetic and cover the documented forms; a producer's
      quirks are best found with a corpus of shapes, not with real personal
      data.
* [ ] write the episode plan for phases 1 through 4 in
      `prompts/episodes/02-episode-plan.md`.

## Recently Completed

* [x] guard the vCard fixtures against real data (2026-10-22; incident):
    * [x] removed the real card from reachable history, deleted the exposed
          tag and release, and republished the clean release.
    * [x] added `tests/04-fixture-privacy.sh`, an allowlist of the reviewed
          synthetic fixtures; a new file fails the suite.
    * [x] the incident account is `records/01-privacy-leak-and-purge.md`.
* [x] generate release tags as `v<date>-<branch>` (2026-10-21; owner instruction):
    * [x] `release-publish.sh` generates `v<date>-<branch>` when the commit has
          no tag, still reuses an existing tag, and still accepts an explicit
          `TAG`.
    * [x] a detached checkout is refused rather than guessed; existing
          `v0.1.0` and `v0.1.1` tags are unchanged.
    * [x] `tests/81-release-publish.sh` publishes with no `TAG` and checks the
          generated form.
* [x] release v0.1.1 (2026-10-21; owner request):
    * [x] the release carries the tabbed viewer, the soft-break fix, and the
          AppImage-style installer.
    * [x] the README and the generated release notes document the one-line
          install and the handler opt-out.
* [x] add the AppImage-style release installer (2026-10-21; owner instruction):
    * [x] `scripts/install.sh` fetches and verifies the release, installs into
          `$PREFIX`, builds from source when no asset matches, and registers the
          vCard handler with a `VCARD_INTEGRATION_NO_HANDLER=1` opt-out.
    * [x] the generated release notes and the README `Installing` section
          describe the one-line install and the reversal command.
    * [x] `tests/11-release-install.sh` covers the handler step and its opt-out
          as well as the local, HTTP, tampered, and unverified cases.
* [x] clear the automatic title selection on a tab switch
      (2026-10-21; owner report):
    * [x] the viewer clears the window focus after `switch-page` and after the
          first presentation, so the `vCard` and `About` titles do not arrive
          selected.
* [x] present the viewer as three tabs (2026-10-21; owner instruction):
    * [x] the `vCard` tab keeps the contact page the viewer already showed.
    * [x] the `Discovered` tab runs `vcard-inspect` and shows the exact command
          and its combined output.
    * [x] the `About` tab shows the project name, the build-time version, and
          the GitHub repository link.
    * [x] the non-GTK content moved into `sources/tools/viewer_content.cpp` and
          is covered by `tests/82-viewer-content.sh`.
* [x] interpret a bare quoted-printable soft break (2026-10-21; owner report):
    * [x] the reader joins a quoted-printable continuation that starts at
          column 0, so an exported 2.1 card's `ADR` and `LABEL` are complete.
    * [x] a five-component `ADR` whose fifth part looks like a postal code is
          read as a postal code, so the short form displays without an extra
          comma.
    * [x] added a synthetic fixture and reader and command-line regression
          tests, and `vcard-inspect` now reports the label beside the address.
    * [x] no value from the supplied card is committed; the fixture is
          invented.
* [x] publish the page set and release `v0.1.0` (2026-10-21; phase 4):
    * [x] added `scripts/release-publish.sh`, `make release-publish`, and
          `tests/81-release-publish.sh`; the publish path builds from the
          released commit, validates the tag, skips a redundant push, and
          re-uploads the assets.
    * [x] created `github.com/pbannister/gnome-vcard-integration` and published
          `v0.1.0` with the tarball, `SHA256SUMS`, and generated release notes.
    * [x] registered the project in the homelab's `sources/projects.yaml` with
          a `pages_source` of this repository's `site.out/`, and deployed the
          standard page set through the homelab publisher.
    * [x] the pages are live at
          <https://labs.bannister.us/projects/gnome-vcard-integration/>.
    * [x] the first `v0.1.0` package named an older staged tree; the publish
          path now stages the current build and exports its version, and the
          corrected tarball, `VERSION`, and release notes were re-uploaded.
* [x] project start (2026-10-21; owner instruction):
    * [x] derived the project from `00-project-skeleton/` and kept the shared
          rule corpus, so `scripts/skeleton-diff.sh` reports no rule drift.
    * [x] declared the project settings in `README.md`: automatic task commits
          enabled, the vCard privacy boundary, and that there are no live-state
          tests.
    * [x] wrote features `03-vcard-reader`, `04-contact-renderer`,
          `05-vcard-handler`, and `06-command-line-tools`, with tasks `01`–`04`
          claiming their requirement identifiers.
    * [x] implemented the reader (vCard 2.1, 3.0, and 4.0), the SVG contact
          card renderer, the MIME handler, the three command-line tools, and the
          GTK4 viewer.
    * [x] added synthetic fixtures under `dataflow.in/vcard/`, the C++ unit
          tests, and the shell tests in the reserved bands.
