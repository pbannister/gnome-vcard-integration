# Task: Put the date first in a release title

## TASK-DESCRIPTION
- Modify: `scripts/release-publish.sh`
- Modify: `tests/81-release-publish.sh`
- Modify: `README.md`
- Modify: `prompts/tasks/00-tasks.md`
- Modify: `TODO.md`

GitHub's release list is narrow, so a long title is truncated at the end.  The
default release title must lead with the date, then the project name, with the
branch and commit after it:

```
2026-10-22 — GNOME VCARD Integration (master-c388c1e)
```

The date is the build date from the version string, in `YYYY-MM-DD` form.  An
explicit `TITLE` still overrides the default.  The release-publish test must
assert the date-first title, and the existing release titles are updated to the
same order.

## TASK-OUTPUT
Produce these complete files in this order, then the `VERIFICATION:` line:
1. `scripts/release-publish.sh`
2. `tests/81-release-publish.sh`
3. `README.md`
4. `prompts/tasks/00-tasks.md`
5. `TODO.md`

## TASK-CONTEXT
<note>
The tag and the release title are separate: the tag remains `v<date>-<branch>`,
while the title is for people reading the release list.  The release notes and
assets are unchanged.
</note>
<constraint>
Do not change an existing tag or release asset while correcting its title.
</constraint>

## TASK-FILES

| Operation | Path |
|---|---|
| modify | `scripts/release-publish.sh` |
| modify | `tests/81-release-publish.sh` |
| modify | `README.md` |
| modify | `prompts/tasks/00-tasks.md` |
| modify | `TODO.md` |

## TASK-VERIFY
- Run: `make test` from the repository root.
- Expected: exit status 0, with `81-release-publish` reporting `ok`.
- Run: `gh release list --limit 3` after updating the existing titles.
- Expected: each listed title begins with `YYYY-MM-DD — GNOME VCARD Integration`.

OUTPUT: the complete files in the order listed, ending with the `VERIFICATION:` line.
