#!/usr/bin/env python3
"""
build_pdf.py — конвертує pipeline/README.md у стильовий HTML,
готовий для друку в PDF через Edge/Chrome headless.

Використання:
  python3 build_pdf.py            # створює report.html
  # потім (з Windows-сторони):
  # msedge.exe --headless --disable-gpu --print-to-pdf=report.pdf report.html
"""
import re
import sys
from pathlib import Path

try:
    import markdown
except ImportError:
    print("Встанови: pip3 install --user --break-system-packages markdown")
    sys.exit(1)


HERE = Path(__file__).parent
SRC_MD = HERE / "README.md"
OUT_HTML = HERE / "report.html"


CSS = r"""
@page {
    size: A4;
    margin: 18mm 16mm 18mm 18mm;
    @bottom-center {
        content: counter(page) " / " counter(pages);
        font-family: 'Times New Roman', serif;
        font-size: 10pt;
        color: #666;
    }
}

* { box-sizing: border-box; }

html, body {
    font-family: 'Times New Roman', 'DejaVu Serif', Georgia, serif;
    font-size: 11pt;
    line-height: 1.45;
    color: #1a1a1a;
    margin: 0;
    padding: 0;
}

body {
    max-width: 100%;
    padding: 0 4mm;
}

h1 {
    font-size: 18pt;
    border-bottom: 2px solid #1a1a1a;
    padding-bottom: 4pt;
    margin-top: 0;
    page-break-before: avoid;
    page-break-after: avoid;
}

h2 {
    font-size: 14pt;
    color: #0b3a66;
    border-bottom: 1px solid #b0c4d8;
    padding-bottom: 2pt;
    margin-top: 18pt;
    margin-bottom: 8pt;
    page-break-after: avoid;
}

h3 {
    font-size: 12pt;
    color: #1a1a1a;
    margin-top: 12pt;
    margin-bottom: 4pt;
    page-break-after: avoid;
}

p { margin: 6pt 0; text-align: justify; }

strong { font-weight: bold; color: #0b3a66; }

ul, ol { margin: 6pt 0 6pt 18pt; padding: 0; }
li { margin: 2pt 0; }

a {
    color: #0b3a66;
    text-decoration: none;
    border-bottom: 1px dotted #0b3a66;
}

/* === КОД === */
code {
    font-family: 'Consolas', 'DejaVu Sans Mono', 'Courier New', monospace;
    font-size: 10pt;
    background: #f4f4f4;
    padding: 1pt 3pt;
    border-radius: 2pt;
    color: #c7254e;
}

pre {
    font-family: 'Consolas', 'DejaVu Sans Mono', 'Courier New', monospace;
    font-size: 9.5pt;
    line-height: 1.35;
    background: #f8f8f8;
    border: 1px solid #d0d0d0;
    border-left: 3px solid #0b3a66;
    padding: 6pt 8pt;
    margin: 6pt 0;
    overflow-x: auto;
    page-break-inside: avoid;
    white-space: pre-wrap;
    word-break: break-word;
}

pre code {
    background: transparent;
    padding: 0;
    border-radius: 0;
    color: #1a1a1a;
    font-size: 9.5pt;
}

/* Блок з виводом термінала (з class="language-text") */
pre > code.language-text {
    color: #1a1a1a;
}

/* === ТАБЛИЦІ === */
table {
    width: 100%;
    border-collapse: collapse;
    margin: 8pt 0;
    page-break-inside: avoid;
    font-size: 10pt;
}

th, td {
    border: 1px solid #b0b0b0;
    padding: 4pt 6pt;
    text-align: left;
    vertical-align: top;
}

th {
    background: #e8eef5;
    font-weight: bold;
    color: #0b3a66;
}

tr:nth-child(even) td { background: #fafafa; }

/* === БЛОКОВІ ЦИТАТИ === */
blockquote {
    border-left: 3px solid #b0b0b0;
    padding: 4pt 12pt;
    margin: 8pt 0 8pt 8pt;
    background: #f8f8f8;
    font-style: italic;
    color: #444;
}

/* === ТИТУЛЬНИЙ БЛОК === */
.title-block {
    text-align: center;
    margin-bottom: 16pt;
    padding-bottom: 8pt;
    border-bottom: 1px solid #b0c4d8;
}

.title-block p { margin: 2pt 0; text-align: center; }

/* === ДРУК === */
@media print {
    h2 { page-break-before: auto; }
}

hr {
    border: none;
    border-top: 1px solid #c0c0c0;
    margin: 10pt 0;
}
"""


def md_to_html(md_text: str) -> str:
    """Convert markdown body to HTML using python-markdown."""
    md = markdown.Markdown(extensions=[
        "fenced_code",
        "tables",
        "toc",
        "sane_lists",
    ])
    return md.convert(md_text)


def build_full_html(body_html: str) -> str:
    """Wrap converted body in a full styled HTML document."""
    return f"""<!DOCTYPE html>
<html lang="uk">
<head>
<meta charset="UTF-8">
<title>Звіт — Мініпроєкт АСПЗ, варіант 11</title>
<style>{CSS}</style>
</head>
<body>
{body_html}
</body>
</html>
"""


def main():
    if not SRC_MD.exists():
        print(f"Немає файлу: {SRC_MD}")
        sys.exit(1)

    md_text = SRC_MD.read_text(encoding="utf-8")

    # Видалити локальні markdown-посилання типу [`pipeline.c:37`](pipeline.c#L37)
    # перетворити на просто `pipeline.c:37` бо у PDF посилання не клікаємі на
    # локальні файли (і виглядають як підкреслений текст без сенсу).
    md_text = re.sub(
        r"\[`([^`]+)`\]\(pipeline\.c#L\d+\)",
        r"`\1`",
        md_text,
    )

    body = md_to_html(md_text)
    html = build_full_html(body)
    OUT_HTML.write_text(html, encoding="utf-8")
    print(f"OK: {OUT_HTML} ({len(html)} bytes)")


if __name__ == "__main__":
    main()
