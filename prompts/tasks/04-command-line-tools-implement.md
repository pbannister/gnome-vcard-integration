# Task: Implement the Command-Line Tools feature

## TASK-DESCRIPTION
- Create: `sources/tools/vcard_inspect.cpp`
- Create: `sources/tools/vcard_render.cpp`
- Create: `sources/tools/vcard_viewer_ui.cpp`
- Create: `tests/60-command-line-tools.sh`
- Create: `tests/95-viewer.sh`
- Modify: `sources/CMakeLists.txt`

The tools implement the requirements in
`prompts/features/06-command-line-tools.md`.
`vcard-inspect` prints the report, the JSON form, and the optional property
list. `vcard-render` writes the SVG card and selects a card by index.
`vcard-viewer` is the GTK4 window the handler opens, built only where the GTK4
development files are present. Every tool keeps the exit-code contract: 0 for a
readable file, 1 for a file that cannot be read, and 2 for a usage error.

## TASK-OUTPUT
Produce these complete files in this order, then the `VERIFICATION:` line:
1. `sources/tools/vcard_inspect.cpp`
2. `sources/tools/vcard_render.cpp`
3. `sources/tools/vcard_viewer_ui.cpp`
4. `tests/60-command-line-tools.sh`
5. `tests/95-viewer.sh`
6. `sources/CMakeLists.txt`

## TASK-CONTEXT
<note>
The JSON form is written directly by `vcard_inspect.cpp` with a string escaper,
so the project does not carry a general JSON library for one output.
The viewer test runs the window under a private `Xvfb` display and checks the
title and the WM class through the X server, which is how the dock matches a
running window to the handler entry.
</note>
<constraint>
The viewer test must use its own private display and must not touch the real
session.
</constraint>

## TASK-FILES

| Operation | Path |
|---|---|
| create | `sources/tools/vcard_inspect.cpp` |
| create | `sources/tools/vcard_render.cpp` |
| create | `sources/tools/vcard_viewer_ui.cpp` |
| create | `tests/60-command-line-tools.sh` |
| create | `tests/95-viewer.sh` |
| modify | `sources/CMakeLists.txt` |

## TASK-VERIFY
- Run: `make test` from the repository root.
- Expected: exit status 0, with `60-command-line-tools` reporting `ok` and `95-viewer` reporting either `ok` or a clean skip.

## TASK-FEATURES
- `prompts/features/06-command-line-tools.md`

## TASK-ACCEPTANCE
- `COMMAND-LINE-TOOLS-R001`
- `COMMAND-LINE-TOOLS-R002`
- `COMMAND-LINE-TOOLS-R004`
- `COMMAND-LINE-TOOLS-R005`
- `COMMAND-LINE-TOOLS-R009`
- `COMMAND-LINE-TOOLS-R011`
- `COMMAND-LINE-TOOLS-R012`
- `COMMAND-LINE-TOOLS-R015`

OUTPUT: the complete files in the order listed, ending with the `VERIFICATION:` line.
