<p align="center"><img src="docs/images/boot7.png" width="300" alt="Dark Funk Durcal Synt boot screen"></p>

<h1 align="center">DARK FUNK DURCAL SYNT</h1>
<p align="center"><b>Firmware + web groovebox for the M-VAVE FM-1</b><br>
Acid · breaks · rituals · melodies without theory</p>
<p align="center">Created at <b>POLARIS SOUND LAB</b><br>
Mérida, Yucatán, México<br>
<a href="https://www.instagram.com/polaris.sound.lab/">Instagram @polaris.sound.lab</a> · <a href="https://www.youtube.com/@PolarisSoundLab">YouTube @PolarisSoundLab</a></p>

> **What is Dark Funk Durcal?** A music project. Search it on **Spotify, YouTube and every audio platform** —
> its music is available on every digital platform.

**English** · [Español](#español)

---

## What it is

Dark Funk Durcal Synt turns the FM-1 into a dark, easy groovebox for acid lines, chopped breaks, jungle, drum'n'bass,
braindance and rave — with no music theory needed: every generated note is always in key.

It is a fork of [Felucca 1.8.2](https://github.com/hugelton/Felucca) (GPL-3.0, Hügelton Instruments). **Every sound
and feature of Felucca stays**; Dark Funk Durcal adds:

| | |
|---|---|
| **RITUAL** | Pick two of 11 styles (ACID, DRILL N BASS, JUNGLE, ROLLING DNB, BREAKCORE, BIG BEAT, RAVE HARDCORE, BRAINDANCE, DARK ELECTRO, AMBIENT IDM, BROKEN FUNK), blend them with FUSION, add CHAOS and press **SUMMON**: drums or a chopped break, acid, bass and melody in all 4 tracks at once. |
| **MELODY** | Feel (scale), key, shape (WAVE, RISE, FALL, Q&A, ARP, HOOK, FEW, BUSY) and density → **MAKE**. Always in key. |
| **DICE** | Each knob rolls a die for one part (drums, acid, melody, sound) — or roll them **ALL**. Hold SAVE to undo. |
| **STRETCH** | Easy granular time-stretch locked to the tempo: FREEZE, 1/2, 1, 2, 4 bars; pitch ±12 without changing time. |
| **TAPE ECHO** *(new in dfd4)* | A three-head tape echo on the delay: HEADS (H1, H2, H3, H1+2, H2+3, H1+3, ALL), WOW (wow, flutter and tape saturation). Repeats at TIME ×1 ×2 ×3, darkening and never running away. |
| **The living eye** *(new in dfd4)* | The animated eye on HOME and the Dark Funk Durcal pages **reacts to the music**: calm and half-closed in ambient, furious (frown, veins, red iris, trembling) when the tempo, density, volume, distortion and resonance rise. Works with MENU › LARGE too. |
| **Sounds** | 10 new acid / bass / pad presets on the ANALOG engine (DFD ACID, SQUELCH, BLACK ACID, RUBBER ACID, SERPENT, ACID LASER, ACID 2, REESE, DETUNE PAD, BRAIN LEAD) and a 32-voice FM bank (FM6 SLOT B1–B32). |
| **Boot ritual** | An animated eye splash: *What is Dark Funk Durcal? Search it on Spotify, YouTube and every audio platform* · *Created at Polaris Sound Lab · Mérida, Yucatán, México*. |
| **MIDI Ritual** *(new)* | A web page that writes MIDI files into the FM-1 tracks and saves your FM-1 tracks as `.mid`, with a pack of 50 sequences built in. |

<p align="center"><img src="docs/images/dfd_pages.png" alt="RITUAL, MELODY, DICE and STRETCH screens"></p>
<p align="center"><img src="docs/images/eye_moods.png" alt="The eye: calm, awake, furious"><br><i>The living eye: calm · awake · furious</i></p>

## Files in this repository

| Path | What it is |
|---|---|
| [`install/DarkFunkDurcal-Installer.html`](install/DarkFunkDurcal-Installer.html) | **The installer.** Open it in Chrome or Edge with the FM-1 on USB and press *Install*. The firmware is inside it. It can also take you back to the official M-VAVE firmware. |
| [`install/DarkFunkDurcal-1.8.2-dfd4.fwsc`](install/DarkFunkDurcal-1.8.2-dfd4.fwsc) | The firmware package itself (for Felucca's tools / advanced users). Identity FM-1_918. |
| [`manual/DarkFunkDurcal-Manual-EN-ES.pdf`](manual/DarkFunkDurcal-Manual-EN-ES.pdf) | The full illustrated manual, English then Spanish (46 pages). |
| [`manual/DarkFunkDurcal-Manual-ES.pdf`](manual/DarkFunkDurcal-Manual-ES.pdf) | The manual in Spanish only (23 pages). |
| [`web/DarkFunkDurcalSynt.html`](web/DarkFunkDurcalSynt.html) | The web groovebox: 8 channels, styles A × B, dice, melody maker, acid tool, time-stretch, MIDI out / clock, `.mid` export, FM-1 screen preview. Works offline in any modern browser. |
| [`web/DarkFunkDurcal-MIDI-Ritual.html`](web/DarkFunkDurcal-MIDI-Ritual.html) | MIDI Ritual: load a `.mid` (or one of the 50 built-in) into the FM-1's tracks; save the FM-1's 4 tracks as `.mid`. Chrome / Edge. |
| [`midi/pack/`](midi/pack) | 33 sequences, 3 per style (Simple / Medium / Wild): drums on ch 10, acid, bass, melody, chords. |
| [`midi/classics/`](midi/classics) | 10 public-domain classics re-arranged as loops (Bach, Beethoven, Mozart, Grieg, Chopin, Pachelbel, Dies Irae, Greensleeves) and 7 classic break rhythms programmed from scratch (Amen, Funky Drummer, Think, Apache, Impeach styles, two-step). |
| [`midi/DarkFunkDurcal-MIDI-Pack.zip`](midi/DarkFunkDurcal-MIDI-Pack.zip) | All of the above in one zip. |
| [`sounds/DarkFunkDurcal_DX7.syx`](sounds/DarkFunkDurcal_DX7.syx) | The 32-voice FM bank as a DX7 SysEx bank (for Dexed or any DX7-compatible synth). On the FM-1 it is already built in. |
| [`firmware-src/`](firmware-src) | The complete source code of this firmware (Felucca 1.8.2 + Dark Funk Durcal), as the GPL-3.0 requires. [`DARK-FUNK-DURCAL.patch`](firmware-src/DARK-FUNK-DURCAL.patch) shows every change against Felucca 1.8.2. |
| [`tools/`](tools) | Scripts that generate the classics MIDI files, the DX7 bank and the MIDI Ritual page. |
| [`docs/images/`](docs/images) | Screens rendered from the real firmware drawing code. |

## Install (short version — the manual has every step with pictures)

1. Use a **USB-C data cable**, connected **directly** to the computer (no hub). Battery charged.
2. Quit every app that uses MIDI (DAWs, the M-VAVE app, other browser tabs).
3. On a Mac, check *Audio MIDI Setup › Window › Show MIDI Studio*: the **FM-1** icon must be there.
4. Open `install/DarkFunkDurcal-Installer.html` in **Chrome or Edge**, press **Install**, allow MIDI.
5. Wait for *Starting › Verifying › Switching to update mode › Writing › Restarting › Done*. Don't unplug while *Writing*.
6. The FM-1 restarts with the eye animation. Done.

To go back: in the installer open *Return to official firmware (V15 / V16)* and choose the official `FM-1.fwsc` from M-VAVE.
The installer saves a complete backup of your music first.

<p align="center"><img src="docs/images/installer_dfd4.png" width="420" alt="The installer"></p>

## How to reach each screen

| Press | You get |
|---|---|
| **HOME** (repeatedly) | HOME › MIXER › CLOCK › **RITUAL › MELODY › DICE › STRETCH** › HOME |
| **FX** (repeatedly) | FX › SLICER › INSERT › INSERT 2 › DLY › **TAPE ECHO** › REVERB › CHORUS |
| **SEQ** | STEP › DETAIL › AUTOMATION |
| **EDIT** | the engine's pages (FILTER for the acid, LANES for drums, PATCH / SLOT for FM6…) |
| **SAVE** | USER › PRESETS › PHRASES › PROJECT › TOOLS — hold SAVE = **undo** |
| **ALGORITHM** knob | choose the track T1–T4 · **PRESETS** knob = sound · **SELECT** = tempo |

On the Dark Funk Durcal pages **OCT+** is the action (SUMMON, MAKE, ALL, APPLY) and **OCT−** goes back.

<p align="center"><img src="docs/images/panel.png" width="480" alt="FM-1 controls"></p>

<p align="center">
<img src="docs/images/home.png" width="200"> <img src="docs/images/tape.png" width="200"> <img src="docs/images/step_acid.png" width="200"> <img src="docs/images/step_drums.png" width="200">
</p>

## MIDI: loading and saving sequences

Open `web/DarkFunkDurcal-MIDI-Ritual.html` in Chrome with the FM-1 connected → **Connect** → pick a sequence (or drop
your own `.mid`) → choose which part goes to T1–T4 → **Write to FM-1**. *Save the 4 tracks* reads your FM-1 tracks and
downloads a `.mid`.

A MIDI file holds **notes, not sound**. For the "exact" sound, SUMMON the style the file is named after (or pick the same
preset) and write the file over it. Melodies of copyrighted songs are not included; the pack brings public-domain
classics and classic break *rhythms* programmed from scratch. Any `.mid` you own you can load yourself.

<p align="center"><img src="docs/images/midi_ritual.png" width="520" alt="MIDI Ritual"></p>

## Build the firmware yourself

See [`firmware-src/BUILDING.md`](firmware-src/BUILDING.md) (JieLi toolchain + three AC79 SDK files). Then:

```sh
cd firmware-src
./build.sh --release 1.8.2-dfd4     # -> build/felucca-1.8.2-dfd4.fwsc (identity FM-1_918)
sh tests/run_tests.sh               # host tests: UI renders, audio goldens, tape echo, fuzzers
```

## Credits and licences

- **Dark Funk Durcal Synt** — created at **Polaris Sound Lab**, Mérida, Yucatán, México.
- Based on **Felucca 1.8.2** by Hügelton Instruments — GPL-3.0, no warranty (see `firmware-src/LICENSE`,
  `firmware-src/LICENSING.md`). Felucca-Engines (MIT), felucca-bsp (MPL-2.0), FM6 engine from Dexed / msfa (Apache-2.0),
  Inter Tight font (SIL OFL 1.1), samples VSCO-2 CE and VCSL (CC0).
- M-VAVE and FM-1 are trademarks of their respective owners. This firmware is not an official M-VAVE product.
  Install it at your own risk, following the manual.

---

## Español

**Dark Funk Durcal Synt** convierte el FM-1 en una groovebox oscura y fácil para líneas acid, breaks troceados, jungle,
drum'n'bass, braindance y rave, sin saber teoría musical: todas las notas generadas siempre están en la escala.

Es una versión de [Felucca 1.8.2](https://github.com/hugelton/Felucca) (GPL-3.0). **Conserva todos los sonidos y
funciones de Felucca** y agrega:

- **RITUAL:** 2 de 11 estilos, FUSION, CHAOS y **SUMMON** llena las 4 pistas.
- **MELODY:** sentimiento, tonalidad, forma y densidad → **MAKE**. Siempre suena bien.
- **DICE:** dados por parte (batería, acid, melodía, sonido) o **ALL**. Mantén SAVE para deshacer.
- **STRETCH:** time-stretch granular amarrado al tempo, con FREEZE y tono ±12.
- **TAPE ECHO (nuevo):** eco de cinta de tres cabezas con WOW (ondulación y saturación). Se llega tocando **FX** 6 veces.
- **El ojo vivo (nuevo):** tranquilo en ambient, furioso (ceño, venas, iris rojo, temblor) cuando la música es intensa.
  Funciona también con MENU › LARGE.
- **Sonidos:** 10 presets acid nuevos y un banco FM de 32 voces (SLOT B1–B32).
- **MIDI Ritual (nuevo):** carga archivos `.mid` (o los 50 del pack) a las pistas del FM-1 y guarda tus pistas como `.mid`.

**Archivos:** el instalador está en `install/`, los manuales en PDF en `manual/` (bilingüe y solo español), la groovebox
web y MIDI Ritual en `web/`, el pack MIDI en `midi/` (estilos, clásicos de dominio público y ritmos de breaks
clásicos), el banco DX7 en `sounds/` y el código fuente completo en `firmware-src/`.

**Instalar:** cable USB-C de datos directo a la Mac, cierra las apps MIDI, abre `install/DarkFunkDurcal-Installer.html`
en Chrome, pulsa **Install**, permite el MIDI y no desconectes mientras dice *Writing*. El manual trae cada paso con
imágenes.

**¿Qué es Dark Funk Durcal?** Búscalo en **Spotify, YouTube y todas las plataformas de audio**: su música está disponible
en cualquier plataforma digital.

**Creado en Polaris Sound Lab · Mérida, Yucatán, México** ·
[Instagram @polaris.sound.lab](https://www.instagram.com/polaris.sound.lab/) ·
[YouTube @PolarisSoundLab](https://www.youtube.com/@PolarisSoundLab)
