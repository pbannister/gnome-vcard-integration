# Task: Guard the vCard fixtures against real data

## TASK-DESCRIPTION
- Create: `tests/04-fixture-privacy.sh`
- Create: `records/01-privacy-leak-and-purge.md`
- Modify: `README.md`
- Modify: `prompts/tasks/00-tasks.md`
- Modify: `TODO.md`

The privacy incident that put a real vCard into the tree showed that the
boundary in prose was not a gate.  Add a fixture allowlist test: only the
reviewed synthetic `.vcf` files may live under `dataflow.in/vcard/`, and a new
file fails the suite until a reviewer adds it to the allowlist.

Add the incident record required by the records rules: what happened, the root
cause, the fix and its verification, the lesson, the safeguards, and the
residual GitHub object the owner accepted.  The record must not repeat the
card's personal data or its filename.

The README's test section names the new test, and `TODO.md` records the
completed guard with a pointer to the record.

## TASK-OUTPUT
Produce these complete files in this order, then the `VERIFICATION:` line:
1. `tests/04-fixture-privacy.sh`
2. `records/01-privacy-leak-and-purge.md`
3. `README.md`
4. `prompts/tasks/00-tasks.md`
5. `TODO.md`

## TASK-CONTEXT
<note>
The record is an incident record and follows `records/README.md`.  The
allowlist is a name check; the header states that content review remains a
human step and that an allowlisted filename can still be overwritten.
</note>
<constraint>
Do not quote the real card's name, telephone numbers, address, email address,
photo URL, or filename in the record or the test.
</constraint>

## TASK-FILES

| Operation | Path |
|---|---|
| create | `tests/04-fixture-privacy.sh` |
| create | `records/01-privacy-leak-and-purge.md` |
| modify | `README.md` |
| modify | `prompts/tasks/00-tasks.md` |
| modify | `TODO.md` |

## TASK-VERIFY
- Run: `make test` from the repository root.
- Expected: exit status 0, with `04-fixture-privacy` reporting `ok`.

OUTPUT: the complete files in the order listed, ending with the `VERIFICATION:` line.
