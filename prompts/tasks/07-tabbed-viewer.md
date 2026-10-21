# Task: Present the viewer as a three-tab panel

## TASK-DESCRIPTION
- Create: `sources/tools/viewer_content.h`
- Create: `sources/tools/viewer_content.cpp`
- Create: `tests/viewer_content_test.cpp`
- Create: `tests/82-viewer-content.sh`
- Modify: `sources/tools/vcard_viewer_ui.cpp`
- Modify: `sources/CMakeLists.txt`
- Modify: `prompts/features/06-command-line-tools.md`
- Modify: `prompts/tasks/00-tasks.md`
- Modify: `README.md`
- Modify: `TODO.md`

`vcard-viewer` must present three tabs.

- `vCard` keeps the contact page the viewer shows today.
- `Discovered` runs `vcard-inspect FILE` and shows the exact command line and
  the combined output the tool printed, so the graphical view has a command-line
  equivalent on screen.
- `About` shows the project name, the build-time version, and the GitHub
  repository link.

The non-GTK content is a small library so it can be tested without a display:
the shell quoting of the command, the exact command and output capture, and the
About identity.  The viewer links that library and puts the three pages in a
`GtkNotebook`.  The vCard page is unchanged.

## TASK-OUTPUT
Produce these complete files in this order, then the `VERIFICATION:` line:
1. `sources/tools/viewer_content.h`
2. `sources/tools/viewer_content.cpp`
3. `tests/viewer_content_test.cpp`
4. `tests/82-viewer-content.sh`
5. `sources/tools/vcard_viewer_ui.cpp`
6. `sources/CMakeLists.txt`
7. `prompts/features/06-command-line-tools.md`
8. `prompts/tasks/00-tasks.md`
9. `README.md`
10. `TODO.md`

## TASK-CONTEXT
<note>
The viewer is built only where the GTK4 development files are present.  The
`Discovered` content runs the installed `vcard-inspect` beside the viewer, or
one found on `PATH`, so it shows the command a person would type.  The About
repository link is `https://github.com/pbannister/gnome-vcard-integration`.
</note>
<constraint>
The `vCard` tab must keep the contact information the viewer already showed;
do not move contact details into another tab or remove them.
</constraint>

## TASK-FILES

| Operation | Path |
|---|---|
| create | `sources/tools/viewer_content.h` |
| create | `sources/tools/viewer_content.cpp` |
| create | `tests/viewer_content_test.cpp` |
| create | `tests/82-viewer-content.sh` |
| modify | `sources/tools/vcard_viewer_ui.cpp` |
| modify | `sources/CMakeLists.txt` |
| modify | `prompts/features/06-command-line-tools.md` |
| modify | `prompts/tasks/00-tasks.md` |
| modify | `README.md` |
| modify | `TODO.md` |

## TASK-VERIFY
- Run: `make test` from the repository root.
- Expected: exit status 0, with `82-viewer-content` and `95-viewer` reporting `ok`.
- Run: `vcard-viewer dataflow.in/vcard/simple.vcf` on a display.
- Expected: a window with `vCard`, `Discovered`, and `About` tabs; the Discovered tab shows the inspector command and report; the About tab shows the version and repository link.

## TASK-FEATURES
- `prompts/features/06-command-line-tools.md`

## TASK-ACCEPTANCE
- `COMMAND-LINE-TOOLS-R016`
- `COMMAND-LINE-TOOLS-R017`
- `COMMAND-LINE-TOOLS-R018`
- `COMMAND-LINE-TOOLS-R019`

OUTPUT: the complete files in the order listed, ending with the `VERIFICATION:` line.
