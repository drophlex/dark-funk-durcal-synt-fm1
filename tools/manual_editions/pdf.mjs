// renders the edition pages to PDF (Letter) and reports pages whose content runs past the footer
import pw from '/opt/npm-tools/node_modules/playwright/index.js'; const { chromium } = pw; import fs from 'fs';
const jobs = JSON.parse(fs.readFileSync(process.argv[2], 'utf8'));
const b = await chromium.launch({ executablePath: '/usr/bin/google-chrome', args: ['--allow-file-access-from-files'] }); const p = await b.newPage();
for (const [src, out] of jobs) {
  await p.goto('file://' + src); await p.waitForTimeout(700);
  const over = await p.evaluate(() => [...document.querySelectorAll('.page')].map((pg, i) => { const r = pg.getBoundingClientRect(); let m = 0; pg.querySelectorAll('*').forEach(e => { if (e.closest('footer') || e.closest('.cf')) return; const q = e.getBoundingClientRect(); if (q.height) m = Math.max(m, q.bottom - r.top); }); return [i + 1, Math.round(m)]; }).filter(x => x[1] > 990));
  await p.pdf({ path: out, width: '8.5in', height: '11in', printBackground: true }); console.log(out.split('/').pop(), 'overflow:', JSON.stringify(over));
}
await b.close();
