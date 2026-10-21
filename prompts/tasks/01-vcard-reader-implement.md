# Task: Implement the VCARD Reader feature

## TASK-DESCRIPTION
- Create: `sources/vcard/vcard_model.h`
- Create: `sources/vcard/vcard_reader.h`
- Create: `sources/vcard/vcard_reader.cpp`
- Create: `sources/util/file_io.h`
- Create: `sources/util/file_io.cpp`
- Create: `sources/version/version.h`
- Create: `sources/version/version_info.cpp`
- Create: `sources/CMakeLists.txt`
- Create: `tests/test_harness.h`
- Create: `tests/vcard_reader_test.cpp`
- Create: `tests/15-vcard-reader.sh`
- Create: `tests/lib/test_helpers.sh`
- Create: `dataflow.in/vcard/simple.vcf`
- Create: `dataflow.in/vcard/legacy.vcf`
- Create: `dataflow.in/vcard/multiple.vcf`

The reader implements the requirements in `prompts/features/03-vcard-reader.md`.
It must accept the three generations of the format: the 2.1 bare type
parameters and quoted-printable values, the 3.0 `TYPE=` form, and the 4.0 form
with its `PREF` parameter. It must unfold continuation lines, decode
quoted-printable and base64, unescape text, split the structured `N`, `ADR`, and
`ORG` values, and keep every property in file order. A card that is readable but
incomplete is returned with a warning; only a file that is not a vCard at all
fails.
The fixtures are synthetic: a real vCard belongs to a person, so every value in
them is invented.

## TASK-OUTPUT
Produce these complete files in this order, then the `VERIFICATION:` line:
1. `sources/vcard/vcard_model.h`
2. `sources/vcard/vcard_reader.h`
3. `sources/vcard/vcard_reader.cpp`
4. `sources/util/file_io.h`
5. `sources/util/file_io.cpp`
6. `sources/version/version.h`
7. `sources/version/version_info.cpp`
8. `sources/CMakeLists.txt`
9. `tests/test_harness.h`
10. `tests/vcard_reader_test.cpp`
11. `tests/15-vcard-reader.sh`
12. `tests/lib/test_helpers.sh`
13. `dataflow.in/vcard/simple.vcf`
14. `dataflow.in/vcard/legacy.vcf`
15. `dataflow.in/vcard/multiple.vcf`

## TASK-CONTEXT
<note>
`documents/10-vcard-format.md` records the format: the content-line grammar, the
folding rule, the two parameter styles, the escaped text, the encoded values,
and the structured names and addresses. The build treats warnings as errors.
</note>
<constraint>
The fixtures are synthetic. Do not commit a real vCard: it carries a person's
name, telephone numbers, address, and photograph, and this repository is
published.
</constraint>

## TASK-FILES

| Operation | Path |
|---|---|
| create | `sources/vcard/vcard_model.h` |
| create | `sources/vcard/vcard_reader.h` |
| create | `sources/vcard/vcard_reader.cpp` |
| create | `sources/util/file_io.h` |
| create | `sources/util/file_io.cpp` |
| create | `sources/version/version.h` |
| create | `sources/version/version_info.cpp` |
| create | `sources/CMakeLists.txt` |
| create | `tests/test_harness.h` |
| create | `tests/vcard_reader_test.cpp` |
| create | `tests/15-vcard-reader.sh` |
| create | `tests/lib/test_helpers.sh` |
| create | `dataflow.in/vcard/simple.vcf` |
| create | `dataflow.in/vcard/legacy.vcf` |
| create | `dataflow.in/vcard/multiple.vcf` |

## TASK-VERIFY
- Run: `make test` from the repository root.
- Expected: exit status 0, with `15-vcard-reader` reporting `ok`.

## TASK-FEATURES
- `prompts/features/03-vcard-reader.md`

## TASK-ACCEPTANCE
- `VCARD-READER-R001`
- `VCARD-READER-R002`
- `VCARD-READER-R004`
- `VCARD-READER-R006`
- `VCARD-READER-R007`
- `VCARD-READER-R008`
- `VCARD-READER-R010`
- `VCARD-READER-R011`
- `VCARD-READER-R019`
- `VCARD-READER-R023`

OUTPUT: the complete files in the order listed, ending with the `VERIFICATION:` line.
