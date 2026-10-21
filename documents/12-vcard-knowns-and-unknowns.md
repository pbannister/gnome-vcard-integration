# vCard: Knowns, Unknowns, and Decisions

This document separates what the specifications say, what this project observed,
what is only conventional, and what had to be decided where the format is
silent.

## Documented

- The content-line grammar and the fold rule.
- `N` has five components and `ADR` has seven, in a fixed order.
- Backslash escapes `\n`, `\,`, `\;`, and `\\`.
- `ENCODING=QUOTED-PRINTABLE` and `ENCODING=BASE64` (spelled `b` in 3.0 and 4.0)
  are the two ways a binary or non-ASCII value travels in a text file.
- `FN` is the formatted name and is required in 3.0 and 4.0.
- `VERSION` is required, and a card without it is not conformant.
- `PREF` is a parameter in 4.0 and a `TYPE` value in 2.1 and 3.0.

## Observed

- Producers disagree about line endings; CRLF, LF, and bare CR all occur.
- A UTF-8 byte-order mark is common at the start of an exported file.
- A 2.1 producer writes `TEL;HOME;VOICE:` rather than `TYPE=`.
- A quoted parameter value can contain `;`, `,`, and `:`.
- A long note is folded, and a quoted-printable value often folds with a
  trailing `=`.
- Apple writes `item1.EMAIL` and `item1.X-ABLabel` to group a property with a
  custom label, and the group is meaningful only to Apple.
- A photo is sometimes a `data:` URI and sometimes an inline base64 value with
  no `MEDIATYPE`; the bytes themselves identify the image.
- A file from a phone may carry one card; a file from an address book may carry
  hundreds.

## Conventional

- `.vcf` is the usual extension, and `.vcard` is a rarer one.
- The type is `text/vcard`; `text/x-vcard` and `text/directory` are legacy
  aliases that `shared-mime-info` maps to it.
- The first `FN` is the name a person expects to see.
- A `PREF=1` value is the one to show first.
- An `ADR` with every component empty is an absent address, not an address of
  empty parts.

## Unknown or unspecified

- What a specific producer means by an `X-` property.
- How a 2.1 producer encodes a line that is both folded and quoted-printable;
  the two mechanisms overlap and the specification does not settle the order.
- Whether a `CHARSET` label is accurate. A file labelled `ISO-8859-1` may really
  be Windows-1252.
- Whether a group prefix is stable across exports.
- How a card with two `FN` values should be displayed; the specification forbids
  it, and producers still emit it.

## Decisions this project made

| Question | Decision | Why |
| -------- | -------- | --- |
| A card with no `FN` | return it with a warning, and derive a display name from `N` | a viewer can still show the person; refusing hides a readable card |
| A card with no `N` | return it with a warning | 4.0 requires only `FN` |
| An unknown `VERSION` | read it as 4.0-shaped and warn | the line grammar is shared; the alternative is refusing a readable file |
| A malformed line | skip it and warn | one bad line must not hide the rest of the card |
| A charset outside UTF-8, US-ASCII, and ISO-8859-1 | keep the octets as written and warn | a wrong transcoding is worse than a visible replacement character |
| A remote `PHOTO` URI | report it, do not fetch it | reading a card must not contact a third party |
| An inline photo with no `MEDIATYPE` | sniff the PNG and JPEG signatures, otherwise keep the bytes and warn | the bytes are the reliable statement |
| A `javascript:` URL | report it, never link it | the card is untrusted input and the SVG is a document |
| Two `FN` values | keep both, show the first, warn | the file is wrong; the reader still has a useful value |
| Text outside a card | warn and ignore it | a concatenated export may carry a stray line |
| A NUL byte in the file | refuse the file as not a vCard | a text format with a NUL is binary damage, not a card |
| A quoted-printable soft break with no leading whitespace | join the next line | some producers write it; the alternative is to lose the rest of the value |
| A five-component `ADR` whose fifth part looks like a postal code | read the fifth part as the postal code | some 2.1 producers omit the trailing empty components |

## Security notes

- A vCard is untrusted input. Every string that reaches the SVG is XML-escaped.
- Only `mailto:`, `tel:`, `http:`, and `https:` become links.
- A `data:` URI is embedded as an image only when its media type is an image
  and its bytes are present; it is never treated as a link.
- The reader holds the file in memory and does not follow a reference.
- The integration writes only under the user's XDG directories.
