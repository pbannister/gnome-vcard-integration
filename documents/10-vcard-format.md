# The vCard Format

This document records what this project needed to know about vCard before it
wrote the reader.
It is a summary, not a substitute for the specifications it cites.

## Sources

- RFC 2425 — A MIME Content-Type for Directory Information (the content-line grammar).
- RFC 2426 — vCard MIME Directory Profile (vCard 3.0).
- RFC 6350 — vCard Format Specification (vCard 4.0).
- RFC 6868 — Parameter Value Encoding in iCalendar and vCard (`^` escapes).
- The vCard 2.1 specification, as published with Versit and still found in the wild.
- The `shared-mime-info` definition of `text/vcard` on a current Linux desktop.

## What a vCard is

A vCard is a text document, normally named with `.vcf`.
It carries one or more contacts.
Each contact is a card delimited by `BEGIN:VCARD` and `END:VCARD`.
The lines between them are content lines: a property name, optional parameters,
and a value.

The three generations differ in how they spell a parameter and how they encode
a value, not in the shape of the document.

| Generation | Typical marker | Parameter style | Notes |
| ---------- | -------------- | --------------- | ----- |
| 2.1 | `VERSION:2.1` | `TEL;HOME;VOICE:...` | bare types, quoted-printable common, `CHARSET=` common |
| 3.0 | `VERSION:3.0` | `TEL;TYPE=HOME,VOICE:...` | `ENCODING=b` for base64, `VALUE=` for typed values |
| 4.0 | `VERSION:4.0` | `TEL;TYPE="voice,home";PREF=1:...` | `PREF` is its own parameter, `FN` is the only required name property |

A producer may write any of them.
A reader that accepts only 4.0 loses most address books exported from older
systems.

## The content line

The grammar, after unfolding, is:

```
[group "."] name *(";" parameter) ":" value
```

- `group` is an optional label. Apple uses `item1`, `item2`, and so on to tie a
  property to a grouped photo or label. The reader keeps it and does not
  interpret it.
- `name` is a token such as `FN`, `TEL`, `ADR`, or `X-SOMETHING`. Names are
  case-insensitive in practice; the model stores them uppercase.
- A `parameter` is `name "=" value *("," value)` in 3.0 and 4.0.
  A value may be quoted, and then may contain `:`, `;`, and `,`.
  In 2.1 a bare token is a type: `TEL;HOME;VOICE:` means the same as
  `TEL;TYPE=HOME,VOICE:`.
- `value` runs to the end of the line. The first unquoted `:` separates the
  parameters from the value; a `:` inside a quoted parameter value does not.

## Folding

A content line may be folded across physical lines.
A fold is a line break followed by exactly one space or tab.
Unfolding removes the line break and that one whitespace character.
Files use CRLF, bare LF, or (rarely) bare CR.

A quoted-printable value adds a second folding-like rule: a `=` at the end of a
physical line is a soft line break, and the value continues on the next line.
After the ordinary unfolding, that soft break appears as a `=` followed by the
next line's text, so the reader drops a trailing `=` when it joins a folded
continuation.
This is the difference between `NOTE:Hello=20world` and a long note that merely
wrapped.

Some producers, Outlook among them, write the soft break with no leading
whitespace on the continuation line:

```
ADR;WORK;ENCODING=QUOTED-PRINTABLE:;;2023 Example Street=0D=0A=
Exampleville;CO;00000
```

That is not a valid vCard fold, but it is common in exported address books.
The reader joins it when a quoted-printable line ends with `=`, because the
alternative is to drop the rest of the value and report a malformed line.

## Parameters this project reads

| Parameter | Meaning | Handling |
| --------- | ------- | -------- |
| `TYPE` | the kind of value (`HOME`, `WORK`, `CELL`, `VOICE`, `INTERNET`, `PREF`) | kept as a list; `PREF` is also a preference |
| `PREF` | 4.0 preference, 1 is most preferred | kept and used to order a section |
| `ENCODING` | `QUOTED-PRINTABLE`, `BASE64`, `b`, `8BIT` | decoded; other values are kept and warned about |
| `CHARSET` | the octet character set | UTF-8, US-ASCII, and ISO-8859-1 are transcoded |
| `VALUE` | the value's type (`uri`, `text`, `date`, `date-time`, `binary`) | recorded; used to decide whether a value is a URI |
| `MEDIATYPE` | the media type of a binary value | recorded and used for the photo data URI |
| `LANGUAGE` | the value's language | recorded |
| `LABEL` | a formatted postal address | read as an address companion |

Everything else is preserved on the property and shown by `vcard-inspect
--properties`.

## Escaping

Inside a text value the backslash escapes these sequences:

| Sequence | Means |
| -------- | ----- |
| `\n` or `\N` | newline |
| `\,` | comma |
| `\;` | semicolon |
| `\\` | backslash |

A comma in a text value need not be escaped, but a comma in a structured value
separates components, so the distinction matters when the value is split.
A backslash before any other character is not defined by the specification; this
project keeps both characters as written rather than inventing a meaning.

## Structured values

A `LABEL` property often carries the formatted address that the producer
intended; this project reports it alongside the `ADR` rather than replacing
the structured address with it.

Some 2.1 producers drop the trailing empty `ADR` components and leave the
postal code in the region position, as in `;;Street;City;00000`.
A five-component address whose fifth part looks like a postal code is read
that way, so the display does not gain a comma between the city and the code.

`N` has five components, in this order:

```
family;given;additional;prefix;suffix
```

`ADR` has seven:

```
post-office-box;extended-address;street;locality;region;postal-code;country
```

`ORG` is a name followed by zero or more organisational units, separated by
semicolons.
Each component may itself be a comma-separated list, though this project keeps
the first value of a list for display and preserves the whole component in the
model.

## Encoded values

- **Quoted-printable** expands `=XX` to the octet `0xXX`. A trailing `=` is a
  soft break. The result is octets in the declared `CHARSET`.
- **Base64** carries binary values such as `PHOTO`, `LOGO`, `SOUND`, and `KEY`.
  In 3.0 and 4.0 the parameter is usually `ENCODING=b`; in 2.1 it is
  `ENCODING=BASE64`.
- **A `data:` URI** may carry the same bytes in a text value, as in
  `PHOTO;VALUE=uri:data:image/png;base64,...`.
- **A remote URI** such as `PHOTO;VALUE=uri:https://example.invalid/photo.png`
  is a reference, not data. This project records it and never fetches it.

ISO-8859-1 is a byte-for-byte transcoding to the first 256 Unicode code points.
Windows-1252 differs in the range `0x80`–`0x9F`; this project does not yet
transcode that range and records the limitation in `TODO.md`.

## The properties this project understands

| Property | What it carries |
| -------- | --------------- |
| `BEGIN`, `END`, `VERSION` | the card structure |
| `FN` | the formatted display name |
| `N` | the structured name |
| `NICKNAME` | one or more nicknames |
| `PHOTO`, `LOGO` | an image, inline or by URI |
| `BDAY`, `ANNIVERSARY` | a date, kept as written |
| `ADR`, `LABEL` | a postal address and an optional formatted label |
| `TEL` | a telephone number |
| `EMAIL` | an email address |
| `IMPP` | an instant-messaging address |
| `URL` | a web address |
| `ORG`, `TITLE`, `ROLE` | the organisation, job title, and role |
| `NOTE` | free text |
| `TZ`, `GEO`, `LANG` | time zone, coordinates, and language |
| `CATEGORIES`, `UID`, `REV`, `PRODID` | classification and bookkeeping |
| `KIND`, `GENDER` | 4.0 identity properties |
| `X-*` | vendor extensions, kept and reported |

The renderer draws the name, organisation, title, telephones, emails,
addresses, URLs, note, and photo.
Everything else is parsed, reported, and available in the model; it is simply
not drawn yet.

## A synthetic example

This card exercises the forms the reader must accept.
Every value is invented.

```
BEGIN:VCARD
VERSION:4.0
FN:Ada Lovelace
N:Lovelace;Ada;Augusta;Countess;
ORG:Analytical Engine Society;Mathematics
TITLE:Mathematician
TEL;TYPE="voice,work";PREF=1:+44 20 0000 0000
EMAIL;TYPE=work:ada@example.invalid
ADR;TYPE=work:;;12 Example Street;London;Greater London;SW1A 1AA;United Kingdom
URL:https://example.invalid/ada
NOTE:Wrote the first published algorithm.\nSecond line.
END:VCARD
```

## What this project does not do

- It does not write or modify a vCard.
- It does not fetch a remote photo, logo, sound, or key.
- It does not validate a signature or a certificate; vCard has neither.
- It does not resolve a `MEMBER` group or a `KIND:group` relationship.
- It does not apply a `LABEL` as a replacement for `ADR`; the address is
  formatted from `ADR` and the label is reported separately.
- It does not localise a `LANGUAGE` value; it shows the value as written.
