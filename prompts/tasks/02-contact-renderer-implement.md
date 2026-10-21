# Task: Implement the Contact Renderer feature

## TASK-DESCRIPTION
- Create: `sources/render/contact_render.h`
- Create: `sources/render/contact_render.cpp`
- Create: `tests/contact_render_test.cpp`
- Create: `tests/40-contact-render.sh`
- Modify: `sources/CMakeLists.txt`

The renderer implements the requirements in
`prompts/features/04-contact-renderer.md`.
It draws one self-contained SVG card: the name, organisation, title, the reach
sections, the postal addresses, the note, and an embedded photo. Every string
from the card is escaped, and only `mailto:`, `tel:`, `http:`, and `https:` are
made clickable, so a card cannot put a `javascript:` link into the file.

## TASK-OUTPUT
Produce these complete files in this order, then the `VERIFICATION:` line:
1. `sources/render/contact_render.h`
2. `sources/render/contact_render.cpp`
3. `tests/contact_render_test.cpp`
4. `tests/40-contact-render.sh`
5. `sources/CMakeLists.txt`

## TASK-CONTEXT
<note>
The renderer reads the model from `sources/vcard/vcard_model.h` and does not
reopen the file, so the card it draws is the card the reader returned.
`documents/10-vcard-format.md` records the value forms it must expect.
</note>
<constraint>
The SVG is XML written from untrusted input. Escape every card-supplied string,
and never build an `href` from a scheme other than `mailto:`, `tel:`, `http:`,
or `https:`.
</constraint>

## TASK-FILES

| Operation | Path |
|---|---|
| create | `sources/render/contact_render.h` |
| create | `sources/render/contact_render.cpp` |
| create | `tests/contact_render_test.cpp` |
| create | `tests/40-contact-render.sh` |
| modify | `sources/CMakeLists.txt` |

## TASK-VERIFY
- Run: `make test` from the repository root.
- Expected: exit status 0, with `40-contact-render` reporting `ok`.

## TASK-FEATURES
- `prompts/features/04-contact-renderer.md`

## TASK-ACCEPTANCE
- `CONTACT-RENDERER-R001`
- `CONTACT-RENDERER-R002`
- `CONTACT-RENDERER-R003`
- `CONTACT-RENDERER-R004`
- `CONTACT-RENDERER-R005`
- `CONTACT-RENDERER-R006`
- `CONTACT-RENDERER-R008`
- `CONTACT-RENDERER-R013`

OUTPUT: the complete files in the order listed, ending with the `VERIFICATION:` line.
