# Task: Interpret a bare quoted-printable soft break

## TASK-DESCRIPTION
- Create: `dataflow.in/vcard/soft-break.vcf`
- Modify: `sources/vcard/vcard_reader.cpp`
- Modify: `sources/tools/vcard_inspect.cpp`
- Modify: `tests/vcard_reader_test.cpp`
- Modify: `tests/60-command-line-tools.sh`
- Modify: `prompts/features/03-vcard-reader.md`
- Modify: `documents/10-vcard-format.md`
- Modify: `documents/12-vcard-knowns-and-unknowns.md`
- Modify: `prompts/tasks/00-tasks.md`
- Modify: `TODO.md`

A real 2.1 card writes a quoted-printable soft break with no leading
whitespace on the continuation line:
`ADR;...;ENCODING=QUOTED-PRINTABLE:...=0D=0A=` followed by a line at column 0.
The reader currently treats the continuation as a malformed property, so the
address is truncated, the `LABEL` is dropped, and two warnings are raised.

The reader must join the next physical line when a quoted-printable property's
line ends with `=`, whether or not that line begins with whitespace.  A
non-quoted-printable line keeps a trailing `=` as text.  A five-component
`ADR` whose fifth part looks like a postal code is read as a postal code, so
the short form `;;street;city;00000` displays without an extra comma.
`vcard-inspect` reports the `LABEL` beside the address.

The fixture is synthetic: every value in it is invented, because a real card
belongs to a person and the repository is public.

## TASK-OUTPUT
Produce these complete files in this order, then the `VERIFICATION:` line:
1. `dataflow.in/vcard/soft-break.vcf`
2. `sources/vcard/vcard_reader.cpp`
3. `sources/tools/vcard_inspect.cpp`
4. `tests/vcard_reader_test.cpp`
5. `tests/60-command-line-tools.sh`
6. `prompts/features/03-vcard-reader.md`
7. `documents/10-vcard-format.md`
8. `documents/12-vcard-knowns-and-unknowns.md`
9. `prompts/tasks/00-tasks.md`
10. `TODO.md`

## TASK-CONTEXT
<note>
The format research is in `documents/10-vcard-format.md`; the decisions and
their reasons are in `documents/12-vcard-knowns-and-unknowns.md`.  The
supplied card that exposed the defect is not committed: the fixture mirrors
its line structure with invented values.
</note>
<constraint>
Do not commit the supplied card, any value from it, or a fixture derived from
its personal data.
</constraint>

## TASK-FILES

| Operation | Path |
|---|---|
| create | `dataflow.in/vcard/soft-break.vcf` |
| modify | `sources/vcard/vcard_reader.cpp` |
| modify | `sources/tools/vcard_inspect.cpp` |
| modify | `tests/vcard_reader_test.cpp` |
| modify | `tests/60-command-line-tools.sh` |
| modify | `prompts/features/03-vcard-reader.md` |
| modify | `documents/10-vcard-format.md` |
| modify | `documents/12-vcard-knowns-and-unknowns.md` |
| modify | `prompts/tasks/00-tasks.md` |
| modify | `TODO.md` |

## TASK-VERIFY
- Run: `make test` from the repository root.
- Expected: exit status 0, with `15-vcard-reader` and `60-command-line-tools` reporting `ok`.
- Run: `vcard-inspect dataflow.in/vcard/soft-break.vcf`.
- Expected: the address and address label contain `Exampleville`, and no `malformed line` warning appears.

## TASK-FEATURES
- `prompts/features/03-vcard-reader.md`

## TASK-ACCEPTANCE
- `VCARD-READER-R004`
- `VCARD-READER-R014`
- `VCARD-READER-R026`
- `VCARD-READER-R027`

OUTPUT: the complete files in the order listed, ending with the `VERIFICATION:` line.
