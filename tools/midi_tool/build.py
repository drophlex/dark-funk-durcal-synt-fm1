import base64, json, glob, os
H = os.path.dirname(os.path.abspath(__file__))
pack = {}
for f in sorted(glob.glob(os.path.join(H, '..', 'midi_pack', '*.mid')) + glob.glob(os.path.join(H, '..', 'midi_pack', 'classics', '*.mid'))):
    pack[os.path.basename(f)] = base64.b64encode(open(f, 'rb').read()).decode()
s = open(os.path.join(H, 'template.html')).read().replace('/*PACK*/{}', json.dumps(pack))
open(os.path.join(H, 'DarkFunkDurcal-MIDI-Ritual.html'), 'w').write(s)
print(len(pack), 'files', len(s), 'bytes')
