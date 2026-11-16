#!/usr/bin/env python3
"""Genera el banco DX7 (32 voces) 'Dark Funk Durcal' para el M-VAVE FM-1.
Formato: SysEx DX7 32-voice bulk (4104 bytes)."""
import sys, json

def env(kind):
    E = {
     'sus':   ((99,70,50,55),(99,95,90,0)),
     'organ': ((99,99,99,80),(99,99,99,0)),
     'pluck': ((99,52,35,62),(99,55,0,0)),
     'perc':  ((99,72,50,70),(99,0,0,0)),
     'short': ((99,85,60,80),(99,0,0,0)),
     'click': ((99,92,70,90),(99,0,0,0)),
     'sweep': ((99,58,40,60),(99,45,0,0)),
     'sweepS':((99,45,35,55),(99,70,40,0)),
     'pad':   ((48,32,30,42),(99,95,92,0)),
     'padm':  ((40,25,25,40),(90,99,85,0)),
     'bow':   ((60,40,35,50),(99,92,88,0)),
     'bass':  ((99,60,35,72),(99,82,70,0)),
     'bassm': ((99,66,40,70),(99,60,35,0)),
    }
    return E[kind]

def op(ol, coarse=1, fine=0, det=7, e='sus', vs=2, rs=2, mode=0, ams=0,
       bp=39, ld=0, rd=0, lc=0, rc=0):
    r, l = env(e)
    return dict(r=r, l=l, ol=ol, c=coarse, f=fine, d=det, vs=vs, rs=rs,
                mode=mode, ams=ams, bp=bp, ld=ld, rd=rd, lc=lc, rc=rc)

OFF = lambda: op(0)

def voice(name, alg, fb, ops, tr=24, peg=((99,99,99,99),(50,50,50,50)),
          lfo=(35,0,0,0,0,0,3), oks=1):
    assert len(ops) == 6
    return dict(name=name, alg=alg, fb=fb, ops=ops, tr=tr, peg=peg, lfo=lfo, oks=oks)

V = []
# ---------------- BAJOS ----------------
V.append(voice('ACID FM   ', 1, 6, [
    op(99,1,e='bass'), op(84,1,e='sweep',vs=4), op(99,1,e='bass',det=8),
    op(72,1,e='sweep'), op(58,2,e='sweep'), op(55,1,e='perc')]))
V.append(voice('RUBBER BAS', 5, 4, [
    op(99,1,e='bass'), op(78,1,e='bassm'), op(92,0,e='bass'),
    op(60,1,e='perc'), op(0), op(50,1,e='perc')]))
V.append(voice('HIDDEN SUB', 5, 0, [
    op(99,0,e='bass',vs=1), op(52,1,e='bassm'), op(85,1,e='bass'),
    op(0), op(0), op(0)]))
V.append(voice('SLAP BASS ', 1, 7, [
    op(99,1,e='pluck'), op(86,3,e='perc',vs=5), op(92,1,e='pluck',det=9),
    op(75,1,e='perc'), op(70,5,e='click'), op(62,1,e='click')]))
V.append(voice('CLOCK BASS', 16, 5, [
    op(99,1,e='bassm'), op(80,2,e='perc'), op(76,1,e='sweep'),
    op(60,3,e='click'), op(70,1,e='sweep'), op(55,1,e='perc')]))
V.append(voice('WOBBLE FM ', 5, 6, [
    op(99,1,e='bass'), op(82,1,e='sus',ams=3), op(95,1,e='bass',det=10),
    op(78,1,e='sus',ams=3), op(0), op(40,1)], lfo=(28,0,0,90,1,4,0)))
V.append(voice('LEATHER BS', 2, 6, [
    op(99,1,e='bass'), op(88,1,e='sweep',vs=4), op(90,1,e='bassm'),
    op(70,2,e='perc'), op(64,1,e='perc'), op(50,4,e='click')]))
V.append(voice('808 FM BAS', 5, 0, [
    op(99,0,e='bass'), op(45,1,e='perc'), op(0), op(0), op(0), op(0)],
    peg=((99,55,99,99),(62,50,50,50))))
# ---------------- LEADS ----------------
V.append(voice('SAW FM    ', 1, 7, [
    op(99,1,e='sus'), op(80,1,e='sweepS'), op(96,1,e='sus',det=10),
    op(82,1,e='sus'), op(75,1,e='sus'), op(70,1,e='sus')], lfo=(34,40,6,0,0,4,3)))
V.append(voice('DARK BELL ', 5, 2, [
    op(99,1,e='pluck',vs=3), op(78,3,fine=50,e='perc'), op(92,2,e='pluck',det=9),
    op(70,7,e='perc'), op(85,1,e='sus'), op(55,4,e='sus')]))
V.append(voice('CRYSTAL   ', 5, 3, [
    op(99,2,e='pluck'), op(72,7,e='perc'), op(90,4,e='pluck',det=10),
    op(65,13,e='click'), op(80,1,e='sus'), op(60,3,e='sus')]))
V.append(voice('WITCH LEAD', 1, 7, [
    op(99,1,e='sus'), op(84,2,e='sweepS'), op(94,1,e='sus',det=4),
    op(75,3,e='sus'), op(70,1,e='sus'), op(78,1,e='sus')], lfo=(30,45,12,0,0,4,5)))
V.append(voice('SOUL FLUTE', 5, 0, [
    op(99,1,e='bow'), op(62,1,e='bow'), op(82,2,e='bow'),
    op(55,1,e='bow'), op(0), op(50,1,e='short')], lfo=(30,50,8,0,0,4,4)))
V.append(voice('SQUARE EYE', 1, 5, [
    op(99,1,e='sus'), op(85,2,e='sus'), op(90,1,e='sus',det=8),
    op(80,2,e='sus'), op(0), op(0)]))
V.append(voice('GLITCH LD ', 2, 7, [
    op(99,1,e='sus'), op(88,5,fine=20,e='perc'), op(85,1,e='sus',det=12),
    op(72,7,e='click'), op(68,1,e='sus'), op(60,9,e='click')], lfo=(70,0,10,0,0,5,5)))
V.append(voice('RITUAL HRP', 5, 1, [
    op(99,1,e='pluck'), op(70,1,e='perc'), op(95,1,e='pluck',det=9),
    op(66,3,e='perc'), op(85,2,e='pluck',det=5), op(60,5,e='click')]))
# ---------------- TECLAS / PLUCKS ----------------
V.append(voice('PREP PIANO', 5, 3, [
    op(99,1,e='pluck',vs=4), op(74,1,e='perc',vs=5), op(95,1,e='pluck',det=9),
    op(66,14,e='click'), op(90,1,e='pluck',det=5), op(70,1,e='perc')]))
V.append(voice('DURCAL EP ', 5, 4, [
    op(99,1,e='pluck',vs=4), op(80,14,e='click',vs=6), op(97,1,e='sus',det=8),
    op(60,1,e='perc'), op(80,1,e='sus',det=6), op(55,1,e='perc')], lfo=(33,0,0,10,0,0,0)))
V.append(voice('MARIMBA   ', 5, 0, [
    op(99,1,e='perc'), op(76,4,e='click'), op(90,4,e='perc'),
    op(60,10,e='click'), op(0), op(0)]))
V.append(voice('EYE PLUCK ', 1, 6, [
    op(99,1,e='pluck'), op(80,1,e='perc'), op(96,1,e='pluck',det=10),
    op(78,1,e='perc'), op(70,2,e='perc'), op(60,1,e='click')]))
V.append(voice('FUNK CLAVI', 1, 7, [
    op(99,1,e='perc'), op(88,3,e='perc'), op(90,1,e='perc',det=9),
    op(80,5,e='click'), op(72,1,e='click'), op(70,2,e='click')]))
V.append(voice('BLACK KOTO', 2, 5, [
    op(99,1,e='pluck'), op(82,3,e='perc'), op(92,1,e='pluck',det=5),
    op(68,4,e='perc'), op(60,7,e='click'), op(0)]))
# ---------------- PADS ----------------
V.append(voice('FOG PAD   ', 5, 3, [
    op(97,1,e='pad'), op(70,1,e='padm'), op(95,1,e='pad',det=11),
    op(65,2,e='padm'), op(92,2,e='pad',det=3), op(55,1,e='padm')], lfo=(20,60,4,0,0,4,3)))
V.append(voice('EMPTY CHOR', 22, 4, [
    op(96,1,e='pad'), op(66,1,e='padm'), op(92,1,e='pad',det=10),
    op(90,2,e='pad',det=4), op(88,0,e='pad',det=12), op(60,3,e='padm')], lfo=(25,60,6,0,0,4,3)))
V.append(voice('MUTE ORGAN', 32, 0, [
    op(95,1,e='organ'), op(88,2,e='organ',det=8), op(84,3,e='organ'),
    op(80,4,e='organ',det=6), op(76,0,e='organ'), op(70,6,e='organ')], lfo=(40,0,5,0,0,4,3)))
V.append(voice('GLASS PAD ', 5, 2, [
    op(96,1,e='pad'), op(72,3,e='padm'), op(94,2,e='pad',det=11),
    op(62,7,e='padm'), op(88,1,e='pad',det=3), op(58,5,e='padm')], lfo=(22,50,3,0,0,4,3)))
# ---------------- PERCUSIÓN / FX ----------------
V.append(voice('FM KICK   ', 5, 0, [
    op(99,0,e='perc',vs=3), op(70,1,e='click'), op(0), op(0), op(0), op(0)],
    tr=12, peg=((99,72,99,99),(80,50,50,50))))
V.append(voice('FM SNARE  ', 32, 7, [
    op(92,1,e='short'), op(0), op(0), op(0), op(0), op(95,3,e='perc',vs=4)],
    peg=((99,80,99,99),(60,50,50,50))))
V.append(voice('FM HAT    ', 32, 7, [
    op(0), op(0), op(0), op(0), op(80,11,fine=40,e='click'), op(95,15,e='click',vs=5)]))
V.append(voice('RITUAL TOM', 5, 2, [
    op(99,1,e='perc'), op(65,1,e='click'), op(0), op(0), op(0), op(0)],
    peg=((99,60,99,99),(66,50,50,50))))
V.append(voice('ZAP LASER ', 5, 5, [
    op(99,1,e='short'), op(90,3,e='short'), op(0), op(0), op(0), op(0)],
    peg=((99,50,99,99),(99,30,50,50))))
V.append(voice('MUTANT BEL', 5, 0, [
    op(99,1,e='perc',vs=3), op(82,3,fine=50,e='sweep'), op(95,2,fine=76,e='perc',det=9),
    op(75,5,fine=30,e='sweep'), op(90,4,fine=21,e='perc',det=4), op(70,1,fine=41,e='sweep')]))
assert len(V) == 32, len(V)

def pack_voice(v):
    b = []
    # DX7 almacena OP6 primero ... OP1 al final
    for o in reversed(v['ops']):
        b += list(o['r']) + list(o['l'])
        b += [o['bp'], o['ld'], o['rd'], (o['rc'] << 2) | o['lc'],
              (o['d'] << 3) | o['rs'], (o['vs'] << 2) | o['ams'], o['ol'],
              (o['c'] << 1) | o['mode'], o['f']]
    b += list(v['peg'][0]) + list(v['peg'][1])
    b += [v['alg'] - 1, (v['oks'] << 3) | v['fb']]
    spd, dly, pmd, amd, sync, wave, pms = v['lfo']
    b += [spd, dly, pmd, amd, (pms << 4) | (wave << 1) | sync, v['tr']]
    b += [ord(c) for c in v['name'][:10].ljust(10)]
    assert len(b) == 128 and all(0 <= x < 128 for x in b), v['name']
    return b

def unpack_expanded(v):
    """Formato expandido de 155/156 bytes (usado para render de prueba)."""
    b = []
    for o in reversed(v['ops']):
        b += list(o['r']) + list(o['l'])
        b += [o['bp'], o['ld'], o['rd'], o['lc'], o['rc'], o['rs'], o['ams'], o['vs'],
              o['ol'], o['mode'], o['c'], o['f'], o['d']]
    b += list(v['peg'][0]) + list(v['peg'][1])
    spd, dly, pmd, amd, sync, wave, pms = v['lfo']
    b += [v['alg'] - 1, v['fb'], v['oks'], spd, dly, pmd, amd, sync, wave, pms, v['tr']]
    b += [ord(c) for c in v['name'][:10].ljust(10)]
    b += [0x3f]
    assert len(b) == 156
    return b

data = []
for v in V: data += pack_voice(v)
chk = (128 - (sum(data) & 127)) & 127
syx = bytes([0xF0, 0x43, 0x00, 0x09, 0x20, 0x00] + data + [chk, 0xF7])
assert len(syx) == 4104
out = sys.argv[1] if len(sys.argv) > 1 else 'DarkFunkDurcal_DX7.syx'
open(out, 'wb').write(syx)
open(out + '.expanded.bin', 'wb').write(bytes(sum((unpack_expanded(v) for v in V), [])))
print('ok', out, len(syx), [v['name'].strip() for v in V])
