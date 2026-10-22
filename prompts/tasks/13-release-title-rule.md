# Task: Codify the release title convention

## TASK-DESCRIPTION
- Modify: `prompts/capabilities/04-release-and-publishing.md`
- Modify: `prompts/tasks/00-tasks.md`
- Modify: `TODO.md`

The shared release capability gains the title rule: a release title leads with
the most significant information, so a narrow release list does not truncate
it. For a dated release, the date in `YYYY-MM-DD` form comes first, then the
project name, then the branch and commit; an explicit title overrides the
default.

The skeleton's copy of the capability is the shared source and is changed as
the authorized cross-project step. The project's copy is synced byte for byte,
so `scripts/skeleton-diff.sh` no longer reports this capability as drift.

## TASK-OUTPUT
Produce these complete files in this order, then the `VERIFICATION:` line:
1. `prompts/capabilities/04-release-and-publishing.md`
2. `prompts/tasks/00-tasks.md`
3. `TODO.md`

## TASK-CONTEXT
<note>
The release-publish script already emits the date-first title and the test
asserts it. This task makes that behaviour a rule, not just an implementation
choice.
</note>
<constraint>
The project copy must remain byte-identical to the skeleton's capability file.
</constraint>

## TASK-FILES

| Operation | Path |
|---|---|
| modify | `prompts/capabilities/04-release-and-publishing.md` |
| modify | `prompts/tasks/00-tasks.md` |
| modify | `TODO.md` |

## TASK-VERIFY
- Run: `make test` from the repository root.
- Expected: exit status 0, with `09-prompt-contract` and `81-release-publish` reporting `ok`.
- Run: `sh scripts/skeleton-diff.sh ~/work/gnome-vcard-integration` from the skeleton.
- Expected: no drift is reported for `prompts/capabilities/04-release-and-publishing.md`.

OUTPUT: the complete files in the order listed, ending with the `VERIFICATION:` line.
