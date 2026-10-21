# Feature: VCARD Reader

## Purpose

The project must read a vCard file: a text document that carries one or more
contacts, written in version 2.1, 3.0, or 4.0, with folded lines, parameters, and
encoded values.

## Requirements

- `VCARD-READER-R001` — The reader must read a file as text with CRLF, LF, or CR line endings, and must strip a leading UTF-8 byte-order mark.
- `VCARD-READER-R002` — The reader must split the file into cards on `BEGIN:VCARD` and `END:VCARD`, keeping every card in file order.
- `VCARD-READER-R003` — The reader must fail with a message when the file contains no card, or when a `BEGIN` is not closed by an `END`.
- `VCARD-READER-R004` — The reader must unfold a continuation line that begins with a space or a tab, and must drop a trailing `=` that precedes a folded continuation, which is how a quoted-printable soft line break appears after unfolding.
- `VCARD-READER-R005` — The reader must parse a content line as `[group.]name[;parameter...]:value`, with names matched case-insensitively.
- `VCARD-READER-R006` — The reader must accept both the vCard 2.1 bare parameter form (`TEL;HOME;VOICE:...`) and the `TYPE=` form, including quoted parameter values and comma-separated value lists.
- `VCARD-READER-R007` — The reader must unescape a text value: `\n` or `\N` becomes a newline, `\,` a comma, `\;` a semicolon, and `\\` a backslash; any other backslash sequence is kept as written.
- `VCARD-READER-R008` — The reader must decode a `QUOTED-PRINTABLE` value, including `=XX` octets and a soft line break, and a `BASE64` value into bytes.
- `VCARD-READER-R009` — The reader must transcode `UTF-8`, `US-ASCII`, and `ISO-8859-1` values to UTF-8, and must warn, without failing, when a value declares a charset it does not know.
- `VCARD-READER-R010` — The reader must split a structured value on unescaped semicolons and commas, and must expose the components of `N`, `ADR`, and `ORG` separately.
- `VCARD-READER-R011` — The reader must read `VERSION`, must warn and assume `3.0` when it is absent, and must warn, without failing, on a version it does not recognise.
- `VCARD-READER-R012` — The reader must read `FN`, `N`, and `NICKNAME`, and must derive a display name from `N` when `FN` is absent.
- `VCARD-READER-R013` — The reader must read `TEL`, `EMAIL`, `URL`, and `IMPP` values together with their group, their `TYPE` values, their `PREF` marker, and their language.
- `VCARD-READER-R014` — The reader must read `ADR` and `LABEL`, keeping every address component in its `N`-style position.
- `VCARD-READER-R015` — The reader must read `ORG`, `TITLE`, `ROLE`, and `NOTE`.
- `VCARD-READER-R016` — The reader must read `BDAY`, `ANNIVERSARY`, `REV`, and `TZ` as text values, and must not reformat a date.
- `VCARD-READER-R017` — The reader must read `PHOTO` and `LOGO` either as inline base64 bytes or as a `data:` URI, must record the media type and the byte count, and must never fetch a remote URI.
- `VCARD-READER-R018` — The reader must keep every property, including `X-` properties, in file order, with its group, its parameters, and its decoded value.
- `VCARD-READER-R019` — The reader must return a readable card together with its warnings rather than refusing it, so a viewer can still show an incomplete card.
- `VCARD-READER-R020` — The reader must not modify the file it reads.
- `VCARD-READER-R021` — No public interface may throw.
- `VCARD-READER-R022` — The reader must reject a file that contains a NUL byte as not a vCard, and must report the reason.
- `VCARD-READER-R023` — The reader must expose the contact facts a renderer needs as accessors: display name, structured name, organisation, title, telephones, emails, addresses, URLs, note, and photo.
- `VCARD-READER-R024` — The reader must recognise the vCard 4.0 `PREF` parameter as well as the `TYPE=PREF` form, and must treat a `PREF` value as an ordering preference.
- `VCARD-READER-R025` — The reader must record a `VALUE=` parameter and a `MEDIATYPE=` parameter on the property that carries it.
- `VCARD-READER-R026` — The reader must join a quoted-printable value's next physical line when the previous line ends with `=`, even when the continuation does not begin with whitespace, because some producers write that form.
- `VCARD-READER-R027` — The reader must read a five-component `ADR` whose fifth component looks like a postal code as a postal code, because some 2.1 producers omit the trailing empty components.

## Behavior

- Given a file whose lines end in LF, every property is read exactly as with CRLF.
- Given `NOTE;ENCODING=QUOTED-PRINTABLE:Hello=20world`, the note is `Hello world`.
- Given `TEL;HOME;VOICE:+44 20 0000 0000`, the telephone has types `HOME` and `VOICE`.
- Given `TEL;TYPE="voice,home";PREF=1:+44 20 0000 0000`, both types are read and the property is preferred.
- Given `N:Lovelace;Ada;Augusta;Countess;` on a 2.1 card, `FN` is absent and the display name is `Countess Ada Augusta Lovelace`.
- Given `PHOTO;ENCODING=b;TYPE=PNG:iVBORw0KGgo=`, the reader records PNG bytes and never fetches anything.
- Given a card with no `FN`, the card is returned with a warning that names the missing property.
- Given a file with two cards, both are returned in order.
- Given a file that is not a vCard, the reader fails with a message and no partial card.
- Given `NOTE;ENCODING=QUOTED-PRINTABLE:first=0D=0A=` followed by a line at column 0, the note contains the continuation and no malformed-line warning is raised.
- Given `ADR:;;Street;City;00000`, the address has locality `City` and postal code `00000`, and no region.

## Dependencies

- None.
