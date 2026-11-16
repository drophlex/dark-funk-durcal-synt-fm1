#!/usr/bin/env python3
"""Dark Funk Durcal MIDI Pack: CLASSICS.
Public-domain melodies (composers dead for over a century, or traditional) re-arranged as short loops for the FM-1, and
classic breakbeat RHYTHMS programmed from scratch (a drum pattern, not a recording). Each file: format 1, 96 PPQ, one
track per FM-1 part on the channels the firmware listens to (T1..T4 = ch 1..4, drums = ch 10).
Output: midi_pack/classics/*.mid
"""
import os
import struct

PPQ = 96
S16 = PPQ // 4
NOTE = {n: i for i, n in enumerate(['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'])}
NOTE.update({'Db': 1, 'Eb': 3, 'Gb': 6, 'Ab': 8, 'Bb': 10})


def n(s):
    """'F#4' -> 66"""
    name, octv = s[:-1], int(s[-1])
    return 12 * (octv + 1) + NOTE[name]


def vlq(v):
    out = [v & 0x7F]
    v >>= 7
    while v:
        out.insert(0, 0x80 | (v & 0x7F))
        v >>= 7
    return bytes(out)


def track(name, events):
    """events: (tick, kind, a, b, ch) kind 'on'/'off'"""
    ev = sorted(events, key=lambda e: (e[0], 0 if e[1] == 'off' else 1))
    body = b'\x00\xff\x03' + bytes([len(name)]) + name.encode()
    t0 = 0
    for t, kind, a, b, ch in ev:
        body += vlq(t - t0) + bytes([(0x90 if kind == 'on' else 0x80) | ch, a, b])
        t0 = t
    body += b'\x00\xff\x2f\x00'
    return b'MTrk' + struct.pack('>I', len(body)) + body


def tempo_track(bpm, title):
    us = int(60_000_000 / bpm)
    body = b'\x00\xff\x03' + bytes([len(title)]) + title.encode()
    body += b'\x00\xff\x51\x03' + us.to_bytes(3, 'big') + b'\x00\xff\x58\x04\x04\x02\x18\x08\x00\xff\x2f\x00'
    return b'MTrk' + struct.pack('>I', len(body)) + body


def mono(seq, ch, vel=100, start=0):
    """seq: list of (note or None, length in 16ths[, accent])"""
    ev, t = [], start
    for item in seq:
        nt, ln = item[0], item[1]
        acc = len(item) > 2 and item[2]
        if nt is not None:
            for x in (nt if isinstance(nt, (list, tuple)) else [nt]):
                ev.append((t, 'on', n(x), 120 if acc else vel, ch))
                ev.append((t + ln * S16 - 4, 'off', n(x), 0, ch))
        t += ln * S16
    return ev


def drums(grid, bars=1):
    """grid: {gm note: 'x..x' strings of 16 per bar, X = accent, g = ghost}"""
    ev = []
    for note, pat in grid.items():
        for k, c in enumerate(pat):
            if c in 'xXg':
                v = 122 if c == 'X' else 50 if c == 'g' else 98
                ev.append((k * S16, 'on', note, v, 9))
                ev.append((k * S16 + S16 // 2, 'off', note, 0, 9))
    return ev


def write(path, bpm, title, parts):
    trks = [tempo_track(bpm, title)] + [track(nm, ev) for nm, ev in parts if ev]
    data = b'MThd' + struct.pack('>IHHH', 6, 1, len(trks), PPQ) + b''.join(trks)
    with open(path, 'wb') as f:
        f.write(data)


OUT = os.path.join(os.path.dirname(__file__), '..', 'midi_pack', 'classics')
os.makedirs(OUT, exist_ok=True)
K, S, CH, OH, RD = 36, 38, 42, 46, 51

# ------------------------------------------------------------------ public-domain melodies
# Grieg, In the Hall of the Mountain King (1875): the riff, B minor
hall = [('B3', 1), ('C#4', 1), ('D4', 1), ('E4', 1), ('F#4', 1), ('D4', 1), ('F#4', 2),
        ('F4', 1), ('C#4', 1), ('F4', 2), ('E4', 1), ('C4', 1), ('E4', 2),
        ('B3', 1), ('C#4', 1), ('D4', 1), ('E4', 1), ('F#4', 1), ('D4', 1), ('F#4', 1), ('B4', 1),
        ('A4', 1), ('F#4', 1), ('D4', 1), ('F#4', 1), ('A4', 4)]
write(f'{OUT}/CLASSIC_Grieg_MountainKing_140bpm_Bm.mid', 140, 'Grieg - Mountain King riff',
      [('MELODY', mono(hall, 3)), ('BASS', mono([('B1', 8), ('F#1', 8), ('B1', 8), ('F#1', 8)], 2)),
       ('DRUMS', drums({K: 'x...x...x...x...', CH: '..x...x...x...x.'}))])

# Beethoven, Fur Elise (1810): the opening, A minor
elise = [('E5', 1), ('D#5', 1), ('E5', 1), ('D#5', 1), ('E5', 1), ('B4', 1), ('D5', 1), ('C5', 1), ('A4', 2), (None, 1),
         ('C4', 1), ('E4', 1), ('A4', 1), ('B4', 2), (None, 1), ('E4', 1), ('G#4', 1), ('B4', 1), ('C5', 2), (None, 1),
         ('E4', 1), ('E5', 1), ('D#5', 1), ('E5', 1), ('D#5', 1), ('E5', 1), ('B4', 1), ('D5', 1), ('C5', 1), ('A4', 2),
         (None, 1), ('C4', 1), ('E4', 1), ('A4', 1), ('B4', 2), (None, 1), ('E4', 1), ('C5', 1), ('B4', 1), ('A4', 4)]
write(f'{OUT}/CLASSIC_Beethoven_FurElise_120bpm_Am.mid', 120, 'Beethoven - Fur Elise',
      [('MELODY', mono(elise, 3)), ('BASS', mono([('A1', 16), ('E1', 16), ('A1', 16)], 2))])

# Bach, Toccata in D minor BWV 565: the opening flourish
toc = [('A5', 1, 1), ('G5', 1), ('A5', 6), (None, 2), ('G5', 1), ('F5', 1), ('E5', 1), ('D5', 1), ('C#5', 4), ('D5', 8),
       (None, 4), ('A4', 1, 1), ('G4', 1), ('A4', 6), (None, 2), ('E4', 2), ('F4', 2), ('C#4', 4), ('D4', 8),
       (None, 4), ('A3', 1, 1), ('G3', 1), ('A3', 6), (None, 2), ('E3', 2), ('F3', 2), ('C#3', 4), ('D3', 8)]
write(f'{OUT}/CLASSIC_Bach_ToccataDm_100bpm_Dm.mid', 100, 'Bach - Toccata BWV 565 opening',
      [('LEAD', mono(toc, 3))])

# Bach, Prelude in C BWV 846: the first four bars' arpeggios (a perfect ARP source)
def arp(notes):
    return [(x, 1) for x in notes * 2]
pre = arp(['C4', 'E4', 'G4', 'C5', 'E5', 'G4', 'C5', 'E5']) + arp(['C4', 'D4', 'A4', 'D5', 'F5', 'A4', 'D5', 'F5']) + \
      arp(['B3', 'D4', 'G4', 'D5', 'F5', 'G4', 'D5', 'F5']) + arp(['C4', 'E4', 'G4', 'C5', 'E5', 'G4', 'C5', 'E5'])
write(f'{OUT}/CLASSIC_Bach_PreludeC_118bpm_C.mid', 118, 'Bach - Prelude in C BWV 846',
      [('ARP', mono(pre, 1, 90)), ('BASS', mono([('C2', 16), ('C2', 16), ('B1', 16), ('C2', 16)], 2))])

# Pachelbel, Canon in D: the ground bass and its chords
cb = [('D3', 4), ('A2', 4), ('B2', 4), ('F#2', 4), ('G2', 4), ('D2', 4), ('G2', 4), ('A2', 4)]
cc = [(['D4', 'F#4', 'A4'], 4), (['C#4', 'E4', 'A4'], 4), (['B3', 'D4', 'F#4'], 4), (['A3', 'C#4', 'F#4'], 4),
      (['B3', 'D4', 'G4'], 4), (['A3', 'D4', 'F#4'], 4), (['B3', 'D4', 'G4'], 4), (['C#4', 'E4', 'A4'], 4)]
write(f'{OUT}/CLASSIC_Pachelbel_CanonD_96bpm_D.mid', 96, 'Pachelbel - Canon in D (ground)',
      [('BASS', mono(cb, 2)), ('CHORDS', mono(cc, 3, 80))])

# Dies Irae (13th-century chant): the first phrase, D dorian - the dark theme of a thousand films
dies = [('F4', 2), ('E4', 2), ('F4', 2), ('D4', 2), ('E4', 2), ('C4', 2), ('D4', 4),
        ('F4', 2), ('F4', 2), ('G4', 2), ('F4', 2), ('E4', 2), ('D4', 2), ('C4', 2), ('E4', 2), ('F4', 2), ('E4', 2), ('D4', 4)]
write(f'{OUT}/CLASSIC_DiesIrae_Chant_88bpm_Dm.mid', 88, 'Dies Irae (chant)',
      [('MELODY', mono(dies, 3)), ('DRONE', mono([('D2', 16), ('D2', 16)], 2, 80))])

# Chopin, Funeral March (Sonata no. 2, 1839): the theme, B-flat minor
fun = [('Bb3', 4), ('Bb3', 3), ('Bb3', 1), ('Bb3', 8), ('Db4', 4), ('C4', 3), ('C4', 1), ('Bb3', 3), ('Bb3', 1),
       ('A3', 3), ('Bb3', 1), ('Bb3', 8)]
write(f'{OUT}/CLASSIC_Chopin_FuneralMarch_70bpm_Bbm.mid', 70, 'Chopin - Funeral March',
      [('MELODY', mono(fun, 3)), ('BASS', mono([(['Bb1', 'Db2'], 8), (['Bb1', 'Gb2'], 8)] * 2, 2, 85))])

# Mozart, Rondo alla Turca (1783): the opening, A minor
tur = [('B4', 1), ('A4', 1), ('G#4', 1), ('A4', 1), ('C5', 4), ('D5', 1), ('C5', 1), ('B4', 1), ('C5', 1), ('E5', 4),
       ('F5', 1), ('E5', 1), ('D#5', 1), ('E5', 1), ('B5', 1), ('A5', 1), ('G#5', 1), ('A5', 1),
       ('B5', 1), ('A5', 1), ('G#5', 1), ('A5', 1), ('C6', 4)]
write(f'{OUT}/CLASSIC_Mozart_AllaTurca_132bpm_Am.mid', 132, 'Mozart - Rondo alla Turca',
      [('MELODY', mono(tur, 3)), ('BASS', mono([('A1', 4), ('E2', 4), ('A1', 4), ('E2', 4), ('A1', 8)], 2))])

# Mozart, Eine kleine Nachtmusik (1787): the call, G major
ekn = [('G4', 2, 1), (None, 1), ('D4', 1), ('G4', 2), (None, 1), ('D4', 1), ('G4', 1), ('D4', 1), ('G4', 1), ('B4', 1),
       ('D5', 4), ('C5', 2, 1), (None, 1), ('A4', 1), ('C5', 2), (None, 1), ('A4', 1), ('C5', 1), ('A4', 1), ('F#4', 1),
       ('A4', 1), ('D4', 4)]
write(f'{OUT}/CLASSIC_Mozart_Nachtmusik_126bpm_G.mid', 126, 'Mozart - Eine kleine Nachtmusik',
      [('MELODY', mono(ekn, 3))])

# Greensleeves (traditional, 16th century), A minor
grs = [('A4', 2), ('C5', 4), ('D5', 2), ('E5', 3), ('F5', 1), ('E5', 2), ('D5', 4), ('B4', 2), ('G4', 3), ('A4', 1),
       ('B4', 2), ('C5', 4), ('A4', 2), ('A4', 3), ('G#4', 1), ('A4', 2), ('B4', 4), ('G#4', 2), ('E4', 6)]
write(f'{OUT}/CLASSIC_Trad_Greensleeves_100bpm_Am.mid', 100, 'Greensleeves (traditional)',
      [('MELODY', mono(grs, 3)), ('BASS', mono([('A1', 12), ('G1', 12), ('A1', 12), ('E1', 12)], 2))])

# ------------------------------------------------------------------ classic break rhythms (programmed, 2 bars)
def two(a, b):
    return {k: a.get(k, '.' * 16) + b.get(k, '.' * 16) for k in set(a) | set(b)}


amen1 = {K: 'x.x.......xx....', S: '....X..x.x..X..x', RD: 'x.x.x.x.x.x.x.x.'}
amen2 = {K: 'x.x.......x.....', S: '....X..x.x....X.', RD: 'x.x.x.x.x.x.x.x.'}
write(f'{OUT}/BREAK_AmenStyle_170bpm.mid', 170, 'Amen-style break (programmed)', [('DRUMS', drums(two(amen1, amen2)))])
write(f'{OUT}/BREAK_AmenStyle_136bpm.mid', 136, 'Amen-style break (programmed)', [('DRUMS', drums(two(amen1, amen2)))])

fd1 = {K: 'x.x...x...x..x..', S: '....X..g.g..X..g', CH: 'xxxxxxx.xxxxxxxx', OH: '.......x........'}
fd2 = {K: 'x.x...x...x..x..', S: '....X..g.g..X.g.', CH: 'xxxxxxx.xxxxxxxx', OH: '.......x........'}
write(f'{OUT}/BREAK_FunkyDrummerStyle_100bpm.mid', 100, 'Funky-Drummer-style break (programmed)',
      [('DRUMS', drums(two(fd1, fd2)))])

th1 = {K: 'x.x....x..x.....', S: '....X.......X...', CH: 'x.x.x.x.x.x.x.x.', OH: '..............x.'}
th2 = {K: 'x.x....x..x..x..', S: '....X..g....X...', CH: 'x.x.x.x.x.x.x.x.', OH: '......x.........'}
write(f'{OUT}/BREAK_ThinkStyle_110bpm.mid', 110, 'Think-style break (programmed)', [('DRUMS', drums(two(th1, th2)))])

ap1 = {K: 'x......x..x.....', S: '....X.......X...', CH: 'x.xxx.xxx.xxx.xx', 45: '......x.......x.'}
write(f'{OUT}/BREAK_ApacheStyle_112bpm.mid', 112, 'Apache-style break (programmed)', [('DRUMS', drums(two(ap1, ap1)))])

im1 = {K: 'x......x..x.....', S: '....X.......X...', CH: 'x.x.x.x.x.x.x.x.', OH: '..............x.'}
im2 = {K: 'x.....xx..x.....', S: '....X.......X..g', CH: 'x.x.x.x.x.x.x.x.'}
write(f'{OUT}/BREAK_ImpeachStyle_96bpm.mid', 96, 'Impeach-style break (programmed)', [('DRUMS', drums(two(im1, im2)))])

ts1 = {K: 'x.........x.....', S: '....X.......X...', CH: 'x.x.x.x.x.x.x.x.'}
ts2 = {K: 'x.........x..x..', S: '....X..g....X...', CH: 'x.x.x.x.x.x.x.xx'}
write(f'{OUT}/BREAK_TwoStep_174bpm.mid', 174, 'Two-step DnB', [('DRUMS', drums(two(ts1, ts2)))])

print('classics:', sorted(os.listdir(OUT)))
