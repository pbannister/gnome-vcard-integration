# Task: Clear the tab-selection focus in the viewer

## TASK-DESCRIPTION
- Modify: `sources/tools/vcard_viewer_ui.cpp`
- Modify: `prompts/features/06-command-line-tools.md`
- Modify: `prompts/tasks/00-tasks.md`
- Modify: `TODO.md`

Selecting a tab makes `GtkNotebook` focus the first focusable child of the new
page.  In the `vCard` and `About` tabs that child is a selectable title label, so
the title arrives selected.  After the page switch settles, clear the window
focus; the values remain selectable by hand, but opening a tab selects nothing.
Clear the initial focus after the window is presented in the same way.

## TASK-OUTPUT
Produce these complete files in this order, then the `VERIFICATION:` line:
1. `sources/tools/vcard_viewer_ui.cpp`
2. `prompts/features/06-command-line-tools.md`
3. `prompts/tasks/00-tasks.md`
4. `TODO.md`

## TASK-CONTEXT
<note>
The focus is cleared with `gtk_window_set_focus(window, NULL)` from an idle
callback connected to the notebook's `switch-page` signal, and once after
`gtk_window_present`.
</note>
<constraint>
Keep the labels selectable when a person clicks them; clear only the automatic
focus and selection that a tab switch causes.
</constraint>

## TASK-FILES

| Operation | Path |
|---|---|
| modify | `sources/tools/vcard_viewer_ui.cpp` |
| modify | `prompts/features/06-command-line-tools.md` |
| modify | `prompts/tasks/00-tasks.md` |
| modify | `TODO.md` |

## TASK-VERIFY
- Run: `make test` from the repository root.
- Expected: exit status 0, with `95-viewer` reporting `ok`.
- Run: `vcard-viewer dataflow.in/vcard/simple.vcf` on a display and switch between the tabs.
- Expected: no title is selected when a tab opens; clicking a label still selects it.

## TASK-FEATURES
- `prompts/features/06-command-line-tools.md`

## TASK-ACCEPTANCE
- `COMMAND-LINE-TOOLS-R020`

OUTPUT: the complete files in the order listed, ending with the `VERIFICATION:` line.
