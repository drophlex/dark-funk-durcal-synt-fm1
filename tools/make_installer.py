#!/usr/bin/env python3
"""Dark Funk Durcal Synt: the one-file FM-1 installer (Felucca's web installer with the package inside).
   tools/make_installer.py <package.fwsc> <version> <identity> <installer.html>
   Rewrites the installer in place: the package, its identity and the version shown."""
import base64, json, re, sys, pathlib
pkg, ver, ident, out = sys.argv[1:5]
p = pathlib.Path(out)
html = p.read_text()
meta = {"version": f"{ver} · Dark Funk Durcal", "product": ident,
        "pkg": "data:application/octet-stream;base64," + base64.b64encode(pathlib.Path(pkg).read_bytes()).decode()}
html, n = re.subn(r"const meta = \{.*?\};", lambda m: "const meta = " + json.dumps(meta) + ";", html, count=1, flags=re.S)
assert n == 1, "no meta in the installer"
html = re.sub(r"FIRMWARE 1\.8\.2-dfd\d+", f"FIRMWARE {ver}", html)
p.write_text(html)
print(f"installer {out}: {len(html) // 1024} KB, {ident} {ver}")
