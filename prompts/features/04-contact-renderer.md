# Feature: Contact Renderer

## Purpose

The project must draw a contact so a person can see it without a contacts
application: one SVG document that stands alone, carrying the name, the
organisation, the ways to reach the person, and an embedded photograph.

## Requirements

- `CONTACT-RENDERER-R001` — The renderer must produce one self-contained SVG document for a contact.
- `CONTACT-RENDERER-R002` — The renderer must draw the display name, the organisation, and the title.
- `CONTACT-RENDERER-R003` — The renderer must draw the telephones, emails, postal addresses, URLs, and note in labelled sections, and must omit a section with no values.
- `CONTACT-RENDERER-R004` — The renderer must embed an inline photo as a base64 data URI, and must draw a placeholder that names the reason when the photo is a remote URI or an unreadable media type.
- `CONTACT-RENDERER-R005` — The renderer must make a `mailto:`, `tel:`, `http:`, or `https:` value clickable, and must not make any other scheme clickable.
- `CONTACT-RENDERER-R006` — The renderer must escape the XML metacharacters in every string that came from the card.
- `CONTACT-RENDERER-R007` — The renderer must draw a newline in a value as a line break.
- `CONTACT-RENDERER-R008` — The renderer must wrap long text instead of running it off the card.
- `CONTACT-RENDERER-R009` — The renderer must draw the phone number as a `tel:` target without spaces or punctuation, keeping the displayed number as written.
- `CONTACT-RENDERER-R010` — The renderer must omit the photo when it is asked to, and must still draw the rest of the card.
- `CONTACT-RENDERER-R011` — The renderer must select a card by a one-based index, must draw the first card when no index is given, and must fail with a message when the index is outside the file.
- `CONTACT-RENDERER-R012` — The renderer must produce the same bytes for the same card and options.
- `CONTACT-RENDERER-R013` — The renderer must format a postal address from the `ADR` components in their documented order, skipping empty components and keeping the country last.
- `CONTACT-RENDERER-R014` — No public interface may throw.
- `CONTACT-RENDERER-R015` — The renderer must keep a UTF-8 name and note as UTF-8 in the output.

## Behavior

- Given a card with a purple organisation and two telephones, the card names both numbers and links each with `tel:`.
- Given a note containing `<script>`, the SVG contains no `<script>` element.
- Given a note containing `https://example.invalid/ada`, the SVG carries an anchor to that URL and no anchor to a `javascript:` URL.
- Given a photo of a PNG, the SVG carries a `data:image/png;base64,` URI.
- Given a photo written as `http://example.invalid/photo.png`, the card draws no remote image and says the photo was not embedded.
- Given a card with an empty `ADR` home address and a filled work address, only the work address is drawn.
- Given `vcard-render --index 2` on a two-card file, the second contact is drawn.
- Given `vcard-render --index 3` on a two-card file, the command fails with a message.

## Dependencies

- `03-vcard-reader.md`
