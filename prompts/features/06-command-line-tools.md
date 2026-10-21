# Feature: Command-Line Tools

## Purpose

Everything the desktop does must also be available on the command line, so a
card can be read, drawn, and registered from a script.

## Requirements

- `COMMAND-LINE-TOOLS-R001` — `vcard-inspect FILE` must print a human-readable report of the file, the cards, each contact's facts, the photo, and every warning.
- `COMMAND-LINE-TOOLS-R002` — `vcard-inspect --json FILE` must print the same facts as one JSON document.
- `COMMAND-LINE-TOOLS-R003` — `vcard-inspect --properties FILE` must also list every property with its group, its parameters, and its decoded value.
- `COMMAND-LINE-TOOLS-R004` — `vcard-inspect` must report a card with warnings in its output and still exit zero, so a caller can read the report; the JSON member `warnings` is the machine-readable signal.
- `COMMAND-LINE-TOOLS-R005` — `vcard-render FILE` must write an SVG card to standard output, and `--output FILE` must write it to a file.
- `COMMAND-LINE-TOOLS-R006` — `vcard-render --index N` must select the contact, and `--no-photo` must omit the photograph.
- `COMMAND-LINE-TOOLS-R007` — `vcard-integrate handler status|install|uninstall` must drive the handler integration and print the result of each action.
- `COMMAND-LINE-TOOLS-R008` — `vcard-integrate handle FILE` must be the desktop entry point.
- `COMMAND-LINE-TOOLS-R009` — Every tool must print `<name> <version> (build <counter>)` for `--version`, where the version is the build-time version.
- `COMMAND-LINE-TOOLS-R010` — Every tool must print usage for `--help` and exit zero.
- `COMMAND-LINE-TOOLS-R011` — An unknown option, an unknown command, an unknown handler subcommand, or a missing required argument must print usage and exit with status 2.
- `COMMAND-LINE-TOOLS-R012` — A file that cannot be read must print `<program>: <message>` on standard error and exit with status 1.
- `COMMAND-LINE-TOOLS-R013` — `vcard-inspect` and `vcard-render` must accept exactly one file.
- `COMMAND-LINE-TOOLS-R014` — `vcard-render` must print a warning on standard error, and still write the card, when the photograph could not be embedded.
- `COMMAND-LINE-TOOLS-R015` — `vcard-viewer FILE.vcf` must open a window titled with the contact's name and carrying the `vcard-viewer` WM class, and must open an explanatory window rather than exiting silently when the file is not a readable card.
- `COMMAND-LINE-TOOLS-R016` — `vcard-viewer` must present a tabbed panel whose tabs are named `vCard`, `Discovered`, and `About`.
- `COMMAND-LINE-TOOLS-R017` — The `Discovered` tab must show the exact `vcard-inspect` command that was run and its combined standard output and standard error, so the graphical view and the command line can be compared.
- `COMMAND-LINE-TOOLS-R018` — The `About` tab must show the project name, the build-time version, and the GitHub repository link.
- `COMMAND-LINE-TOOLS-R019` — The `vCard` tab must show the same contact information the viewer showed before the tabbed panel.

## Behavior

- `vcard-inspect FILE` on the fixture prints `cards: 1` and names the contact.
- `vcard-inspect --json FILE` parses as JSON and reports `cards[0].formattedName`.
- On a card with no `FN`, the report names the warning and the JSON `warnings` array is not empty, and the command exits zero.
- `vcard-render --output card.svg FILE` leaves a non-empty SVG carrying the display name.
- `vcard-inspect` with no file argument exits 2; with a path that does not exist it exits 1.
- `vcard-viewer` on the fixture leaves a window named for the contact on the display, and `vcard-integrate handle` puts the same window up.
- The viewer opens on the `vCard` tab, and its `Discovered` tab names the inspector and the file and shows the report.
- The `About` tab shows `GNOME VCARD Integration`, the build-time version, and the repository URL.
- For a file the inspector refuses, the `vCard` tab shows the error and the `Discovered` tab shows the inspector's exact command and error output.

## Dependencies

- `03-vcard-reader.md`
- `04-contact-renderer.md`
- `05-vcard-handler.md`
