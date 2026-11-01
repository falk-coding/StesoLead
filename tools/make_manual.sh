#!/usr/bin/env bash
# Bygger docs/Manual-sv.pdf och docs/Manual-en.pdf från Markdown (pandoc + Chromium via Playwright).
set -euo pipefail
cd "$(dirname "$0")/.."
tmp=$(mktemp -d)
for lang in sv en; do
  pandoc "docs/Manual-$lang.md" -s --metadata lang="$lang" \
    -c "$(pwd)/tools/manual.css" --embed-resources -o "$tmp/Manual-$lang.html" \
    --variable pagetitle="Bakelit – manual"
  node -e "
    const { chromium } = require('playwright');
    (async () => {
      const b = await chromium.launch();
      const p = await b.newPage();
      await p.goto('file://$tmp/Manual-$lang.html');
      await p.pdf({ path: 'docs/Manual-$lang.pdf', format: 'A4', printBackground: true,
                    displayHeaderFooter: true, headerTemplate: '<span></span>',
                    footerTemplate: '<div style=\"font-size:8px;width:100%;text-align:center;color:#888\">Bakelit · Falk Instruments · <span class=pageNumber></span></div>',
                    margin: { top: '16mm', bottom: '18mm', left: '16mm', right: '16mm' } });
      await b.close();
    })();"
done
rm -rf "$tmp"
echo "docs/Manual-sv.pdf och docs/Manual-en.pdf klara"
