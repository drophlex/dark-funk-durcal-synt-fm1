#!/usr/bin/env python3
"""Dark Funk Durcal Synt · manuals per edition (plug-in, web, FM-1), English, Spanish and both.

New pages (HTML -> PDF with Chrome) for the plug-in and the web editions, joined with the illustrated pages of the
full manual (manual/DarkFunkDurcal-Manual-*.pdf) that belong to each edition:
  python3 build.py            -> manual/DarkFunkDurcal-{Plugin,Web,FM1}-Manual-{EN,ES,EN-ES}.pdf
Needs: pypdf, node + playwright (Chrome)."""
import os, subprocess, json, pathlib
from pypdf import PdfReader, PdfWriter
HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parents[1]
MAN = ROOT / 'manual'
OUT = pathlib.Path('/tmp/dfd_manual_editions'); OUT.mkdir(exist_ok=True)
L = 'en'
def T(en, es): return es if L == 'es' else en
DON = 'https://www.paypal.com/donate/?business=drophlex%40gmail.com&amp;no_recurring=0&amp;item_name=Dark+Funk+Durcal&amp;currency_code=USD'
WEB = 'https://drophlex.github.io/dark-funk-durcal-synt-fm1/'
IG, YT = 'https://www.instagram.com/polaris.sound.lab/', 'https://www.youtube.com/@PolarisSoundLab'
CSS = """
@page{size:8.5in 11in;margin:0}*{box-sizing:border-box}
body{margin:0;background:#120f0d;color:#eee5d6;font:12.4px/1.55 Inter,-apple-system,'Segoe UI',Arial,sans-serif;-webkit-print-color-adjust:exact;print-color-adjust:exact}
.page{width:8.5in;height:11in;padding:.62in .66in .74in;position:relative;overflow:hidden;page-break-after:always;background:radial-gradient(120% 70% at 50% 0%,#211915,#120f0d)}
.kick{font:10px 'IBM Plex Mono',ui-monospace,monospace;letter-spacing:3px;color:#e0533a;text-transform:uppercase;margin-bottom:6px}
h1{font:400 33px/1.08 Georgia,'Times New Roman',serif;letter-spacing:1.5px;margin:0 0 12px;text-transform:uppercase;color:#f3ead9}
h1 i{font-style:normal;color:#e0533a}
h2{font:400 17px Georgia,serif;letter-spacing:1px;margin:16px 0 6px;color:#f3ead9;text-transform:uppercase}
p{margin:6px 0}.lead{font-size:14px;color:#d7ccba}
a{color:#f0b49f}
figure{margin:10px auto}figure img{display:block;width:100%;border-radius:8px;border:1px solid #3a2d26}
figcaption{font-size:10px;color:#a3978a;margin-top:5px;text-align:center}
table{width:100%;border-collapse:collapse;margin:8px 0;font-size:11.3px}
th{text-align:left;font:9.5px 'IBM Plex Mono',monospace;letter-spacing:1.4px;color:#e0533a;border-bottom:1px solid #4a3a31;padding:5px 6px}
td{border-bottom:1px solid #2c231e;padding:5px 6px;vertical-align:top}td:first-child{color:#f3ead9;font-weight:600;white-space:nowrap}
.tip{border-left:3px solid #e0533a;background:#1d1714;padding:8px 12px;margin:10px 0;border-radius:0 6px 6px 0;font-size:11.5px}.tip b{color:#e0533a;margin-right:4px}
.code{background:#0a0807;border:1px solid #3a2d26;border-radius:6px;padding:9px 12px;font:11.5px 'IBM Plex Mono',monospace;margin:6px 0}
.two{display:grid;grid-template-columns:1fr 1fr;gap:16px;align-items:start}
.callout{position:relative;margin:8px auto}.callout img{display:block;width:100%;border-radius:8px;border:1px solid #3a2d26}
.dot{position:absolute;transform:translate(-50%,-50%);width:22px;height:22px;border-radius:50%;background:#e0533a;color:#120f0d;border:1.5px solid #f3ead9;font:700 11px/19px Arial;text-align:center}
footer{position:absolute;left:.66in;right:.66in;bottom:.34in;display:flex;justify-content:space-between;font:9px 'IBM Plex Mono',monospace;letter-spacing:1.5px;color:#8f8276;border-top:1px solid #3a2d26;padding-top:6px}
.cover{height:100%;display:flex;flex-direction:column;align-items:center;justify-content:center;text-align:center}
.cover .eye{width:3.2in;height:3.2in}.ct{font:400 46px/1.05 Georgia,serif;letter-spacing:7px;margin-top:20px}.cs{font:400 18px Georgia,serif;letter-spacing:12px;color:#c9bca9;margin-top:6px}
.cm{font:11px 'IBM Plex Mono',monospace;letter-spacing:3px;color:#e0533a;margin-top:26px}.cl{font:10px 'IBM Plex Mono',monospace;letter-spacing:2px;border:1px solid #e0533a;color:#e0533a;border-radius:99px;padding:3px 12px;margin-top:10px}
.cf{position:absolute;bottom:.5in;left:0;right:0;font-size:10.5px;color:#a3978a;line-height:1.8}.cf b{color:#eee5d6}
.don{display:inline-block;margin-top:6px;background:#e0533a;color:#120f0d;border-radius:99px;padding:3px 12px;font-weight:700;text-decoration:none}
"""
EYE = """<svg class="eye" viewBox="0 0 200 200"><g fill="none" stroke="#eee5d6" stroke-width="2.2"><circle cx="100" cy="100" r="62" stroke-dasharray="9 6"/>
<path d="M100 38 L158 140 L42 140 Z"/><path d="M58 106 Q100 70 142 106 Q100 136 58 106 Z"/><circle cx="100" cy="106" r="13"/></g><circle cx="100" cy="106" r="5" fill="#e0533a"/>
<g stroke="#eee5d6" stroke-width="2">""" + ''.join(f'<line x1="{100 + 74 * __import__("math").cos(a * 0.3927):.1f}" y1="{100 + 74 * __import__("math").sin(a * 0.3927):.1f}" x2="{100 + 86 * __import__("math").cos(a * 0.3927):.1f}" y2="{100 + 86 * __import__("math").sin(a * 0.3927):.1f}"/>' for a in range(16)) + "</g></svg>"

def img(n, cap='', w='100%'):
    src = (HERE / 'img' / n).as_uri()
    return f'<figure style="width:{w}"><img src="{src}">' + (f'<figcaption>{cap}</figcaption>' if cap else '') + '</figure>'
def table(head, rows): return '<table><tr>' + ''.join(f'<th>{h}</th>' for h in head) + '</tr>' + ''.join('<tr>' + ''.join(f'<td>{c}</td>' for c in r) + '</tr>' for r in rows) + '</table>'
def tip(t): return f'<div class="tip"><b>{T("Tip", "Consejo")}</b> {t}</div>'
def callouts(n, spots):
    src = (HERE / 'img' / n).as_uri()
    return '<div class="callout"><img src="' + src + '">' + ''.join(f'<span class="dot" style="left:{x}%;top:{y}%">{i}</span>' for i, (x, y) in enumerate(spots, 1)) + '</div>'

def cover(edition):
    name = {'plugin': T('PLUG-IN EDITION · VST3 · AU · APP', 'EDICIÓN PLUG-IN · VST3 · AU · APP'), 'web': T('WEB EDITION', 'EDICIÓN WEB')}[edition]
    return f'''<div class="cover">{EYE}<div class="ct">DARK FUNK DURCAL</div><div class="cs">SYNT</div>
<div class="cm">{T('USER MANUAL', 'MANUAL DE USUARIO')} · {name}</div><div class="cl">{T('ENGLISH EDITION', 'EDICIÓN EN ESPAÑOL')}</div>
<div class="cf">Dark Funk Durcal {T('design', 'diseño')} · {T('developed at', 'desarrollado en')} <b>POLARIS SOUND LAB</b> · Mérida, Yucatán, México · <a href="{DON}">♥ {T('DONATE', 'DONAR')}</a><br>
<a href="{IG}">Instagram @polaris.sound.lab</a> · <a href="{YT}">YouTube @PolarisSoundLab</a> · <a href="{WEB}">drophlex.github.io/dark-funk-durcal-synt-fm1</a></div></div>'''

def plugin_pages():
    P = [('cover', cover('plugin'))]
    P.append((T('The plug-in', 'El plug-in'), f'''<div class="kick">{T('The plug-in, piece by piece', 'El plug-in, pieza por pieza')}</div><h1>{T('A whole <i>ritual</i> in one window', 'Todo un <i>ritual</i> en una ventana')}</h1>
{callouts('plugin_main.jpg', [(13, 11), (55, 4), (82, 4), (93, 4), (50, 22), (50, 47), (82, 23), (82, 56), (50, 79), (90, 96)])}
{table(['#', T('Part', 'Parte'), T('What it does', 'Qué hace')], [
 ('1', 'RITUAL', T('two styles (A and B), FUSION between them, CHAOS, the mood and the key; <b>SUMMON</b> writes drums, acid, bass and melody at once', 'dos estilos (A y B), FUSIÓN entre ellos, CAOS, el sentimiento y la tonalidad; <b>INVOCAR</b> escribe batería, ácido, bajo y melodía a la vez')),
 ('2', T('Presets · UNDO · REDO', 'Presets · DESHACER · REHACER'), T('43 factory sounds; every change can be undone', '43 sonidos de fábrica; cada cambio se puede deshacer')),
 ('3', 'PLAY', T('its own transport, like XO: it plays with the DAW stopped (the DAW\'s play drives it too)', 'su propio transporte, como XO: suena con el DAW detenido (el play del DAW también lo mueve)')),
 ('4', '⇢ MIDI · ⇢ WAV', T('drag the pattern onto a track, as MIDI (one channel) or as a loop of audio', 'arrastra el patrón a una pista, como MIDI (un canal) o como loop de audio')),
 ('5', T('The eye', 'El ojo'), T('it feels the music: the more intense, the wider it opens', 'siente la música: entre más intensa, más se abre')),
 ('6', T('The pattern', 'El patrón'), T('drums, acid, bass and lead; the playing step glows', 'batería, ácido, bajo y lead; el paso que suena brilla')),
 ('7', 'LIVE', T('8 pads: four mutes, FILL, STUTTER, BUILD/DROP, NEXT PHRASE; scenes A–H', '8 pads: cuatro silencios, FILL, STUTTER, BUILD/DROP, SIGUIENTE FRASE; escenas A–H')),
 ('8', 'NEXT · KEYS', T('3 NEXT offers three melodies (try 1, 2, 3, KEEP); KEYS PLAY picks what your keyboard plays, in key', '3 NEXT ofrece tres melodías (prueba 1, 2, 3, KEEP); LAS TECLAS TOCAN elige qué toca tu teclado, en tono')),
 ('9', 'SOUND', T('8 banks of 8 knobs (next page)', '8 bancos de 8 perillas (página siguiente)')),
 ('10', 'ES · ? · ♥', T('language, the quick guide, donate', 'idioma, la guía rápida, donar'))])}'''))
    banks = [('1 PERFORM', 'DJ Filter · Drive · Echo · Reverb · Acid Cutoff · Acid Reso · Acid Env · Acid Decay'),
             ('2 ACID', 'Accent · Slide Time · Saw/Square · Acid Drive · Acid Tune · Acid Level · Acid Echo · Acid Reverb'),
             ('3 DRUMS', 'Drum Kit · Drum Pitch · Drum Decay · Drum Crush · Kick Tune · Kick Decay · Snare Tone · Hat Decay'),
             ('4 BASS', 'Bass Sound · Bass Tone · Bass Wah · Bass Glide · Bass Drive · Bass Detune · Bass Tune · Bass Level'),
             ('5 LEAD', 'Lead Sound · Brightness · Vowel · Vibrato · Attack · Release · Detune · Lead Level'),
             ('6 FX', 'Echo Heads · Echo Time · Echo Repeats · Tape Wow · Reverb Size · Reverb Damp · Bit Crush · Rate Crush'),
             ('7 GROOVE', 'Style A · Style B · Fusion · Chaos · Feel · Key · Swing · Density'),
             ('8 MIX & PLAY', 'Drums Level · Drums Echo · Lead Echo · Lead Reverb · Stutter Rate · Keys Play · Scale Lock · Master')]
    P.append((T('Sound', 'Sonido'), f'''<div class="kick">SOUND</div><h1>{T('Eight banks, <i>sixty-four knobs</i>', 'Ocho bancos, <i>sesenta y cuatro perillas</i>')}</h1>
<p class="lead">{T('The SOUND strip at the bottom has eight banks. Click a bank (or move through them from a controller) and its eight knobs appear. Every knob is a DAW parameter you can automate, and every one is saved with the set.', 'La franja SOUND de abajo tiene ocho bancos. Pulsa un banco (o recórrelos desde un controlador) y aparecen sus ocho perillas. Cada perilla es un parámetro del DAW que puedes automatizar, y todas se guardan con el set.')}</p>
{table([T('Bank', 'Banco'), T('Its eight knobs', 'Sus ocho perillas')], banks)}
{tip(T('Start on 1 PERFORM: it has the moves that matter live (the DJ filter, drive, echo, reverb and the acid filter). Bank 7 GROOVE re-shapes the ritual itself.', 'Empieza en 1 PERFORM: tiene los movimientos que importan en vivo (el filtro DJ, saturación, eco, reverb y el filtro del ácido). El banco 7 GROOVE cambia el ritual mismo.'))}
{tip(T('A knob: drag up or down; double-click returns it to its default. Right-click a knob in Live to automate it.', 'Una perilla: arrastra arriba o abajo; doble clic la regresa a su valor inicial. Clic derecho en una perilla en Live para automatizarla.'))}'''))
    P.append((T('Drag, drop, transport', 'Arrastrar, soltar, transporte'), f'''<div class="kick">{T('In your DAW', 'En tu DAW')}</div><h1>{T('Into Ableton <i>and back</i>', 'Hacia Ableton <i>y de vuelta</i>')}</h1>
{table([T('Feature', 'Función'), T('How it works', 'Cómo funciona')], [
 ('PLAY / STOP', T('Like XO: the plug-in\'s own PLAY plays with Live stopped, to try ideas. When Live plays, the plug-in follows Live\'s tempo and bar; always at Live\'s tempo.', 'Como XO: el PLAY propio del plug-in suena con Live detenido, para probar ideas. Cuando Live toca, el plug-in sigue el tempo y el compás de Live; siempre al tempo de Live.')),
 ('⇢ MIDI', T('Drag it onto a MIDI track: the whole pattern in <b>one track, one channel</b> (1), drums on their GM notes. It plays a Drum Rack and synths as it is.', 'Arrástralo a una pista MIDI: todo el patrón en <b>una pista, un canal</b> (1), la batería en sus notas GM. Toca un Drum Rack y sintes tal cual.')),
 ('⇢ WAV', T('Drag it onto an audio track: the pattern rendered at 48 kHz / 24-bit by a fresh copy of the engine, twice through, the second pass kept: the echo and reverb tails of the end ring into the start, so it loops cleanly.', 'Arrástralo a una pista de audio: el patrón hecho a 48 kHz / 24 bits por una copia nueva del motor, dos veces, quedándose con la segunda: las colas de eco y reverb del final suenan en el inicio, así hace loop limpio.')),
 (T('Drop a .mid', 'Suelta un .mid'), T('Drag a MIDI clip from Live or the Finder onto the plug-in: drums (channel 10, or only drum notes) fill the drum lanes; a melody goes to BASS, ACID or LEAD by its register, with ties for held notes, slide where notes overlap and accent from the velocity.', 'Arrastra un clip MIDI desde Live o el Finder sobre el plug-in: la batería (canal 10, o sólo notas de batería) llena los carriles de batería; una melodía va a BAJO, ÁCIDO o LEAD según su registro, con ligaduras en notas sostenidas, slide donde se encimen y acento por la velocidad.')),
 (T('Keys and MIDI in', 'Teclas y MIDI'), T('Your keyboard (or a clip) plays the part chosen in KEYS PLAY, always in key with SCALE LOCK; KEYS TRANSPOSE moves the whole pattern.', 'Tu teclado (o un clip) toca la parte elegida en LAS TECLAS TOCAN, siempre en tono con SCALE LOCK; KEYS TRANSPOSE mueve todo el patrón.'))])}
{img('plugin_help_' + ('es' if L == 'es' else 'en') + '.jpg', T('The quick guide (?): every feature in one line.', 'La guía rápida (?): cada función en una línea.'), '88%')}'''))
    return P

def web_pages():
    tabs = [(1, '◉ LIVE', T('the 8 pads, scenes A–H and NEXT PHRASE, played with the mouse, the computer keys or a MIDI controller', 'los 8 pads, escenas A–H y SIGUIENTE FRASE, con el ratón, las teclas o un controlador MIDI')),
            (2, T('SEQUENCE', 'SECUENCIA'), T('the grid: paint hits, then accents, slides, rolls, chance or notes; each track has its own length (polyrhythm)', 'la cuadrícula: pinta golpes, luego acentos, slides, redobles, probabilidad o notas; cada pista tiene su propio largo (polirritmo)')),
            (3, T('MELODY', 'MELODÍA'), T('the melody maker: pick a shape and a feel, every note fits', 'el creador de melodías: elige una forma y un sentimiento, todas las notas encajan')),
            (4, T('DICE', 'DADOS'), T('re-roll one part (drums, acid, bass, melody, sound) when you don\'t know what to do', 'vuelve a tirar una parte (batería, ácido, bajo, melodía, sonido) cuando no sabes qué hacer')),
            (5, T('SOUND', 'SONIDO'), T('each track\'s synth: presets, filter, accent, oscillator, character, movement and mixer', 'el sinte de cada pista: presets, filtro, acento, oscilador, carácter, movimiento y mezcla')),
            (8, 'MIDI', T('Web MIDI (Chrome, Edge): routing per track, clock in and out, MIDI learn, the FM-1 preset and the DX7 bank', 'Web MIDI (Chrome, Edge): ruteo por pista, reloj de entrada y salida, aprender MIDI, el preset del FM-1 y el banco DX7')),
            (9, 'FM-1', T('play your FM-1 from the web: it plays its own FM engines from this sequencer', 'toca tu FM-1 desde la web: suena con sus propios motores FM desde este secuenciador'))]
    lg = 'es' if L == 'es' else 'en'
    P = [('cover', cover('web'))]
    P.append((T('The web groovebox', 'La groovebox web'), f'''<div class="kick">{T('The web groovebox', 'La groovebox web')}</div><h1>{T('Open it, press <i>AWAKEN</i>', 'Ábrela, pulsa <i>DESPERTAR</i>')}</h1>
<p class="lead">{T('The web edition is a whole groovebox in the browser, on a computer or a phone: eight tracks (kick, snare, hats, glitch, acid, bass, melody, chords), styles that mix, a live mode and MIDI. Nothing to install.', 'La edición web es una groovebox completa en el navegador, en computadora o teléfono: ocho pistas (bombo, tarola, hats, glitch, ácido, bajo, melodía, acordes), estilos que se mezclan, un modo en vivo y MIDI. No hay nada que instalar.')}</p>
<div class="two"><div>{img('web_' + lg + '.jpg', T('STYLES: two styles, FUSION, the feel, the key and SUMMON A WHOLE TRACK.', 'ESTILOS: dos estilos, FUSIÓN, el sentimiento, la tonalidad e INVOCAR PISTA COMPLETA.'))}</div>
<div><h2>{T('Your first track', 'Tu primera pista')}</h2><ol><li>{T(f'Open <a href="{WEB}">the web</a> and press <b>AWAKEN</b> (browsers start audio after a click). EN / ES is at the top right.', f'Abre <a href="{WEB}">la web</a> y pulsa <b>DESPERTAR</b> (los navegadores arrancan el audio tras un clic). EN / ES está arriba a la derecha.')}</li>
<li>{T('Pick <b>Style A</b> and <b>Style B</b>, move <b>FUSION</b>, choose a <b>feel</b>.', 'Elige <b>Estilo A</b> y <b>Estilo B</b>, mueve <b>FUSIÓN</b>, escoge un <b>sentimiento</b>.')}</li>
<li>{T('Press <b>SUMMON A WHOLE TRACK</b>, then <b>PLAY</b>. Or start from one of the 19 rituals (ready tracks).', 'Pulsa <b>INVOCAR PISTA COMPLETA</b> y luego <b>TOCAR</b>. O empieza con uno de los 19 rituales (pistas listas).')}</li>
<li>{T('Drag across the eye: filter and colour. TAP sets the tempo; ⟲1/16 · 1/8 · 1/4 are loops of the last beats.', 'Arrastra sobre el ojo: filtro y color. TAP fija el tempo; ⟲1/16 · 1/8 · 1/4 son loops de los últimos tiempos.')}</li></ol>
{tip(T('No theory needed: every note always falls inside the scale of the feel and key you chose.', 'No necesitas teoría: cada nota siempre cae dentro de la escala del sentimiento y la tonalidad que elegiste.'))}</div></div>'''))
    P.append((T('The tabs', 'Las pestañas'), f'''<div class="kick">{T('The tabs', 'Las pestañas')}</div><h1>{T('Everything is <i>one tab away</i>', 'Todo está a <i>una pestaña</i>')}</h1>
{table([T('Tab', 'Pestaña'), T('What it holds', 'Qué contiene')], [(n, d) for _, n, d in tabs] + [(T('MIX · STRETCH · PROJECT', 'MEZCLA · STRETCH · PROYECTO'), T('levels and sends · time-stretch of a track · save, open, export MIDI and WAV', 'niveles y envíos · estirar el tiempo de una pista · guardar, abrir, exportar MIDI y WAV'))])}
<div class="two"><div>{img('web_' + lg + '_tab2.jpg', T('SEQUENCE', 'SECUENCIA'))}</div><div>{img('web_' + lg + '_tab1.jpg', '◉ LIVE')}{img('web_' + lg + '_tab8.jpg', 'MIDI')}</div></div>'''))
    P.append((T('Sound and dice', 'Sonido y dados'), f'''<div class="kick">{T('Sound, melody and dice', 'Sonido, melodía y dados')}</div><h1>{T('Shape every <i>voice</i>', 'Dale forma a cada <i>voz</i>')}</h1>
<div class="two"><div>{img('web_' + lg + '_tab5.jpg', T('SOUND: the synth of the selected track, with its presets and the scale pads.', 'SONIDO: el sinte de la pista elegida, con sus presets y los pads en escala.'))}</div>
<div>{img('web_' + lg + '_tab3.jpg', T('MELODY', 'MELODÍA'))}{img('web_' + lg + '_tab4.jpg', T('DICE', 'DADOS'))}</div></div>
{tip(T('Exports: PROJECT saves the session; MIDI downloads the pattern for any DAW (one channel per track for the FM-1 in the MIDI tab); the plug-in edition drags MIDI and WAV straight onto tracks.', 'Exportar: PROYECTO guarda la sesión; MIDI descarga el patrón para cualquier DAW (un canal por pista para el FM-1 en la pestaña MIDI); la edición plug-in arrastra MIDI y WAV directo a las pistas.'))}'''))
    return P

def render(pages, lang, path):
    global L; L = lang
    body = []
    for i, (title, html) in enumerate(pages):
        if title == 'cover': body.append(f'<section class="page">{html}</section>'); continue
        body.append(f'<section class="page">{html}<footer><span>DARK FUNK DURCAL SYNT · {T("MANUAL", "MANUAL")}</span><span>{title}</span><span>{i + 1:02d}</span></footer></section>')
    path.write_text(f'<!doctype html><html lang="{lang}"><meta charset="utf-8"><style>{CSS}</style><body>{"".join(body)}</body></html>')

def build_pages(fn, lang):
    global L; L = lang; return fn()

if __name__ == '__main__':
    jobs = []
    for ed, fn in (('plugin', plugin_pages), ('web', web_pages)):
        for lang in ('en', 'es'):
            h = OUT / f'{ed}_{lang}.html'; render(build_pages(fn, lang), lang, h); jobs.append([str(h), str(OUT / f'{ed}_{lang}.pdf')])
    (OUT / 'jobs.json').write_text(json.dumps(jobs))
    subprocess.run(['node', str(HERE / 'pdf.mjs'), str(OUT / 'jobs.json')], check=True)
    # the full manual's own pages for each edition (1-based page numbers of DarkFunkDurcal-Manual-EN/ES.pdf)
    OLD = {'plugin': [22, 25, 26, 27, 29], 'web': [24, 21, 28, 29], 'fm1': list(range(1, 24)) + [28, 29]}
    NAME = {'plugin': 'Plugin', 'web': 'Web', 'fm1': 'FM1'}
    for ed in ('plugin', 'web', 'fm1'):
        per = {}
        for lang in ('en', 'es'):
            old = PdfReader(str(MAN / f'DarkFunkDurcal-Manual-{lang.upper()}.pdf')); w = PdfWriter()
            if ed != 'fm1':
                new = PdfReader(str(OUT / f'{ed}_{lang}.pdf'))
                for p in new.pages: w.add_page(p)
            for n in OLD[ed]: w.add_page(old.pages[n - 1])
            out = MAN / f'DarkFunkDurcal-{NAME[ed]}-Manual-{lang.upper()}.pdf'
            with open(out, 'wb') as f: w.write(f)
            per[lang] = out; print(out.name, len(w.pages), 'pages')
        w = PdfWriter()
        for lang in ('en', 'es'):
            for p in PdfReader(str(per[lang])).pages: w.add_page(p)
        out = MAN / f'DarkFunkDurcal-{NAME[ed]}-Manual-EN-ES.pdf'
        with open(out, 'wb') as f: w.write(f)
        print(out.name, len(w.pages), 'pages')
