# API reference

The controller's HTTP API is specified by ICT's *Protege WX DLL API Reference*.
It is published as a MadCap Flare HTML5 help site rather than a document, so
there is no PDF to download:

<https://my.ict.co/wxdllapidocs/Topics/Software%20Manuals/Appliance-Client%20Application/AC%20API%20Spec/Home%20Page.htm>

Version used while writing this client: **published 31 August 2026**, 73 topics.

`tools/fetch_api_docs.py` converts a local mirror of that site into markdown
and a single PDF under `docs/vendor/`, which is gitignored — the output is
derived data, and the source is the vendor's to distribute.

## Regenerating

Requires `pandoc`, `weasyprint` and `beautifulsoup4`:

```sh
sudo pacman -S --needed pandoc-cli python-weasyprint python-beautifulsoup4
```

**1. Mirror the site.** Topic pages are served without authentication, so no
cookie is needed. `robots.txt` permits all agents.

```sh
wget --recursive --level=inf --no-parent --page-requisites --convert-links \
     --domains=my.ict.co --include-directories=/wxdllapidocs \
     --reject-regex='login\.php' --restrict-file-names=windows \
     --wait=1 --random-wait \
     -P ~/Downloads/wxdllapidocs \
     'https://my.ict.co/wxdllapidocs/Topics/Software%20Manuals/Appliance-Client%20Application/AC%20API%20Spec/Home%20Page.htm'
```

`--include-directories` is what keeps wget inside the doc site; without it, it
follows the portal header into the rest of `my.ict.co`.

**2. Convert.** The mirror path is optional and defaults to the location above.

```sh
python3 tools/fetch_api_docs.py [path/to/mirror/my.ict.co/wxdllapidocs]
```

| Output | Purpose |
| --- | --- |
| `docs/vendor/md/*.md` | one file per topic — the useful form for grep and Find in Files |
| `docs/vendor/wxdllapidocs.md` | the same content concatenated, headings nested by TOC depth |
| `docs/vendor/wxdllapidocs.pdf` | 324 pages, bookmarked, with the site's cross-references intact |

## Topics relevant to this client

| Topic | Covers |
| --- | --- |
| `overview-authentication-server.md` | the flow implemented in `ControllerAPI.cpp` |
| `overview-authentication-client.md` | the pre-4.00.1676 flow, not yet implemented |
| `overview-authentication-migration.md` | differences between the two |
| `overview-sessions.md` | `InitSession`, keep-alive, `CloseSession` |
| `controller-settings.md` | `GXT_CONTROLLERSETTINGS_TBL`, the first planned data query |
| `table-names.md` | the `SubType` values for CRUD operations |

## Conversion notes

Worth knowing if the output ever looks wrong:

- **The site has no search index offline.** Flare keeps it in `Data/`, which
  the vendor serves only to logged-in users. Grep `docs/vendor/md/` instead.
- **The table of contents is reconstructed.** `Data/Tocs/*.js` is behind the
  same login, but every topic ships its own branch of the sidenav
  pre-expanded, so merging all 73 sidenavs recovers the full ordered tree.
  A topic the nav never reaches is appended at the end rather than dropped.
- **A few tables appear as raw HTML in the markdown.** GitHub-flavored
  markdown cannot express `colspan`, so pandoc falls back to HTML for those.
  They render correctly in the PDF.
- **The spec contains no images.** Its only `<img>` is a 1×1 layout spacer,
  which the converter strips. `docs/vendor/images/` is created only if a future
  revision adds real ones.
