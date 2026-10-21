# TODO

## Open Questions

* [ ] publish the standard page set and a release (phase 4).
      — The page set builds and the release scripts exist, but the project is
      not yet registered with the homelab's `pages_source` and no release
      tarball has been published.
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

* [x] project start (2026-10-21; owner instruction):
    * [x] derived the project from `00-project-skeleton/` and kept the shared
          rule corpus, so `scripts/skeleton-diff.sh` reports no rule drift.
    * [x] declared the project settings in `README.md`: automatic task commits
          enabled, the vCard privacy boundary, and that there are no live-state
          tests.
    * [x] wrote features `03-vcard-reader`, `04-contact-renderer`,
          `05-vcard-handler`, and `06-command-line-tools`, with tasks `02`–`05`
          claiming their requirement identifiers.
    * [x] implemented the reader (vCard 2.1, 3.0, and 4.0), the SVG contact
          card renderer, the MIME handler, the three command-line tools, and the
          GTK4 viewer.
    * [x] added synthetic fixtures under `dataflow.in/vcard/`, the C++ unit
          tests, and the shell tests in the reserved bands.
