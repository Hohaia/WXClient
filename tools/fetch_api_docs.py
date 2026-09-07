#!/usr/bin/env python3
"""Convert a local mirror of the Protege WX DLL API help site into markdown
and a single PDF under docs/vendor/.

The vendor publishes the spec as a MadCap Flare HTML5 site, not a document.
Mirror it first (see docs/api-reference.md), then run this script. Outputs:

    docs/vendor/md/*.md          one file per topic, for grep and Find in Files
    docs/vendor/wxdllapidocs.md  the same content concatenated
    docs/vendor/wxdllapidocs.pdf single PDF with bookmarks and working xrefs

All three come from the same cleaned DOM, so they cannot drift. docs/vendor/
is gitignored: the output is derived, and the source is the vendor's.

Requires pandoc, weasyprint and beautifulsoup4:

    sudo pacman -S --needed pandoc-cli python-weasyprint python-beautifulsoup4
"""
import argparse
import re
import shutil
import subprocess
import sys
import urllib.parse
from pathlib import Path

from bs4 import BeautifulSoup, NavigableString

REPO_ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = REPO_ROOT / "docs" / "vendor"
MD_DIR = OUT_DIR / "md"
IMG_DIR = OUT_DIR / "images"
PRINT_CSS = REPO_ROOT / "tools" / "api_docs_print.css"

DEFAULT_MIRROR = Path.home() / "Downloads/wxdllapidocs/my.ict.co/wxdllapidocs"

IMG_TOKEN = "__IMGDIR__"
LINK_TOKEN = "__LINK__"

TITLE = "Protege WX DLL API Reference"
SOURCE_URL = (
    "https://my.ict.co/wxdllapidocs/Topics/Software%20Manuals/"
    "Appliance-Client%20Application/AC%20API%20Spec/Home%20Page.htm"
)

# Chrome the Flare skin injects into every topic.
DROP_SELECTORS = ("div.toolbarcontainer", "a.backtotoplink", "a.codeSnippetCopyButton")


def load(path):
    return BeautifulSoup(path.read_text(encoding="utf-8", errors="replace"), "html.parser")


def norm(href):
    """Normalise an href to a bare topic filename, or None."""
    if not href:
        return None
    href = href.split("#")[0]
    if not href:
        return None
    return urllib.parse.unquote(href).split("/")[-1]


def slugify(text):
    text = re.sub(r"[^\w\s-]", "", text.lower())
    return re.sub(r"[\s_]+", "-", text).strip("-")


def find_topics_dir(mirror):
    """Locate the topic directory by its landing page, so a reorganised
    mirror does not require editing a hardcoded path."""
    for landing in sorted(mirror.rglob("Home Page.htm")):
        if len(list(landing.parent.glob("*.htm"))) > 5:
            return landing.parent
    return None


# --------------------------------------------------------------------------
# TOC reconstruction
#
# Flare keeps the full tree in Data/Tocs/*.js, which the vendor serves only to
# logged-in users. Each topic does ship its own branch of the sidenav
# pre-expanded, so merging every sidenav recovers the complete ordered tree.
# --------------------------------------------------------------------------
def parse_ul(ul):
    items = []
    for li in ul.find_all("li", recursive=False):
        a = li.find("a", recursive=False) or li.find("a")
        sub = li.find("ul", recursive=False)
        items.append(
            {
                "href": a.get("href") if a else None,
                "children": parse_ul(sub) if sub else [],
            }
        )
    return items


def collect_toc(files):
    top_order, children = None, {}

    def record(nodes):
        for node in nodes:
            fn = norm(node["href"])
            kids = [norm(c["href"]) for c in node["children"]]
            kids = [k for k in kids if k and k != fn and k in files]
            # Keep the richest version seen; a page expands only its own branch.
            if kids and len(kids) > len(children.get(fn, [])):
                children[fn] = kids
            record(node["children"])

    for path in sorted(files.values()):
        ul = load(path).select_one("ul.sidenav")
        if not ul:
            continue
        items = parse_ul(ul)
        if top_order is None:
            top_order = [norm(i["href"]) for i in items if norm(i["href"]) in files]
        record(items)

    return top_order or [], children


def flatten(top_order, children):
    out, seen = [], set()

    def walk(fn, depth):
        if fn in seen:
            return
        seen.add(fn)
        out.append((fn, depth))
        for child in children.get(fn, []):
            walk(child, depth + 1)

    for fn in top_order:
        walk(fn, 0)
    return out


# --------------------------------------------------------------------------
# Cleaning
# --------------------------------------------------------------------------
def clean_topic(path, files, images):
    soup = load(path)
    body = soup.select_one("div.body-container")
    if body is None:
        return None, None

    for selector in DROP_SELECTORS:
        for el in body.select(selector):
            el.decompose()

    # Flare's namespaced tags hold real content (e.g. OEM part numbers).
    for el in list(body.find_all(re.compile(r"^madcap:", re.I))):
        el.unwrap()

    # Variable spans would otherwise leak raw HTML into markdown headings.
    for el in list(body.select("span.mc-variable")):
        el.unwrap()

    # Flare pads layout with a 1x1 spacer; invisible, but noisy in markdown.
    for el in list(body.find_all("img", src=True)):
        if el["src"].lower().endswith("transparent.gif"):
            el.decompose()

    # Let the renderer size columns rather than honour authored widths.
    for el in list(body.find_all(["col", "colgroup"])):
        el.decompose()

    # Toggler heads are links with no real target. Keep the text and drop the
    # link, before link rewriting turns it into a self-referential href.
    for el in list(body.select("a.MCToggler")):
        el.unwrap()

    # Flare ships toggler bodies collapsed and reveals them with JavaScript.
    # Left alone, the PDF renderer honours display:none and silently drops the
    # content: an earlier build lost the code samples from four topics while
    # still printing their headings.
    for el in list(body.find_all(style=re.compile(r"display\s*:\s*none", re.I))):
        del el["style"]

    # Flare records the snippet language in a proprietary style property, but
    # only on a handful of snippets (4 of 573 in the Aug 2026 revision) — among
    # them the C# reference implementation. Carry it through so those get a
    # tagged fence. Must run before the wrappers below are unwrapped.
    for el in body.select("div.codeSnippetBody[style]"):
        match = re.search(r"mc-code-lang:\s*([A-Za-z0-9+#_-]+)", el.get("style", ""))
        if not match:
            continue
        # pandoc takes code-block attributes from <pre>, not the inner <code>.
        for pre in el.select("pre"):
            pre["class"] = [match.group(1).lower()]

    # Attribute-carrying wrappers force pandoc to emit raw HTML instead of
    # markdown; the inner <pre><code> is all that carries meaning.
    for el in list(body.select("div#mc-main-content, div.codeSnippet, div.codeSnippetBody")):
        el.unwrap()

    for img in body.find_all("img", src=True):
        resolved = (path.parent / urllib.parse.unquote(img["src"])).resolve()
        if not resolved.exists():
            print(f"  ! missing image, left as-is: {img['src']} in {path.name}")
            continue
        flat = images.setdefault(resolved, resolved.name)
        img["src"] = f"{IMG_TOKEN}/{flat}"

    for a in body.find_all("a", href=True):
        target = norm(a["href"])
        if target in files:
            a["href"] = LINK_TOKEN + urllib.parse.quote(target)

    # Flare indents its code samples with non-breaking spaces. Normalise them
    # so the samples can be pasted into an editor and so ordinary space-based
    # greps match. This also normalises nbsp in prose, which is the right
    # trade-off for a corpus that exists to be searched.
    for node in list(body.find_all(string=True)):
        if type(node) is NavigableString and "\u00a0" in node:
            node.replace_with(node.replace("\u00a0", " "))

    heading = body.find(["h1", "h2", "h3"])
    return body, heading.get_text(" ", strip=True) if heading else path.stem


def shift_headings(soup, by):
    """Nest a topic's headings to its depth in the TOC, so the concatenated
    document is a hierarchy rather than 73 flat H1s."""
    if by <= 0:
        return
    for level in range(5, 0, -1):
        for h in soup.find_all(f"h{level}"):
            h.name = f"h{min(level + by, 6)}"


def run_pandoc(args, stdin=None, cwd=None):
    proc = subprocess.run(
        ["pandoc", *args],
        input=stdin.encode() if stdin else None,
        capture_output=True,
        cwd=cwd,
    )
    if proc.returncode != 0:
        sys.exit(f"pandoc failed: {' '.join(args)}\n{proc.stderr.decode()}")
    return proc.stdout.decode()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "mirror",
        nargs="?",
        default=DEFAULT_MIRROR,
        type=Path,
        help=f"root of the wget mirror (default: {DEFAULT_MIRROR})",
    )
    mirror = parser.parse_args().mirror.expanduser()

    for tool in ("pandoc", "weasyprint"):
        if shutil.which(tool) is None:
            sys.exit(
                f"{tool} not found. Install with:\n"
                "  sudo pacman -S --needed pandoc-cli python-weasyprint"
            )

    if not mirror.is_dir():
        sys.exit(f"mirror not found: {mirror}\nSee docs/api-reference.md to create it.")

    topics = find_topics_dir(mirror)
    if topics is None:
        sys.exit(f"no topic directory found under {mirror}")
    print(f"topics: {topics}")

    MD_DIR.mkdir(parents=True, exist_ok=True)

    files = {p.name: p for p in topics.glob("*.htm")}
    top_order, children = collect_toc(files)
    toc = flatten(top_order, children)

    # Anything the nav does not reach still belongs in the output.
    unlinked = sorted(set(files) - {fn for fn, _ in toc})
    toc.extend((fn, 0) for fn in unlinked)

    images, fragments, titles = {}, {}, {}
    for fn, _ in toc:
        body, title = clean_topic(files[fn], files, images)
        if body is None:
            print(f"  ! no body-container, skipped: {fn}")
            continue
        fragments[fn] = str(body)
        titles[fn] = title

    if images:
        IMG_DIR.mkdir(parents=True, exist_ok=True)
        for source, flat in images.items():
            shutil.copy2(source, IMG_DIR / flat)

    md_names = {fn: slugify(Path(fn).stem) or f"topic-{i}" for i, fn in enumerate(fragments)}
    anchors = {fn: slugify(titles[fn]) for fn in fragments}

    def substitute(text, img_prefix, link_fmt):
        text = text.replace(IMG_TOKEN, img_prefix)
        for fn in fragments:
            text = text.replace(LINK_TOKEN + urllib.parse.quote(fn), link_fmt(fn))
        return text

    # --- per-topic markdown -------------------------------------------------
    for fn in fragments:
        html = substitute(fragments[fn], "../images", lambda t: f"./{md_names[t]}.md")
        out = MD_DIR / f"{md_names[fn]}.md"
        run_pandoc(["-f", "html", "-t", "gfm", "--wrap=none", "-o", str(out)], stdin=html)

    # --- concatenated HTML, headings nested by TOC depth --------------------
    sections = []
    for fn, depth in toc:
        if fn not in fragments:
            continue
        html = substitute(fragments[fn], "images", lambda t: f"#{anchors[t]}")
        frag = BeautifulSoup(html, "html.parser")
        shift_headings(frag, depth)
        heading = frag.find(re.compile(r"^h[1-6]$"))
        if heading is not None:
            heading["id"] = anchors[fn]
        sections.append(f"<section>{frag}</section>")

    single = OUT_DIR / "single.html"
    single.write_text(
        '<!DOCTYPE html>\n<html lang="en">\n<head>\n<meta charset="utf-8">\n'
        f"<title>{TITLE}</title>\n</head>\n<body>\n"
        + "\n".join(sections)
        + "\n</body>\n</html>\n",
        encoding="utf-8",
    )

    # --- single markdown ----------------------------------------------------
    run_pandoc(
        ["-f", "html", "-t", "gfm", "--wrap=none", "-o", "wxdllapidocs.md", "single.html"],
        cwd=OUT_DIR,
    )
    # A note rather than a heading, so the site's own H1 still leads.
    md_path = OUT_DIR / "wxdllapidocs.md"
    md_path.write_text(
        "> Offline conversion of the vendor's Flare help site.\n"
        f"> Source: <{SOURCE_URL}>\n\n" + md_path.read_text(encoding="utf-8"),
        encoding="utf-8",
    )

    # --- single PDF ---------------------------------------------------------
    run_pandoc(
        [
            "-f", "html", "single.html",
            "-o", "wxdllapidocs.pdf",
            "--pdf-engine=weasyprint",
            "--toc", "--toc-depth=3",
            f"--css={PRINT_CSS}",
            "--metadata", f"title={TITLE}",
            "-V", "lang=en",
        ],
        cwd=OUT_DIR,
    )

    print(f"topics converted : {len(fragments)}")
    print(f"images embedded  : {len(images)}")
    if unlinked:
        print(f"not in nav       : {', '.join(unlinked)}")
    for name in ("wxdllapidocs.pdf", "wxdllapidocs.md"):
        size = (OUT_DIR / name).stat().st_size / 1024
        print(f"{name:20} {size:.0f} KiB")
    print(f"per-topic md     : {len(list(MD_DIR.glob('*.md')))} files in {MD_DIR}")


if __name__ == "__main__":
    main()
