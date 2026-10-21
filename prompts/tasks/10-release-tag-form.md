# Task: Generate release tags as v<date>-<branch>

## TASK-DESCRIPTION
- Modify: `scripts/release-publish.sh`
- Modify: `tests/81-release-publish.sh`
- Modify: `README.md`
- Modify: `prompts/tasks/00-tasks.md`
- Modify: `TODO.md`

When `release-publish.sh` has to invent a tag, it must use `v<date>-<branch>`,
with the build date in `YYYY-MM-DD` form and the current branch, for example
`v2026-10-21-master`.  A tag already on the commit is still reused, an explicit
`TAG` still overrides the generated one, and a checkout that is not on a branch
is refused with a request to set `TAG` rather than guessing.

The release-publish test must publish without a `TAG` and assert the generated
form, then re-publish with the same tag and assert the assets are re-uploaded.
Existing published tags are not renamed.

## TASK-OUTPUT
Produce these complete files in this order, then the `VERIFICATION:` line:
1. `scripts/release-publish.sh`
2. `tests/81-release-publish.sh`
3. `README.md`
4. `prompts/tasks/00-tasks.md`
5. `TODO.md`

## TASK-CONTEXT
<note>
The version string is still the build-time `<date>-<branch>-<hash>`; the tag is
the human-facing name and carries only date and branch.  Two releases from the
same branch on the same day therefore collide, and the second refuses unless
`TAG` names a distinct tag.
</note>
<constraint>
Do not move or rename an existing published tag as part of this change.
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
- Run: `sh scripts/release-publish.sh` from a clean scratch repository with no tag on HEAD.
- Expected: the created tag matches `v<date>-<branch>` and is pushed with the branch.

OUTPUT: the complete files in the order listed, ending with the `VERIFICATION:` line.
