/* SPDX-License-Identifier: GPL-3.0-only
 * Dark Funk Durcal Synt (Polaris Sound Lab), on Felucca 1.8.2 (Hügelton Instruments).
 *
 * HOME's pages after MIXER and CLOCK (params.c PAGES, FAM_TRK: HOME taps go round them):
 *   RITUAL   STYLE A, STYLE B, FUSION, CHAOS; OCT+ SUMMON: the four tracks' sounds, patterns, key and tempo at once
 *            (T1 the drums or a chopped break, T2 the acid, T3 the bass or a second acid, T4 the melody or chords)
 *   MELODY   FEEL (the scale), KEY, SHAPE, NOTES; OCT+ MAKE: a melody that always fits into the selected track (T1: T4)
 *   DICE     each KNOB turned right rolls a die: K1 the drums, K2 the acid, K3 the melody, K4 a sound; OCT+ ROLL ALL
 *   STRETCH  SOURCE, STRETCH, PITCH, GRAIN; OCT+ APPLY: the selected track becomes a GRAIN time-stretch of a sample (a
 *            set or USR1..3: your own recordings), the scan locked to the bars (LFO SAW, SYNC, FREE), pitch on its own
 *   LIVE     (dfd6) MUTES, REPEAT, BUILD, THROW; OCT+ DROP: everything off at once
 *   NEXT     (dfd6) PHRASE ORIGINAL / ANSWER / VARIATION / TENSION of T2..T4, heard at once; K4 new ones; OCT+ KEEP
 * The eye: on these pages and on HOME. Integer maths, bounded loops; writes happen in the main loop (the UI), the way
 * a pattern or a preset load does them (load_begin / load_end: one undo). */

/* ------------------------------------------------------------------ randomness --- */
static uint32_t dfd_seed = 0x9E3779B9u;
static uint32_t dfd_rnd(void)
{
    uint32_t x = dfd_seed ^ fm1_ms;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    dfd_seed = x ? x : 0x2545F491u;
    return dfd_seed;
}
static uint32_t dfd_pick(uint32_t n) { return n ? (dfd_rnd() >> 8) % n : 0u; }
static int dfd_chance(uint32_t pct) { return (dfd_rnd() >> 8) % 100u < pct; }

/* ------------------------------------------------------------------ the styles --- */
enum { DK_GRID, DK_BREAK };                           /* T1: the DRUM engine's grid, or SLICE chopping the BREAK */
enum { MOOD_DARK, MOOD_ACID, MOOD_FUNKY, MOOD_MELAN, MOOD_DREAMY, MOOD_ALIEN, MOOD_RAVE, MOOD_HYPNO, MOOD_N };
static const char *const NN_NAME[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
static const char *const MOOD_NAME[MOOD_N] = {"DARK", "ACID", "FUNKY", "MELAN", "DREAMY", "ALIEN", "RAVE", "HYPNO"};
static const uint8_t MOOD_SCALE[MOOD_N] = {8, 6, 3, 2, 9, 13, 2, 6};   /* N_SCALE: PHRY MPEN DOR MIN LYD WHOLE MIN MPEN */
static const int8_t MOOD_PROG[MOOD_N][4] = {{0, 1, 0, 6}, {0, 0, 2, 3}, {0, 3, 0, 4}, {0, 5, 2, 6},
                                            {0, 1, 4, 3}, {0, 2, 4, 1}, {0, 5, 6, 0}, {0, 0, 0, 3}};
typedef struct {
    const char *name, *full;
    uint8_t bpm_lo, bpm_hi, mood, kind, tpl, kit;    /* kit: DRUM preset (grid) or SLICE preset (break) */
    uint8_t acid;                                    /* ANALOG preset of T2 */
    uint8_t b_eng, b_pre, b_slot;                    /* T3: engine, preset, FM6 bank voice + 1 (0: none) */
    uint8_t m_eng, m_pre, m_slot, m_chords;          /* T4: the same; 1: chords, not a melody */
    uint8_t acid_d, lead_d, rolls, swing;            /* densities and rolls in %, the track SWING */
} dfd_style_t;
/* ANALOG presets: 4 ACID 6 RAVE 7 SUB BASS 1 SOFT PAD 11 STRINGS; DFD's 12 DFD ACID 13 SQUELCH 14 BLACK ACID 15 RUBBER
 * ACID 16 SERPENT 17 ACID LASER 18 ACID 2 19 REESE 20 DETUNE PAD 21 BRAIN LEAD. FM6 (12) bank voices B1..B32: dfd_bank.h.
 * PHYS (9): 1 MARIMBA 4 KALIMBA 8 HARP. VOICE (5): 0 CHOIR AAH 1 VOX LEAD 3 WHISPER. LOFI (3): 2 ARP 8BIT */
#define TPL_N 11
static const dfd_style_t DFD_STYLES[] = {
    /* name     full              bpm      mood        kind      tpl kit acid  T3 eng pre slot  T4 eng pre slot ch  acid lead roll sw */
    {"ACID", "ACID", 124, 134, MOOD_ACID, DK_GRID, 0, 0, 12, 0, 7, 0, 12, 1, 10, 0, 85, 30, 10, 8},
    {"DRILL", "DRILL N BASS", 165, 180, MOOD_MELAN, DK_BREAK, 1, 0, 13, 12, 2, 4, 12, 0, 17, 0, 55, 50, 75, 4},
    {"JUNGLE", "JUNGLE", 160, 172, MOOD_DARK, DK_BREAK, 2, 0, 15, 0, 7, 0, 5, 0, 0, 1, 40, 25, 50, 10},
    {"DNB", "ROLLING DNB", 170, 176, MOOD_DARK, DK_BREAK, 3, 0, 18, 0, 19, 0, 12, 4, 23, 1, 45, 30, 35, 4},
    {"BRKCOR", "BREAKCORE", 180, 200, MOOD_ALIEN, DK_BREAK, 4, 1, 17, 0, 18, 0, 3, 2, 0, 0, 70, 40, 90, 0},
    {"BIGBT", "BIG BEAT", 128, 138, MOOD_RAVE, DK_BREAK, 5, 0, 14, 0, 14, 0, 0, 6, 0, 0, 60, 35, 25, 12},
    {"RAVE", "RAVE HARDCORE", 138, 150, MOOD_RAVE, DK_BREAK, 6, 0, 14, 0, 2, 0, 0, 6, 0, 1, 60, 55, 40, 5},
    {"BRAIN", "BRAINDANCE", 110, 140, MOOD_DREAMY, DK_GRID, 7, 3, 15, 12, 2, 1, 9, 4, 0, 0, 60, 50, 45, 5},
    {"ELECTR", "DARK ELECTRO", 126, 136, MOOD_DARK, DK_GRID, 8, 2, 16, 12, 2, 8, 12, 0, 12, 0, 65, 35, 20, 6},
    {"AMBIEN", "AMBIENT IDM", 86, 104, MOOD_DREAMY, DK_GRID, 9, 1, 16, 0, 7, 0, 0, 20, 0, 1, 40, 35, 25, 10},
    {"FUNK", "BROKEN FUNK", 98, 116, MOOD_FUNKY, DK_GRID, 10, 1, 15, 12, 2, 4, 12, 0, 18, 1, 60, 40, 20, 25},
    /* dfd6: warped funk (a talking vox lead over rubber bass, slow and swung), slap drum n bass (EP chords), a music-box
     * IDM (prepared piano, broken beats), spectral step (vox and reese) */
    {"WARPFK", "WARPED FUNK", 100, 108, MOOD_FUNKY, DK_GRID, 10, 1, 15, 12, 1, 2, 5, 1, 0, 0, 45, 45, 35, 28},
    {"SLAPDN", "SLAP DNB", 168, 178, MOOD_FUNKY, DK_BREAK, 1, 0, 13, 12, 1, 4, 12, 1, 18, 1, 45, 40, 60, 4},
    {"TOYBOX", "MUSIC BOX IDM", 112, 132, MOOD_MELAN, DK_GRID, 7, 3, 15, 12, 1, 3, 12, 1, 17, 0, 35, 60, 55, 8},
    {"SPECTR", "SPECTRAL STEP", 134, 144, MOOD_HYPNO, DK_GRID, 8, 2, 16, 0, 19, 0, 5, 1, 0, 0, 50, 35, 30, 10},
};
#define DFD_NSTYLES (sizeof DFD_STYLES / sizeof DFD_STYLES[0])
/* the drum templates (16ths): x hit, g ghost, o open hat / accent; a style's set of 3 kicks, 3 snares, 2 hats */
static const char *const DFD_TPL[TPL_N][8] = {
    {"x...x...x...x...", "x...x...x...x...", "x...x...x..xx...", "....x.......x...", "....x.......x..g", "....x..g....x...", "..x...x...x...x.", "ggxgggxgggxgggxg"},
    {"x.x.......xx....", "x...x.....x..x..", "x.x....x..x.....", "....x..x.x..x..x", "....x.x..x.xx...", "..x.x..x.x.x.xxx", "xxxxxxxxxxxxxxxx", "x.xxx.xxx.xxx.xx"},
    {"x.x.......xx....", "x.x.......x.....", "x.........x..x..", "....x..x.x..x..x", "....x..x.x.x.x..", "....x.g..gx.g.x.", "x.x.x.x.x.x.x.x.", "xgxgxgxgxgxgxgxg"},
    {"x.........x.....", "x.......x.x.....", "x.........x...x.", "....x.......x...", "....x..g....x..g", "....x.......x.g.", "..x...x...x...x.", "xgxgxgxgxgxgxgxg"},
    {"x.xx..x.x.x..x..", "xx..x..xx..x.x..", "x.x.x..xx.x..xx.", "....x.xx.xx.x.xx", "..x.x..xxx..x.xx", ".x..x.x.xx.x.x.x", "xxxxxxxxxxxxxxxx", "x.xxx.xxx.xxx.xx"},
    {"x.....x.x.......", "x.....x...x..x..", "x..x..x...x.....", "....x.......x...", "....x..x....x...", "....x.......x.g.", "x.x.x.x.x.x.x.xo", "xxxxxxxxxxxxxxxx"},
    {"x.x...x...x.....", "x...x.x...x.....", "x.x.......x.x...", "....x..x.x..x...", "....x.......x.x.", "....x..x.x..x..x", "..x...x...x...x.", "x.x.x.x.x.x.x.x."},
    {"x..x......x..x..", "x.x...x..x...x..", "x.....x.....x...", "....x..x....x.x.", "...x..x...x..x..", "....x.....x.....", "xxx.x.xxx.x.x.x.", "x.xxx..x.xx.x..x"},
    {"x.....x...x.....", "x.....x..x....x.", "x.....x...x..x..", "....x.......x...", "....x.......x..g", "....x...g...x...", "x.x.x.x.x.x.x.x.", "xxx.xxx.xxx.xxx."},
    {"x......x..x.....", "x.....x.....x...", "x..........x..x.", "....x.....x.....", ".....x......x...", "....x.......x...", "x.x..x.x.xx..x.x", "..x.x...x.x..x.."},
    {"x.........x..x..", "x..x......x.....", "x......x..x..x..", "....x..g.g..x..g", "....x.g...g.x...", ".g..x..g..g.x.g.", "x.x.x.x.x.x.x.xo", "xgxgxgxgxgxgxgxg"},
};

/* the page state (not stored: the next SUMMON makes new music anyway) */
static struct {
    uint8_t a, b, fusion, chaos;                     /* RITUAL: styles, FUSION and CHAOS 0..100 */
    uint8_t mood, root, shape, notes;                /* MELODY */
    uint8_t src, stretch, grain;                     /* STRETCH; pitch: */
    int8_t pitch;
    uint8_t last_cat;                                /* DICE: the last die (its name below) */
    const char *last_roll;
    uint32_t roll_ms, flash_ms;                      /* the eye: a roll or a summon flashes it */
    int8_t prog[4];                                  /* the chords of the last summon (scale degrees) */
} dfd = {0, 1, 30, 50, MOOD_ACID, 9, 0, 50, 6, 2, 64, 0, 0, "READY", 0, 0, {0, 0, 2, 3}};

/* ------------------------------------------------------------------ scales --- */
static uint32_t dfd_scale(uint8_t *iv)               /* the mood's scale: its intervals, how many */
{
    uint32_t m = SCALE_MASK[MOOD_SCALE[dfd.mood % MOOD_N]], i, n = 0;
    for (i = 0; i < 12u; i++)
        if ((m >> i) & 1u)
            iv[n++] = (uint8_t)i;
    return n ? n : 1u;
}
/* scale degree d (any integer) above base (a MIDI C) in the key: a note that always fits */
static uint32_t dfd_note(int32_t d, int32_t base)
{
    uint8_t iv[12];
    int32_t n = (int32_t)dfd_scale(iv), o = d >= 0 ? d / n : -((-d + n - 1) / n), r = d - o * n, v;
    v = base + (int32_t)dfd.root + 12 * o + iv[r];
    return (uint32_t)(v < 24 ? v + 12 * ((24 - v + 11) / 12) : v > 108 ? v - 12 * ((v - 108 + 11) / 12) : v);
}
static void dfd_key(track_t *t)                      /* the track's keys and its notes in the key (SCL) */
{
    t->p[P_ROOT] = (int16_t)(dfd.root % 12u);
    t->p[P_SCALE] = MOOD_SCALE[dfd.mood % MOOD_N];
    t->p[P_QUANT] = 1;                               /* SNAP: the keys play the scale */
}

/* ------------------------------------------------------------------ sounds --- */
static void dfd_sound(track_t *t, uint32_t eng, uint32_t pre, uint32_t slot)
{
    set_engine_of(t, eng);
    apply_preset_to(t, pre);
    if (eng == ENGI_FM6 && slot)                     /* a bank voice B1..B32 (fm6_poll loads it) */
        t->p[P_E7] = (int16_t)(FM6_OWN + slot);
}
static const dfd_style_t *dfd_side(void)             /* FUSION: style A or B for this part */
{
    return &DFD_STYLES[(dfd_pick(100) < dfd.fusion ? dfd.b : dfd.a) % DFD_NSTYLES];
}

/* ------------------------------------------------------------------ steps --- */
static void dfd_clear(track_t *t, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < NSTEP; i++)
        step_clear(&t->step[i]);
    t->p[P_SLEN] = (int16_t)len;
}
static void dfd_put(step_t *s, uint32_t note, uint32_t acc)
{
    s->time = ST_NOTE;
    s->note[0] = (uint8_t)note;
    s->n = 1;
    s->vel = acc ? 120 : 96;
    s->flags = (uint8_t)(acc ? SF_ACCENT : 0u);
}
static void dfd_hold(track_t *t, uint32_t from, uint32_t len)   /* steps after a note: TIEs (its length) */
{
    uint32_t k;
    for (k = 1; k < len && from + k < (uint32_t)t->p[P_SLEN] && from + k < NSTEP; k++)
        if (t->step[from + k].time != ST_NOTE)
            t->step[from + k].time = ST_TIE;
}

/* ------------------------------------------------------------------ drums --- */
#define LN_KICK 0
#define LN_SNARE 1
#define LN_CLAP 2
#define LN_CH 3
#define LN_OH 4
#define LN_TOM 5
#define LN_RIM 6
#define LN_BELL 7
static void dfd_lane(track_t *t, const char *s, uint32_t lane, uint32_t ghost_lane)
{
    uint32_t k;
    for (k = 0; k < 16u && s[k]; k++) {
        step_t *st = &t->step[k];
        uint32_t l = s[k] == 'g' ? ghost_lane : s[k] == 'o' && lane == LN_CH ? LN_OH : lane;
        if (s[k] == '.')
            continue;
        st->time = ST_NOTE;
        st->hit |= (uint8_t)(1u << l);
        if (s[k] == 'o' || (s[k] == 'x' && lane == LN_SNARE && dfd_chance(15)))
            st->acc |= (uint8_t)(1u << l);
    }
}
static void dfd_grid(track_t *t, const dfd_style_t *ks, const dfd_style_t *ss, const dfd_style_t *hs)
{
    uint32_t k, c = dfd.chaos, rolls = (ks->rolls + ss->rolls) / 2u;
    dfd_clear(t, 16);
    dfd_lane(t, DFD_TPL[ks->tpl][dfd_pick(3)], LN_KICK, LN_KICK);
    dfd_lane(t, DFD_TPL[ss->tpl][3 + dfd_pick(3)], LN_SNARE, LN_RIM);
    dfd_lane(t, DFD_TPL[hs->tpl][6 + dfd_pick(2)], LN_CH, LN_CH);
    for (k = 0; k < 16u; k++) {
        step_t *s = &t->step[k];
        if (!(s->hit & 1u) && (k & 3u) && dfd_chance(c / 8u)) { s->time = ST_NOTE; s->hit |= 1u << LN_KICK; }
        if (!s->hit && dfd_chance(c / 5u)) { s->time = ST_NOTE; s->hit |= 1u << LN_RIM; }
        if ((s->hit & (1u << LN_SNARE)) && dfd_chance(rolls * (30u + c) / 200u))
            step_set_ratchet(s, 2u + dfd_pick(3));
        if (s->hit == (1u << LN_CH) && dfd_chance(c / 4u))
            step_set_chance(s, 50u + 25u * dfd_pick(2));
        if (dfd_chance(c / 6u)) { s->time = ST_NOTE; s->hit |= (uint8_t)(1u << (dfd_chance(50) ? LN_TOM : LN_BELL)); }
    }
    if (rolls > 40u && dfd_chance(rolls))            /* a fill into the next bar */
        for (k = 13; k < 16u; k++)
            if (dfd_chance(60)) { t->step[k].time = ST_NOTE; t->step[k].hit |= 1u << LN_SNARE; step_set_ratchet(&t->step[k], 2u + dfd_pick(3)); }
}
/* the BREAK (SLICE, DIV 16: note 60 + k plays slice k): its kicks 0 2 7 10, snares 4 12, ghosts 9 14, hats 6 8 */
static const uint8_t BRK_K[] = {0, 2, 7, 10}, BRK_S[] = {4, 12}, BRK_G[] = {9, 14}, BRK_H[] = {6, 8, 3, 1};
static uint32_t dfd_slice_for(uint32_t k, uint32_t c)
{
    if (!dfd_chance(15u + c * 3u / 4u))
        return k;                                    /* the break as it is */
    if (k == 4u || k == 12u) return BRK_S[dfd_pick(2)];
    if (!(k & 3u)) return BRK_K[dfd_pick(4)];
    switch (dfd_pick(4)) {
    case 0: return BRK_G[dfd_pick(2)];
    case 1: return BRK_H[dfd_pick(4)];
    case 2: return BRK_S[dfd_pick(2)];
    default: return dfd_pick(16);
    }
}
static void dfd_break(track_t *t, const dfd_style_t *st)
{
    uint32_t k, c = dfd.chaos, len = c > 60u ? 32u : 16u, prev = 0;
    dfd_clear(t, len);
    t->p[P_ROOT] = 0;                                /* (a slice is note 60 + k: no key, no snap) */
    t->p[P_SCALE] = 0;
    t->p[P_QUANT] = 0;
    for (k = 0; k < len; k++) {
        step_t *s = &t->step[k];
        uint32_t sl = (k >= 16u && t->step[k - 16u].time == ST_NOTE && dfd_chance(55)) ? (uint32_t)(t->step[k - 16u].note[0] - 60u) & 15u
                                                                                      : dfd_slice_for(k & 15u, c);
        if ((k & 3u) && dfd_chance(c / 5u))
            sl = prev;                               /* a stutter: the slice again */
        if (k && (k & 1u) && dfd_chance(c / 7u))
            continue;                                /* a gap */
        dfd_put(s, 60u + (sl & 15u), sl == 4u || sl == 12u);
        if ((sl == 4u || sl == 12u || sl == 9u || sl == 14u) && dfd_chance(st->rolls * (25u + c) / 150u))
            step_set_ratchet(s, 2u + dfd_pick(3));   /* the snare rolls */
        if (k >= len - 3u && st->rolls > 40u && dfd_chance(st->rolls / 2u))
            step_set_ratchet(s, 3u + dfd_pick(2));   /* the fill */
        prev = sl;
    }
}
static void dfd_drums(track_t *t)
{
    const dfd_style_t *a = &DFD_STYLES[dfd.a % DFD_NSTYLES], *b = &DFD_STYLES[dfd.b % DFD_NSTYLES];
    const dfd_style_t *st = dfd_pick(100) < dfd.fusion ? b : a;
    load_begin(t, UNDO_SOUND | UNDO_PAT);
    if (st->kind == DK_BREAK) {
        dfd_sound(t, 13u, st->kit, 0);               /* SLICE CHOP / STUTTER: the BREAK */
        dfd_break(t, st);
    } else {
        dfd_sound(t, ENGI_DRUM, st->kit, 0);
        dfd_grid(t, dfd_side(), dfd_side(), dfd_side());
    }
    t->p[P_SSWING] = (int16_t)st->swing;
    load_end(t);
}

/* ------------------------------------------------------------------ acid, bass, chords, melody --- */
static void dfd_acid(track_t *t, uint32_t second)
{
    uint8_t iv[12];
    int32_t n = (int32_t)dfd_scale(iv);
    const int8_t pool[10] = {0, 0, 0, (int8_t)(n - 1), 2, 4, 1, 0, (int8_t)n, 3};
    uint32_t k, c = dfd.chaos, base = second ? 48u : 36u;
    uint32_t dens = (DFD_STYLES[dfd.a % DFD_NSTYLES].acid_d * (100u - dfd.fusion) + DFD_STYLES[dfd.b % DFD_NSTYLES].acid_d * dfd.fusion) / 100u;
    if (second)
        dens = dens * 7u / 10u;
    dfd_clear(t, 16);
    for (k = 0; k < 16u; k++) {
        step_t *s = &t->step[k];
        int32_t d;
        if (k && !dfd_chance(dens * (80u + c * 3u / 10u) / 100u))
            continue;
        d = pool[dfd_pick(10)] + (dfd_chance(22) ? n : 0);
        dfd_put(s, dfd_note(d, (int32_t)base), dfd_chance(28));
        if (dfd_chance(20u + c / 10u))
            s->flags |= SF_SLIDE;
        if (dfd_chance(c / 12u))
            step_set_ratchet(s, 2);
    }
    for (k = 0; k < 16u; k++)                         /* a slide only into a note */
        if ((t->step[k].flags & SF_SLIDE) && t->step[(k + 1u) & 15u].time != ST_NOTE)
            t->step[k].flags &= (uint8_t)~SF_SLIDE;
    dfd_key(t);
}
static void dfd_bass(track_t *t)
{
    uint32_t k;
    dfd_clear(t, 16);
    for (k = 0; k < 16u; k++) {
        int32_t d = dfd.prog[k / 4u];
        int strong = !(k & 3u), syn = (k & 3u) >= 2u && dfd_chance(25u + dfd.chaos * 3u / 10u);
        if (strong || syn || ((trk[0].step[k].hit & 1u) && dfd_chance(50))) {
            dfd_put(&t->step[k], dfd_note(d + (dfd_chance(15) ? 4 : 0), 36) + (dfd_chance(12) ? 12u : 0u), strong && dfd_chance(30));
            if (strong)
                dfd_hold(t, k, 2u + dfd_pick(2));
        }
    }
    dfd_key(t);
}
static void dfd_chords(track_t *t)
{
    uint32_t k, seg, len = dfd.chaos < 35u || dfd.mood == MOOD_DREAMY || dfd.mood == MOOD_MELAN ? 32u : 16u;
    dfd_clear(t, len);
    seg = len / 4u;
    for (k = 0; k < 4u; k++) {
        step_t *s = &t->step[k * seg];
        int32_t d = dfd.prog[k];
        s->time = ST_NOTE;
        s->note[0] = (uint8_t)dfd_note(d, 48);
        s->note[1] = (uint8_t)dfd_note(d + 2, 48);
        s->note[2] = (uint8_t)dfd_note(d + 4, 48);
        s->n = 3;
        if (dfd.chaos > 45u) { s->note[3] = (uint8_t)dfd_note(d + 6, 48); s->n = 4; }
        s->vel = 90;
        dfd_hold(t, k * seg, seg);
    }
    t->p[P_VOICE] = V_POLY;
    dfd_key(t);
}
enum { SH_WAVE, SH_RISE, SH_FALL, SH_QA, SH_ARP, SH_HOOK, SH_FEW, SH_BUSY, SH_N };
static const char *const SHAPE_NAME[SH_N] = {"WAVE", "RISE", "FALL", "Q&A", "ARP", "HOOK", "FEW", "BUSY"};
static int32_t dfd_snap(int32_t d, uint32_t k)       /* the nearest note of the chord playing at step k */
{
    int32_t cd = dfd.prog[(k & 15u) / 4u], best = d, bd = 999, o, i;
    uint8_t iv[12];
    int32_t n = (int32_t)dfd_scale(iv);
    for (o = -2; o <= 3; o++)
        for (i = 0; i < 3; i++) {
            int32_t y = cd + 2 * i + o * n, e = y > d ? y - d : d - y;
            if (e < bd) { bd = e; best = y; }
        }
    return best;
}
static void dfd_melody(track_t *t, uint32_t shape, uint32_t notes, int32_t base)
{
    uint8_t iv[12], on[32], hook[8];
    int32_t n = (int32_t)dfd_scale(iv), span = n + 3, prev = n / 2, d;
    uint32_t k, j, cnt = 0, nh = 0, len = shape == SH_QA || (dfd.chaos > 65u && shape != SH_HOOK) ? 32u : 16u;
    uint32_t dens = shape == SH_FEW ? 16u : shape == SH_BUSY ? 90u : 20u + notes * 6u / 10u;
    dfd_clear(t, len);
    if (shape == SH_HOOK) {                           /* a 2-beat motif, said again (and again) */
        for (k = 0; k < 8u; k++)
            if (!k || dfd_chance(dens * (k & 3u ? (k & 1u ? 55u : 100u) : 150u) / 100u))
                hook[nh++] = (uint8_t)k;
        for (j = 0; j < len; j += 8u)
            for (k = 0; k < nh; k++)
                on[cnt++] = (uint8_t)(j + hook[k]);
    } else {
        for (k = 0; k < len; k++)
            if (!k || dfd_chance(dens * (k & 3u ? (k & 1u ? 55u : 100u) : 150u) / 100u))
                on[cnt++] = (uint8_t)k;
    }
    for (j = 0; j < cnt; j++) {
        uint32_t s = on[j];
        int32_t u = cnt > 1u ? (int32_t)(j * 256u / (cnt - 1u)) : 0;
        switch (shape) {
        case SH_RISE: d = u * span / 256; break;
        case SH_FALL: d = (256 - u) * span / 256; break;
        case SH_WAVE: d = span / 2 + (int32_t)DFD_S((uint32_t)(u * (int32_t)len / 16)) * span / 2 / 16384; break;
        case SH_QA: {
            int32_t half = (int32_t)len / 2, uu = s < (uint32_t)half ? (int32_t)s * 256 / half : ((int32_t)s - half) * 256 / half;
            d = s < (uint32_t)half ? uu * span * 85 / 25600 : (256 - uu) * span * 85 / 25600;
            break;
        }
        case SH_ARP: { static const int8_t A[6] = {0, 2, 4, 7, 4, 2}; d = A[j % 6u] + dfd.prog[(s & 15u) / 4u]; break; }
        case SH_HOOK: d = prev + (int32_t)dfd_pick(5) - 2; break;
        default: d = prev + (int32_t)dfd_pick(5) - 2; break;
        }
        if (shape == SH_HOOK && s >= 8u) {           /* the motif's note again (its MIDI note), a turn at the end */
            uint32_t note = t->step[on[j % nh]].note[0];
            if (s >= len - 4u && dfd_chance(60))
                note = dfd_note(dfd_snap(prev + (int32_t)dfd_pick(3) - 1, s), base);
            dfd_put(&t->step[s], note, !(s & 7u) && dfd_chance(50));
            continue;
        }
        if (shape != SH_ARP && dfd_chance(40))
            d += dfd_chance(50) ? 1 : -1;
        if (!(s & 3u) && shape != SH_ARP)
            d = dfd_snap(d, s);
        d = d < -1 ? -1 : d > 2 * n + 2 ? 2 * n + 2 : d;
        if (j == cnt - 1u) {                         /* the end: the chord's root, the nearest one */
            int32_t cd = dfd.prog[(s & 15u) / 4u], o, best = cd, bd = 999;
            for (o = -1; o <= 3; o++) {
                int32_t y = cd + o * n, e = y > d ? y - d : d - y;
                if (e < bd) { bd = e; best = y; }
            }
            d = best;
        }
        if (shape == SH_QA && s < len / 2u && j + 1u < cnt && on[j + 1u] >= len / 2u)
            d = 4;                                   /* the question ends open (the fifth) */
        prev = d;
        dfd_put(&t->step[s], dfd_note(d, base), !(s & 7u) && dfd_chance(50));
    }
    for (j = 0; j < cnt; j++)                         /* lengths: to the next note (BUSY: short) */
        dfd_hold(t, on[j], (j + 1u < cnt ? (uint32_t)on[j + 1u] - on[j] : len - on[j]) > (shape == SH_BUSY ? 2u : 4u) ?
                               (shape == SH_BUSY ? 2u : 4u) : (j + 1u < cnt ? (uint32_t)on[j + 1u] - on[j] : len - on[j]));
    dfd_key(t);
}

/* ------------------------------------------------------------------ SUMMON --- */
static void dfd_summon(void)
{
    const dfd_style_t *a = &DFD_STYLES[dfd.a % DFD_NSTYLES], *b = &DFD_STYLES[dfd.b % DFD_NSTYLES], *st;
    uint32_t k, lo = (a->bpm_lo * (100u - dfd.fusion) + b->bpm_lo * dfd.fusion) / 100u;
    uint32_t hi = (a->bpm_hi * (100u - dfd.fusion) + b->bpm_hi * dfd.fusion) / 100u;
    track_t *t;
    dfd.mood = (dfd.fusion > 50u ? b : a)->mood;
    for (k = 0; k < 4u; k++)
        dfd.prog[k] = MOOD_PROG[dfd.mood][k];
    if (dfd_chance(40)) {                            /* another order of the mood's chords now and then */
        int8_t x = dfd.prog[1];
        dfd.prog[1] = dfd.prog[3];
        dfd.prog[3] = x;
    }
    song.g[G_BPM] = (int16_t)clamp((int32_t)(lo + dfd_pick(hi - lo + 1u)), 40, 240);
    dfd_drums(&trk[0]);
    t = &trk[1];                                     /* T2: the acid */
    st = dfd_side();
    load_begin(t, UNDO_SOUND | UNDO_PAT);
    dfd_sound(t, 0, st->acid, 0);
    dfd_acid(t, 0);
    load_end(t);
    t = &trk[2];                                     /* T3: the bass, or a second acid */
    st = dfd_side();
    load_begin(t, UNDO_SOUND | UNDO_PAT);
    dfd_sound(t, st->b_eng, st->b_pre, st->b_slot);
    if (st->b_eng == 0u && st->b_pre >= 12u && st->b_pre <= 18u)
        dfd_acid(t, 1);
    else
        dfd_bass(t);
    load_end(t);
    t = &trk[3];                                     /* T4: the melody or the chords */
    st = dfd_side();
    load_begin(t, UNDO_SOUND | UNDO_PAT);
    dfd_sound(t, st->m_eng, st->m_pre, st->m_slot);
    if (st->m_chords)
        dfd_chords(t);
    else
        dfd_melody(t, dfd_pick(SH_N - 1u), (a->lead_d * (100u - dfd.fusion) + b->lead_d * dfd.fusion) / 100u + 20u, 60);
    load_end(t);
    dfd.flash_ms = fm1_ms;
    ui.force = 1;
    ui_say("SUMMONED ", a->name);
}

/* ------------------------------------------------------------------ DICE --- */
static void dfd_shuffle_quarters(track_t *t)
{
    step_t q[16];
    uint32_t order[4] = {0, 1, 2, 3}, k, i;
    for (k = 3; k > 1u; k--) { uint32_t j = 1u + dfd_pick(k), x = order[k]; order[k] = order[j]; order[j] = x; }
    memcpy(q, t->step, sizeof q);
    for (k = 0; k < 4u; k++)
        for (i = 0; i < 4u; i++)
            t->step[k * 4u + i] = q[order[k] * 4u + i];
}
static void dfd_roll(uint32_t cat)
{
    track_t *t;
    uint32_t k, r = dfd_pick(5);
    static const char *const DN[5] = {"GROOVE", "BREAK CHOP", "ROLLS", "GHOSTS", "HATS"};
    static const char *const AN[5] = {"NEW LINE", "NOTES", "ACCENTS", "SLIDES", "OCTAVES"};
    static const char *const MN[5] = {"NEW SHAPE", "NOTES", "RHYTHM", "ENDING", "CHORDS"};
    static const char *const SN[5] = {"DRUM KIT", "ACID SOUND", "BASS SOUND", "LEAD SOUND", "ALL SOUNDS"};
    if (fm1_ms - dfd.roll_ms < 120u)                 /* one die per gesture's detents */
        return;
    dfd.roll_ms = fm1_ms;
    dfd.last_cat = (uint8_t)cat;
    if (cat == 0u) {                                 /* the drums: T1 */
        t = &trk[0];
        load_begin(t, UNDO_PAT);
        if (r == 0u) {
            load_end(t);
            dfd_drums(t);
        } else {
            for (k = 0; k < (uint32_t)t->p[P_SLEN] && k < NSTEP; k++) {
                step_t *s = &t->step[k];
                if (r == 1u && k == 0u) {
                    if (ENGINES[t->eng_req % NENGINES] == &ENG_SLICE) {
                        uint32_t i;
                        for (i = 0; i < 16u; i++)
                            if (t->step[i].time == ST_NOTE && dfd_chance(45))
                                t->step[i].note[0] = (uint8_t)(60u + dfd_slice_for(i, 100));
                    } else {
                        dfd_shuffle_quarters(t);
                    }
                }
                if (r == 2u && s->time == ST_NOTE)
                    step_set_ratchet(s, dfd_chance(28) ? 2u + dfd_pick(3) : 1u);
                if (r == 3u && drum_track(t) && dfd_chance(25)) { s->time = ST_NOTE; s->hit ^= 1u << LN_RIM; }
                if (r == 3u && !drum_track(t) && s->time == ST_NOTE && dfd_chance(20)) s->note[0] = (uint8_t)(60u + BRK_G[dfd_pick(2)]);
                if (r == 4u && drum_track(t)) {
                    s->hit &= (uint8_t)~(1u << LN_CH);
                    if (dfd_chance(55 + dfd.chaos / 3u)) { s->time = ST_NOTE; s->hit |= 1u << LN_CH; step_set_chance(s, dfd_chance(30) ? 60u : 100u); }
                }
            }
            load_end(t);
        }
        dfd.last_roll = DN[r];
    } else if (cat == 1u) {                          /* the acid: T2 (and the second acid on T3) */
        t = &trk[1];
        load_begin(t, UNDO_PAT);
        if (r == 0u) {
            dfd_acid(t, 0);
        } else {
            for (k = 0; k < 16u; k++) {
                step_t *s = &t->step[k];
                if (s->time != ST_NOTE) continue;
                if (r == 1u) s->note[0] = (uint8_t)dfd_note((int32_t)dfd_pick(5) - (dfd_chance(30) ? 1 : 0), 36) + (dfd_chance(22) ? 12u : 0u);
                if (r == 2u) { s->flags &= (uint8_t)~SF_ACCENT; s->vel = 96; if (dfd_chance(30)) { s->flags |= SF_ACCENT; s->vel = 120; } }
                if (r == 3u) { s->flags &= (uint8_t)~SF_SLIDE; if (t->step[(k + 1u) & 15u].time == ST_NOTE && dfd_chance(30)) s->flags |= SF_SLIDE; }
                if (r == 4u) s->note[0] = (uint8_t)(36u + dfd.root + ((s->note[0] - 36u - dfd.root) % 12u) + (dfd_chance(28) ? 12u : 0u));
            }
        }
        load_end(t);
        dfd.last_roll = AN[r];
    } else if (cat == 2u) {                          /* the melody: T4 */
        t = &trk[3];
        load_begin(t, UNDO_PAT);
        if (r == 4u) {
            dfd.prog[1] = (int8_t)dfd_pick(7);
            dfd.prog[2] = (int8_t)dfd_pick(7);
            dfd_chords(t);
        } else if (r == 0u) {
            dfd.shape = (uint8_t)dfd_pick(SH_N);
            dfd_melody(t, dfd.shape, dfd.notes, 60);
        } else {
            uint32_t len = (uint32_t)t->p[P_SLEN];
            int32_t prev = 4;
            for (k = 0; k < len && k < NSTEP; k++) {
                step_t *s = &t->step[k];
                if (r == 1u && s->time == ST_NOTE && s->n == 1u) {
                    prev = dfd_snap(prev + (int32_t)dfd_pick(5) - 2, k);
                    s->note[0] = (uint8_t)dfd_note(prev, 60);
                }
                if (r == 2u && (k & 1u) && dfd_chance(30)) {   /* a note moves half a beat */
                    step_t x = t->step[k];
                    t->step[k] = t->step[k - 1u];
                    t->step[k - 1u] = x;
                }
                if (r == 3u && k >= len - 4u && s->time == ST_NOTE)
                    s->note[0] = (uint8_t)dfd_note(dfd_snap((int32_t)dfd_pick(4), k), 60);
            }
        }
        load_end(t);
        dfd.last_roll = MN[r];
    } else {                                         /* a sound */
        static const uint8_t ACIDS[] = {4, 12, 13, 14, 15, 16, 17, 18};
        static const uint8_t LEADS[][3] = {{12, 0, 17}, {12, 0, 10}, {12, 0, 18}, {12, 0, 32}, {9, 4, 0}, {9, 1, 0},
                                           {5, 1, 0}, {5, 0, 0}, {0, 21, 0}, {0, 6, 0}, {3, 2, 0}, {12, 0, 12}};
        static const uint8_t BASSES[][3] = {{12, 2, 4}, {12, 2, 1}, {12, 2, 2}, {12, 2, 8}, {0, 7, 0}, {0, 19, 0}, {0, 2, 0}};
        if (r == 0u || r == 4u) {
            t = &trk[0];
            load_begin(t, UNDO_SOUND);
            apply_preset_to(t, dfd_pick(ENGINES[t->eng_req % NENGINES]->npresets));
            load_end(t);
        }
        if (r == 1u || r == 4u) {
            t = &trk[1];
            load_begin(t, UNDO_SOUND);
            dfd_sound(t, 0, ACIDS[dfd_pick(sizeof ACIDS)], 0);
            load_end(t);
        }
        if (r == 2u || r == 4u) {
            const uint8_t *b = BASSES[dfd_pick(sizeof BASSES / 3u)];
            t = &trk[2];
            load_begin(t, UNDO_SOUND);
            dfd_sound(t, b[0], b[1], b[2]);
            load_end(t);
        }
        if (r == 3u || r == 4u) {
            const uint8_t *l = LEADS[dfd_pick(sizeof LEADS / 3u)];
            t = &trk[3];
            load_begin(t, UNDO_SOUND);
            dfd_sound(t, l[0], l[1], l[2]);
            load_end(t);
        }
        dfd.last_roll = SN[r];
    }
    dfd.flash_ms = fm1_ms;
    ui.force = 1;
}

/* ------------------------------------------------------------------ STRETCH --- */
static const char *const STR_NAME[5] = {"FREEZE", "1 BAR", "2 BARS", "4 BARS", "1/2 BAR"};
static const uint8_t STR_SYNC[5] = {0, 3, 2, 1, 4};   /* N_LSYNC: OFF 4BAR 2BAR 1/1 1/2 */
static const uint8_t STR_LEN[5] = {16, 16, 32, 64, 16};
static void dfd_stretch_set(track_t *t)              /* the page's values into a GRAIN track (live) */
{
    t->p[P_E0] = (int16_t)(dfd.src % SMP_NALL);
    t->p[P_E2] = (int16_t)dfd.grain;
    t->p[P_E4] = (int16_t)dfd.pitch;
    t->p[P_LSYNC] = STR_SYNC[dfd.stretch % 5u];
    t->p[P_LD_SHP] = dfd.stretch % 5u ? 63 : 0;      /* FREEZE: the position stands */
}
static void dfd_stretch_apply(void)
{
    track_t *t = TSEL;
    uint32_t k, len = STR_LEN[dfd.stretch % 5u];
    load_begin(t, UNDO_SOUND | UNDO_PAT);
    set_engine_of(t, 8u);                            /* GRAIN */
    apply_preset_to(t, 2);                           /* FROZEN: a long, smooth sound */
    t->p[P_E1] = 64;                                 /* POS: the centre; the LFO (BI) scans the whole sample */
    t->p[P_E3] = 110;                                /* DENS */
    t->p[P_E5] = 6;                                  /* SPRD: a narrow spread, the scan is heard */
    t->p[P_E6] = 4;                                  /* RAND */
    t->p[P_E7] = 127;                                /* TONE */
    t->p[P_LWAVE] = 2;                               /* SAW */
    t->p[P_LPOL] = 0;                                /* BI */
    t->p[P_LTRIG] = 1;                               /* FREE: with SYNC, in time with the bars from PLAY */
    t->p[P_LPHASE] = 0;
    t->p[P_LFADE] = 0;
    t->p[P_ATK] = 2;
    t->p[P_SUS] = 127;
    dfd_stretch_set(t);
    dfd_clear(t, len);                               /* one note held: the scan runs under it */
    dfd_put(&t->step[0], 60, 0);
    for (k = 1; k < len; k++)
        t->step[k].time = ST_TIE;
    load_end(t);
    ui.force = 1;
    ui_say("STRETCH ", STR_NAME[dfd.stretch % 5u]);
}

/* ------------------------------------------------------------------ the pages' knobs and OCT+ --- */
static int dfd_page(uint32_t g) { return g >= GR_DFD_RITUAL && g <= GR_DFD_NEXT; }

/* ------------------------------------------------------------------ LIVE (dfd6) --- */
/* K1 MUTES: a set of the tracks out (perform.c's black-key mutes: ramped, never saved), K2 REPEAT OFF 1/8 1/16 1/32
 * (the FX layer's REPEAT, from the next 1/16), K3 BUILD: the master HPF climbing (the FX macro FILTER, right), K4 THROW
 * OFF ECHO REVERB BOTH; OCT+ DROP: all of it off at once (the drop after a build). The FX layer works beside it. */
static const char *const LV_MUTE[8] = {"ALL IN", "NO DRUMS", "NO ACID", "NO BASS", "NO LEAD", "DRUMS ONLY", "DRUM+BASS", "LEAD ONLY"};
static const uint8_t LV_MBITS[8] = {0, 1, 2, 4, 8, 14, 10, 7};
static const char *const LV_REP[4] = {"OFF", "1/8", "1/16", "1/32"};
static const char *const LV_THR[4] = {"OFF", "ECHO", "VERB", "BOTH"};
static const char *const LV_MCOL[8] = {"ALL", "NO DR", "NO AC", "NO BS", "NO LD", "DRUMS", "DR+BS", "LEAD"};
static struct { uint8_t mutes, rep, build, thr; uint32_t drop_ms; } lv;
static void dfd_live_apply(void)
{
    uint32_t b = (uint32_t)LV_MBITS[lv.mutes & 7u] << PF_M1;
    if (lv.rep)
        b |= PF_BIT(PF_R8 + lv.rep - 1u);
    if (lv.thr & 1u) b |= PF_BIT(PF_ETHR);
    if (lv.thr & 2u) b |= PF_BIT(PF_RTHR);
    if (lv.rep && !((b >> (PF_R8 + lv.rep - 1u)) & perf_avail() & 1u))
        ui_message("TOO SLOW FOR 1/8");
    dfd_live_fx = b;
    perf_k[0] = (int8_t)lv.build;
}
static void dfd_drop(void)
{
    lv.mutes = lv.rep = lv.build = lv.thr = 0;
    dfd_live_apply();
    lv.drop_ms = fm1_ms;
    dfd.flash_ms = fm1_ms;
    ui_say("DROP ", "");
}

/* ------------------------------------------------------------------ NEXT (dfd6) --- */
/* Three continuations of a track's phrase that always fit: 1 ANSWER (the line mirrored around its middle, home at the
 * end), 2 VARIATION (some notes a step up or down), 3 TENSION (the second half climbing, ending open). K1 PHRASE
 * ORIGINAL / 1 / 2 / 3 (heard at once, browsing: one undo), K2 TRACK T2..T4, K4 turned: new ones; OCT+ KEEP. */
static const char *const NX_NAME[4] = {"ORIGINAL", "1 ANSWER", "2 VARIATION", "3 TENSION"};
static const char *const NX_COL[4] = {"ORIG", "ANSWR", "VARI", "TENSE"};
static struct { uint8_t on, pick, tgt, seed, len; step_t orig[NSTEP]; } nx = {0, 0, 3, 1, 16, {{{0}}}};
static uint32_t nx_r;
static uint32_t nx_rnd(void) { nx_r = nx_r * 1664525u + 1013904223u; return nx_r >> 9; }
static int32_t dfd_deg(uint32_t note)                 /* the scale degree (above C3 in the key) nearest a note */
{
    int32_t d, best = 0, bd = 999;
    for (d = -28; d <= 42; d++) {
        int32_t e = (int32_t)dfd_note(d, 48) - (int32_t)note;
        e = e < 0 ? -e : e;
        if (e < bd) { bd = e; best = d; }
    }
    return best;
}
static void nx_apply(void)
{
    track_t *t = &trk[nx.tgt % NTRK];
    uint32_t i, cnt = 0, last = 0, len = nx.len ? nx.len : 16u;
    int32_t sum = 0, mid, n;
    uint8_t iv[12];
    n = (int32_t)dfd_scale(iv);
    load_begin(t, UNDO_PAT);
    memcpy(t->step, nx.orig, sizeof nx.orig);
    t->p[P_SLEN] = (int16_t)len;
    for (i = 0; i < len && i < NSTEP; i++)
        if (nx.orig[i].time == ST_NOTE && nx.orig[i].n) { sum += dfd_deg(nx.orig[i].note[0]); cnt++; last = i; }
    if (nx.pick && cnt) {
        mid = sum / (int32_t)cnt;
        nx_r = 0x51ED27u + nx.seed * 7919u + nx.pick * 104729u;
        for (i = 0; i < len && i < NSTEP; i++) {
            step_t *s = &t->step[i];
            int32_t d, nd, delta, k;
            if (s->time != ST_NOTE || !s->n)
                continue;
            d = dfd_deg(s->note[0]);
            if (nx.pick == 1u)
                nd = 2 * mid - d + (int32_t)(nx_rnd() % 3u) - 1;
            else if (nx.pick == 2u)
                nd = nx_rnd() % 100u < 35u ? d + (nx_rnd() & 1u ? 1 : -1) * (int32_t)(1u + nx_rnd() % 2u) : d;
            else
                nd = i >= len / 2u ? d + (int32_t)(i - len / 2u) / 3 + 1 : d;
            if (i == last)                           /* the end: home (ANSWER, VARIATION) or open, a fifth up (TENSION) */
                nd = nx.pick == 3u ? (mid / n) * n + 4 + n : (nd + n * 8) / n * n - n * 8 + ((nd % n + n) % n > n / 2 ? n : 0);
            delta = (int32_t)dfd_note(nd, 48) - (int32_t)s->note[0];
            for (k = 0; k < s->n && k < 4; k++)
                s->note[k] = (uint8_t)clamp((int32_t)s->note[k] + delta, 24, 108);
        }
    }
    load_end(t);
}
static void nx_capture(void)
{
    track_t *t = &trk[nx.tgt % NTRK];
    memcpy(nx.orig, t->step, sizeof nx.orig);
    nx.len = (uint8_t)clamp(t->p[P_SLEN], 1, NSTEP);
    nx.on = 1;
}
static int32_t dfd_step(int32_t v, int32_t d, int32_t lo, int32_t hi) { v += d; return v < lo ? lo : v > hi ? hi : v; }
static void dfd_knob(uint32_t g, uint32_t slot, int32_t steps)
{
    if (g == GR_DFD_RITUAL) {
        if (slot == 0u) dfd.a = (uint8_t)dfd_step(dfd.a, steps > 0 ? 1 : -1, 0, DFD_NSTYLES - 1);
        if (slot == 1u) dfd.b = (uint8_t)dfd_step(dfd.b, steps > 0 ? 1 : -1, 0, DFD_NSTYLES - 1);
        if (slot == 2u) dfd.fusion = (uint8_t)dfd_step(dfd.fusion, steps * 2, 0, 100);
        if (slot == 3u) dfd.chaos = (uint8_t)dfd_step(dfd.chaos, steps * 2, 0, 100);
    } else if (g == GR_DFD_MELODY) {
        if (slot == 0u) dfd.mood = (uint8_t)dfd_step(dfd.mood, steps > 0 ? 1 : -1, 0, MOOD_N - 1);
        if (slot == 1u) dfd.root = (uint8_t)((dfd.root + 12u + (steps > 0 ? 1u : 11u)) % 12u);
        if (slot == 2u) dfd.shape = (uint8_t)dfd_step(dfd.shape, steps > 0 ? 1 : -1, 0, SH_N - 1);
        if (slot == 3u) dfd.notes = (uint8_t)dfd_step(dfd.notes, steps * 2, 0, 100);
    } else if (g == GR_DFD_DICE) {
        if (steps > 0)
            dfd_roll(slot);
    } else if (g == GR_DFD_LIVE) {
        if (slot == 0u) lv.mutes = (uint8_t)dfd_step(lv.mutes, steps > 0 ? 1 : -1, 0, 7);
        if (slot == 1u) lv.rep = (uint8_t)dfd_step(lv.rep, steps > 0 ? 1 : -1, 0, 3);
        if (slot == 2u) lv.build = (uint8_t)dfd_step(lv.build, steps * 3, 0, 100);
        if (slot == 3u) lv.thr = (uint8_t)dfd_step(lv.thr, steps > 0 ? 1 : -1, 0, 3);
        dfd_live_apply();
    } else if (g == GR_DFD_NEXT) {
        if (chain_busy()) { ui_message("STOP TO EDIT"); return; }
        if (slot == 1u) {
            if (nx.on && nx.pick) { nx.pick = 0; nx_apply(); }
            nx.on = 0;
            nx.tgt = (uint8_t)dfd_step(nx.tgt, steps > 0 ? 1 : -1, 1, 3);
        } else if (slot == 0u || (slot == 3u && steps > 0)) {
            if (!nx.on)
                nx_capture();
            if (slot == 0u)
                nx.pick = (uint8_t)dfd_step(nx.pick, steps > 0 ? 1 : -1, 0, 3);
            else
                nx.seed++;
            nx_apply();
            dfd.flash_ms = fm1_ms;
        }
    } else {
        if (slot == 0u) dfd.src = (uint8_t)dfd_step(dfd.src, steps > 0 ? 1 : -1, 0, SMP_NALL - 1);
        if (slot == 1u) dfd.stretch = (uint8_t)dfd_step(dfd.stretch, steps > 0 ? 1 : -1, 0, 4);
        if (slot == 2u) dfd.pitch = (int8_t)dfd_step(dfd.pitch, steps, -12, 12);
        if (slot == 3u) dfd.grain = (uint8_t)dfd_step(dfd.grain, steps * 2, 0, 127);
        if (ENGINES[TSEL->eng_req % NENGINES] == &ENG_GRAIN)
            dfd_stretch_set(TSEL);                   /* a GRAIN track: live */
    }
    ui.force = 1;
}
static void dfd_oct(void)
{
    uint32_t g = cur_page()->graph;
    if (chain_busy() && g != GR_DFD_LIVE) { ui_message("STOP TO EDIT"); return; }
    if (g == GR_DFD_RITUAL) {
        dfd_summon();
    } else if (g == GR_DFD_MELODY) {
        track_t *t = song.sel ? TSEL : &trk[3];
        load_begin(t, UNDO_PAT);
        dfd_melody(t, dfd.shape, dfd.notes, ENGINES[t->eng_req % NENGINES] == &ENG_ANALOG && t->p[P_VOICE] ? 36 : 60);
        load_end(t);
        dfd.flash_ms = fm1_ms;
        ui_say("MELODY ", SHAPE_NAME[dfd.shape % SH_N]);
    } else if (g == GR_DFD_DICE) {                   /* ROLL ALL: new styles, then a summon */
        dfd.a = (uint8_t)dfd_pick(DFD_NSTYLES);
        dfd.b = (uint8_t)dfd_pick(DFD_NSTYLES);
        dfd.fusion = (uint8_t)(20u + dfd_pick(50));
        dfd.chaos = (uint8_t)(30u + dfd_pick(60));
        dfd.root = (uint8_t)dfd_pick(12);
        dfd_summon();
        dfd.last_roll = "EVERYTHING";
    } else if (g == GR_DFD_LIVE) {
        dfd_drop();
    } else if (g == GR_DFD_NEXT) {
        nx.on = 0;                                   /* KEEP: what plays stays; the next turn starts from it */
        nx.pick = 0;
        dfd.flash_ms = fm1_ms;
        ui_say("KEPT", "");
    } else {
        dfd_stretch_apply();
    }
    ui.force = 1;
}
static const char *dfd_act_name(void)
{
    uint32_t g = cur_page()->graph;
    return g == GR_DFD_RITUAL ? "SUMMON" : g == GR_DFD_MELODY ? "MAKE" : g == GR_DFD_DICE ? "ALL" : g == GR_DFD_LIVE ? "DROP" :
           g == GR_DFD_NEXT ? "KEEP" : "APPLY";
}

/* ------------------------------------------------------------------ the eye --- */
static int32_t dfd_level(void)                        /* 0..256: how loud it plays (the eye's pupil) */
{
    int32_t m = 0, i;
    for (i = 0; i < NTRK; i++)
        m = trk[i].peak > m ? trk[i].peak : m;
    m >>= 7;
    return song.playing ? (m > 256 ? 256 : m) : 0;
}
/* the eye's mood (dfd v4): 0 calm (ambient, slow, quiet) .. 256 furious (fast, dense, loud, distorted) */
static int32_t dfd_ins, dfd_ins_tgt;
static uint32_t dfd_ins_ms;
static int32_t dfd_ins_force = -1;                  /* (the host renders: a mood held) */
static int32_t dfd_intensity(void)
{
    uint32_t t = fm1_ms;
    if (dfd_ins_force >= 0) return dfd_ins_force;
    if (t - dfd_ins_ms >= 120u || dfd_ins_ms == 0u) {   /* the target: a few times a second */
        int32_t bpm = clamp(song.g[G_BPM], 40, 240), tempo = clamp((bpm - 95) * 256 / 85, 0, 256);
        int32_t on = 0, drive = 0, res = 0, i, k, dens, lv = dfd_level();
        for (i = 0; i < NTRK; i++) {
            const track_t *tk = &trk[i];
            if (tk->p[P_MUTE]) continue;
            for (k = 0; k < 16 && k < NSTEP; k++)
                on += tk->step[k].time == ST_NOTE;
            drive = tk->p[P_DIST] > drive ? tk->p[P_DIST] : drive;
        }
        res = TSEL->p[P_E5];                         /* the selected track's resonance (ANALOG) */
        dens = clamp(on * 256 / 40, 0, 256);
        drive = clamp(drive * 2, 0, 256);
        res = clamp(res * 2, 0, 256);
        dfd_ins_tgt = song.playing ? (lv * 30 + tempo * 28 + dens * 20 + drive * 14 + res * 8) / 100 : 24;
        dfd_ins_ms = t ? t : 1u;
    }
    dfd_ins += (dfd_ins_tgt - dfd_ins) / 6 + (dfd_ins_tgt > dfd_ins ? 1 : dfd_ins_tgt < dfd_ins ? -1 : 0);
    return clamp(dfd_ins, 0, 256);
}
static void dfd_eye(int32_t cx, int32_t cy, int32_t r, int32_t open, int32_t lx, int32_t ly, int32_t pupil, int32_t red,
                    int32_t ang)
{
    uint16_t bone = T_TEXT, bg = cv_bg, faint = ux_mix(cv_bg, T_TEXT, 35), blood = T_REC;
    int32_t W = r * 62 / 100, H = 1 + r * 36 / 100 * open / 256, x, y, ir = r * 24 / 100, pr = pupil, th = r > 30 ? 2 : 1;
    int32_t ix = cx + lx, iy = cy + ly, k;
    uint32_t a = (fm1_ms / (uint32_t)(60 - ang * 52 / 256)) & 255u;   /* the ring turns faster when furious */
    dfd_disc(cx, cy, r, bg);
    dfd_ring(cx, cy, r + r / 7 + 1, 256, 1, (int32_t)a, th, ang > 200 ? blood : bone);
    dfd_ring(cx, cy, r, 256, 0, 0, 1, faint);
    if (ang > 150) {                                 /* spikes on the outer ring, trembling */
        int32_t n = 8, len = r / 8 + (ang - 150) * r / 640;
        for (k = 0; k < n; k++) {
            int32_t q = k * 32 + (int32_t)a * 2, j = (int32_t)((fm1_ms / 40u + (uint32_t)k * 7u) % 3u);
            int32_t r0 = r + r / 7 + 3, r1 = r0 + len + j;
            dfd_line(cx + r0 * DFD_C(q) / 16384, cy + r0 * DFD_S(q) / 16384, cx + r1 * DFD_C(q) / 16384,
                     cy + r1 * DFD_S(q) / 16384, 1, ang > 200 ? blood : bone);
        }
    }
    {
        int32_t P[4][2] = {{cx, cy - r + 1}, {cx + r * 87 / 100, cy + r / 2}, {cx - r * 87 / 100, cy + r / 2}, {cx, cy - r + 1}}, s2;
        for (s2 = 0; s2 < 3; s2++)
            dfd_line(P[s2][0], P[s2][1], P[s2 + 1][0], P[s2 + 1][1], th, bone);
    }
    for (y = -ir; y <= ir; y++)
        for (x = -ir; x <= ir; x++) {
            int32_t d2 = x * x + y * y, ax = ix + x - cx, lim;
            if (d2 > ir * ir || ax < -W || ax > W) continue;
            lim = H * (W * W - ax * ax) / (W * W);
            if (y + iy - cy > lim || y + iy - cy < -lim) continue;
            cv_pset(ix + x, iy + y, d2 > (ir - 1) * (ir - 1) ? bone : d2 <= pr * pr ? (d2 <= (pr / 3) * (pr / 3) ? bg : bone)
                                                                   : (red ? blood : bg));
        }
    if (ang > 120 && H > 3) {                        /* veins from the corners */
        int32_t v = (ang - 120) * 4 / 136 + 1;
        for (k = 0; k < v; k++) {
            int32_t sx = k & 1 ? 1 : -1, yy = (k - v / 2) * H / (v + 1);
            int32_t x0 = cx + sx * (W - 1), x1 = cx + sx * (W - W / 3 - (k % 3)), y1 = cy + yy + ((k * 5) % 3) - 1;
            dfd_line(x0, cy + yy / 3, x1, y1, 1, blood);
        }
    }
    for (x = -W; x < W; x += 2) {
        int32_t y0 = H * (W * W - x * x) / (W * W), y1 = H * (W * W - (x + 2) * (x + 2)) / (W * W);
        dfd_line(cx + x, cy - y0, cx + x + 2, cy - y1, th, bone);
        dfd_line(cx + x, cy + y0, cx + x + 2, cy + y1, th, bone);
    }
    if (ang > 140) {                                 /* the brow: slanting down to the middle (anger) */
        int32_t lift = H + r / 8 + 2, drop = (ang - 140) * r / 700 + 1;
        dfd_line(cx - W, cy - lift - drop, cx - W / 4, cy - lift + drop, th, ang > 200 ? blood : bone);
        dfd_line(cx + W, cy - lift - drop, cx + W / 4, cy - lift + drop, th, ang > 200 ? blood : bone);
    } else if (ang < 70) {                           /* calm: a soft lid line, slow */
        dfd_line(cx - W * 2 / 3, cy - H - 2, cx + W * 2 / 3, cy - H - 2, 1, faint);
    }
}
/* SMEAR (dfd6; dfd8: in the eye's original colours): a pass over the canvas pixels the eye just drew (its box):
 * columns of pixels dragged down (a pixel sort) and a row or two torn sideways (more on a summon, a roll, a furious mood, a loud part). Integer maths, bounded loops */
static int dfd_grey(uint16_t c)
{
    int32_t r = ((c >> 11) & 31) * 2, g = (c >> 5) & 63, b = (c & 31) * 2;
    return (r > g ? r - g : g - r) <= 4 && (b > g ? b - g : g - b) <= 4;
}
static int32_t dfd_lum(uint16_t c) { return ((c >> 11) & 31) * 2 + ((c >> 5) & 63) + (c & 31) * 2; }   /* 0..187 */
static void dfd_holo(int32_t cx, int32_t cy, int32_t r, int32_t tear)
{
    int32_t x0 = cx - r * 3 / 2, x1 = cx + r * 3 / 2, y0 = cy - r * 5 / 4 + cv_oy, y1 = cy + r * 5 / 4 + cv_oy, x;
    int32_t lb = dfd_lum(cv_bg), sp = r > 0 ? 1536 / r : 64, drift = DFD_S(fm1_ms / 40u) * 90 / 16384;
    uint32_t t = fm1_ms / 6u, z = (fm1_ms / 50u) * 2654435761u;
    if (ux.mono || (ux.pal[0] == 0x0000u && ux.pal[2] == 0xFFFFu) || (dfd_grey(ux.text) && dfd_grey(ux.theme)))   /* GREY, MONO: no colour */
        return;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > (int32_t)cv_w) x1 = (int32_t)cv_w;
    if (y1 > (int32_t)cv_h) y1 = (int32_t)cv_h;
    if (x1 - x0 < 2 || y1 - y0 < 2)
        return;
    (void)lb; (void)sp; (void)drift; (void)t;           /* (the original colours: no recolouring, the drips and tears only) */
    {                                                /* the drips: a column's pixel dragged down, fading */
        uint32_t k, n = 2u + (uint32_t)tear / 12u, q = (fm1_ms / 70u) * 2246822519u + 1u;
        for (k = 0; k < n; k++) {
            int32_t px = x0 + (int32_t)((q >> 7) % (uint32_t)(x1 - x0)), py = y0 + (int32_t)((q >> 15) % (uint32_t)((y1 - y0) * 3 / 4 + 1));
            int32_t len = 3 + (int32_t)((q >> 23) % (uint32_t)(r / 2 + 2 + tear / 8)), j;
            uint16_t src = swap16(cv_px[(uint32_t)py * cv_w + (uint32_t)px]);
            q = q * 1664525u + 1013904223u;
            if (src == cv_bg)
                continue;
            for (j = 1; j < len && py + j < y1; j++) {
                uint16_t *p = &cv_px[(uint32_t)(py + j) * cv_w + (uint32_t)px];
                *p = swap16(ux_mix(swap16(*p), src, 90 - j * 80 / len));
            }
        }
    }
    if (tear > 0) {                                  /* the slices: rows of the box slid sideways */
        uint32_t k, n = 1u + (uint32_t)tear / 48u;
        for (k = 0; k < n; k++) {
            int32_t h = 1 + (int32_t)((z >> 3) % 4u), ry = y0 + (int32_t)((z >> 9) % (uint32_t)(y1 - y0)), d = (int32_t)((z >> 17) % 9u) - 4, j;
            d = d * (1 + tear / 64);
            z = z * 1664525u + 1013904223u;
            if (!d) d = 2;
            for (j = 0; j < h && ry + j < y1; j++) {
                uint16_t *row = &cv_px[(uint32_t)(ry + j) * cv_w];
                if (d > 0) for (x = x1 - 1; x >= x0 + d; x--) row[x] = row[x - d];
                else for (x = x0; x < x1 + d; x++) row[x] = row[x - d];
            }
        }
        if ((z >> 5) & 1u) {                         /* a bar of foil */
            int32_t bw = r / 2 + (int32_t)((z >> 11) % (uint32_t)(r + 1)), by = cy - r + (int32_t)((z >> 19) % (uint32_t)(2 * r + 1));
            cv_rect(cx - r * 3 / 2 + (int32_t)((z >> 7) % (uint32_t)(3 * r - bw + 1)), by, bw, 1 + (int32_t)((z >> 23) & 1u) * 2, (z >> 9) & 1u ? T_TEXT : T_REC);
        }
    }
}
static void dfd_eye_auto(int32_t cx, int32_t cy, int32_t r)   /* blinking, looking around, flashing on a roll; its mood */
{
    uint32_t t = fm1_ms;
    int32_t ang = dfd_intensity(), open = 256, lv = dfd_level(), jx = 0, jy = 0, sp, pupil;
    int32_t fl = t - dfd.flash_ms < 600u ? (int32_t)(600u - (t - dfd.flash_ms)) * 256 / 600 : 0;
    uint32_t blink = ang < 70 ? 7900u : ang > 180 ? 2600u : 5300u;
    if ((t % blink) < 140u)
        open = 20;                                   /* a blink */
    if (ang < 70)
        open = open * (150 + ang) / 256;             /* calm: heavy, half-closed lids */
    else if (ang > 170)
        open = open * (256 - (ang - 170)) / 256;     /* furious: narrowed */
    if (!song.playing && fl == 0)
        open = open * 2 / 3;
    if (ang > 190) {                                 /* trembling */
        uint32_t z = (t / 33u) * 2654435761u;
        int32_t amp = 1 + (ang - 190) * r / 1400;
        jx = (int32_t)((z >> 13) % (uint32_t)(2 * amp + 1)) - amp;
        jy = (int32_t)((z >> 21) % (uint32_t)(2 * amp + 1)) - amp;
    }
    sp = ang < 70 ? 40 : ang > 170 ? 7 : 16;         /* the gaze: slow drift when calm, darting when furious */
    pupil = r / 10 + lv * r / 1500 + fl * r / 1800;
    if (ang > 170) pupil = pupil * 2 / 3;            /* pinned pupil */
    if (ang < 70) pupil += r / 20;                   /* wide, dreamy */
    dfd_eye(cx + jx, cy + jy, r, open, DFD_S(t / (uint32_t)sp) * (r / 6) / 16384 + jx,
            DFD_S(t / (uint32_t)(sp + 7) + 64u) * (r / 14) / 16384, pupil, fl > 60 || lv > 200 || ang > 200, ang);
    dfd_holo(cx, cy, r, fl / 2 + (ang > 190 ? (ang - 190) * 2 : 0) + (lv > 220 ? 40 : 0) + ((t % 3100u) < 90u ? 60 : 0));
}
static void dfd_home_eye(void) { dfd_eye_auto(212, 30, 20); }

/* ------------------------------------------------------------------ the graph strip of the pages --- */
static void dfd_bar(int32_t x, int32_t y, int32_t w, uint32_t pct)
{
    cv_rect(x, y, w, 4, ux_mix(cv_bg, T_TEXT, 20));
    cv_rect(x, y, w * (int32_t)pct / 100, 4, T_THEME);
}
static void dfd_graph(uint32_t g)
{
    int32_t h = (int32_t)graph_h(), small = h < 90, r = small ? 22 : 34, cx = small ? 34 : 50, cy = h / 2, tx = small ? 70 : 102;
    char b[24];
    dfd_eye_auto(cx, cy, r);
    if (g == GR_DFD_RITUAL) {
        const dfd_style_t *a = &DFD_STYLES[dfd.a % DFD_NSTYLES], *bb = &DFD_STYLES[dfd.b % DFD_NSTYLES];
        cv_text_on(tx, small ? 6 : 14, &AF_M, a->full, T_TEXT, cv_bg);
        cv_text_on(tx, small ? 22 : 36, &AF_S, "X", T_MID, cv_bg);
        cv_text_on(tx, small ? 34 : 54, &AF_M, bb->full, T_TEXT, cv_bg);
        if (!small) {
            dfd_bar(tx, 78, 120, dfd.fusion);
            cv_text_on(tx, 88, &AF_S, "[OCT+] SUMMON", T_MID, cv_bg);
        }
    } else if (g == GR_DFD_MELODY) {
        str_cpy(b, MOOD_NAME[dfd.mood % MOOD_N], sizeof b);
        str_cpy(b + str_len(b), " IN ", 5);
        str_cpy(b + str_len(b), NN_NAME[dfd.root % 12u], 4);
        cv_text_on(tx, small ? 6 : 14, &AF_M, b, T_TEXT, cv_bg);
        cv_text_on(tx, small ? 26 : 40, &AF_M, SHAPE_NAME[dfd.shape % SH_N], T_THEME, cv_bg);
        if (!small) {                                /* the shape, drawn */
            int32_t k, px = tx, py = 0;
            for (k = 0; k <= 8; k++) {
                int32_t u = k * 32, y;
                switch (dfd.shape) {
                case SH_RISE: y = 26 - u * 24 / 256; break;
                case SH_FALL: y = 2 + u * 24 / 256; break;
                case SH_QA: y = k <= 4 ? 26 - k * 5 : 6 + (k - 4) * 5; break;
                case SH_ARP: y = 26 - (k % 4) * 7; break;
                case SH_FEW: y = 14 + (k & 1) * 4; break;
                case SH_BUSY: y = 14 + (int32_t)((k * 7) % 5) * 3 - 6; break;
                case SH_HOOK: y = 14 + ((k % 4) == 1 ? -8 : (k % 4) == 3 ? 6 : 0); break;
                default: y = 14 + DFD_S((uint32_t)u) * 11 / 16384; break;
                }
                y += 62;
                if (k) dfd_line(px, py, tx + k * 14, y, 2, T_THEME);
                dfd_dot(tx + k * 14, y, 4, T_TEXT);
                px = tx + k * 14;
                py = y;
            }
            cv_text_on(tx, 98, &AF_S, song.sel ? "[OCT+] THIS TRACK" : "[OCT+] INTO T4", T_MID, cv_bg);
        }
    } else if (g == GR_DFD_DICE) {
        static const char *const CN[4] = {"K1 DRUMS", "K2 ACID", "K3 MELODY", "K4 SOUND"};
        uint32_t age = fm1_ms - dfd.roll_ms, face = (dfd.roll_ms / 7u) % 6u, k;
        int32_t dx = tx + 6, dy = small ? 8 : 12, ds = small ? 24 : 34;
        static const int8_t PIP[6][6][2] = {{{0, 0}}, {{-1, -1}, {1, 1}}, {{-1, -1}, {0, 0}, {1, 1}},
                                           {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}}, {{-1, -1}, {1, -1}, {0, 0}, {-1, 1}, {1, 1}},
                                           {{-1, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {1, 1}}};
        if (age < 300u)
            dx += (int32_t)((fm1_ms / 16u) & 3u) * 2 - 3;
        cv_rrect(dx, dy, ds, ds, 6, T_TEXT, cv_bg);
        for (k = 0; k <= face; k++)
            dfd_disc(dx + ds / 2 + PIP[face][k][0] * ds * 3 / 10, dy + ds / 2 + PIP[face][k][1] * ds * 3 / 10, ds / 12, cv_bg);
        cv_text_on(dx + ds + 8, dy + 2, &AF_S, CN[dfd.last_cat % 4u], T_MID, cv_bg);
        cv_text_on(tx, dy + ds + 6, &AF_M, dfd.last_roll, T_TEXT, cv_bg);
        if (!small)
            cv_text_on(tx, 88, &AF_S, "TURN = ROLL", T_MID, cv_bg);
    } else if (g == GR_DFD_LIVE) {
        uint32_t age = fm1_ms - lv.drop_ms;
        cv_text_on(tx, small ? 6 : 14, &AF_M, LV_MUTE[lv.mutes & 7u], lv.mutes ? T_ACCENT : T_TEXT, cv_bg);
        str_cpy(b, "REPEAT ", sizeof b);
        str_cpy(b + str_len(b), LV_REP[lv.rep & 3u], 6);
        cv_text_on(tx, small ? 26 : 40, &AF_M, lv.rep ? b : lv.thr ? LV_THR[lv.thr & 3u] : "PLAY IT", T_THEME, cv_bg);
        if (!small) {
            uint32_t k;
            for (k = 0; k < 4u; k++)                 /* the four tracks: lit in, dark out */
                cv_rrect(tx + (int32_t)k * 22, 62, 18, 10, 3, (LV_MBITS[lv.mutes & 7u] >> k) & 1u ? ux_mix(cv_bg, T_TEXT, 20) : T_TEXT, cv_bg);
            dfd_bar(tx + 92, 65, 36, lv.build);
            cv_text_on(tx, 88, &AF_S, age < 900u ? "* DROP *" : "[OCT+] DROP", age < 900u ? T_ACCENT : T_MID, cv_bg);
        }
    } else if (g == GR_DFD_NEXT) {
        const track_t *t = &trk[nx.tgt % NTRK];
        str_cpy(b, "T", sizeof b);
        b[1] = (char)('1' + nx.tgt % NTRK);
        b[2] = 0;
        str_cpy(b + 2, nx.on ? "  TRYING" : "  READY", 10);
        cv_text_on(tx, small ? 6 : 14, &AF_M, NX_NAME[nx.pick & 3u], nx.pick ? T_ACCENT : T_TEXT, cv_bg);
        cv_text_on(tx, small ? 26 : 40, &AF_S, b, T_MID, cv_bg);
        if (!small) {                                /* the phrase: its first 16 steps, a dot a note */
            uint32_t k, lo = 127, hi = 0;
            for (k = 0; k < 16u; k++)
                if (t->step[k].time == ST_NOTE && t->step[k].n) {
                    if (t->step[k].note[0] < lo) lo = t->step[k].note[0];
                    if (t->step[k].note[0] > hi) hi = t->step[k].note[0];
                }
            for (k = 0; k < 16u; k++) {
                int32_t x = tx + (int32_t)k * 8;
                cv_rect(x, 80, 6, 1, ux_mix(cv_bg, T_TEXT, 18));
                if (t->step[k].time == ST_NOTE && t->step[k].n)
                    dfd_disc(x + 3, 76 - (hi > lo ? (int32_t)(t->step[k].note[0] - lo) * 22 / (int32_t)(hi - lo) : 11), 3, T_TEXT);
                else if (t->step[k].time == ST_TIE)
                    cv_rect(x, 66, 6, 2, T_MID);
            }
            cv_text_on(tx, 88, &AF_S, "[OCT+] KEEP", T_MID, cv_bg);
        }
    } else {
        cv_text_on(tx, small ? 6 : 14, &AF_M, SMP_ALL_NAMES[dfd.src % SMP_NALL], T_TEXT, cv_bg);
        str_cpy(b, STR_NAME[dfd.stretch % 5u], sizeof b);
        cv_text_on(tx, small ? 26 : 40, &AF_M, b, T_THEME, cv_bg);
        if (!small) {                                /* the scan: a playhead over the bar of the sample */
            uint32_t bars = dfd.stretch % 5u == 0u ? 0u : dfd.stretch == 1u ? 1u : dfd.stretch == 2u ? 2u : dfd.stretch == 3u ? 4u : 1u;
            uint32_t ms = (uint32_t)(60000 / clamp(song.g[G_BPM], 40, 240)) * 4u * (bars ? bars : 1u), pos;
            pos = bars && song.playing ? (fm1_ms % ms) * 120u / ms : 60u;
            dfd_bar(tx, 70, 120, 100);
            cv_rect(tx + (int32_t)pos - 1, 64, 3, 16, T_ACCENT);
            cv_text_on(tx, 88, &AF_S, "[OCT+] APPLY", T_MID, cv_bg);
        }
    }
}
/* the four knobs' cards */
static void dfd_columns(uint32_t g)
{
    char v[12];
    if (g == GR_DFD_RITUAL) {
        draw_column(0, "STYLE A", DFD_STYLES[dfd.a % DFD_NSTYLES].name, "", VAL(0u), (int32_t)dfd.a * 1000 / (int32_t)(DFD_NSTYLES - 1), ICON_AUTO);
        draw_column(1, "STYLE B", DFD_STYLES[dfd.b % DFD_NSTYLES].name, "", VAL(1u), (int32_t)dfd.b * 1000 / (int32_t)(DFD_NSTYLES - 1), ICON_AUTO);
        fmt_int(v, dfd.fusion);
        draw_column(2, "FUSION", v, "%", VAL(2u), dfd.fusion * 10, ICON_AUTO);
        fmt_int(v, dfd.chaos);
        draw_column(3, "CHAOS", v, "%", VAL(3u), dfd.chaos * 10, ICON_AUTO);
    } else if (g == GR_DFD_MELODY) {
        draw_column(0, "FEEL", MOOD_NAME[dfd.mood % MOOD_N], "", VAL(0u), (int32_t)dfd.mood * 1000 / (MOOD_N - 1), ICON_AUTO);
        draw_column(1, "KEY", NN_NAME[dfd.root % 12u], "", VAL(1u), (int32_t)dfd.root * 1000 / 11, ICON_AUTO);
        draw_column(2, "SHAPE", SHAPE_NAME[dfd.shape % SH_N], "", VAL(2u), (int32_t)dfd.shape * 1000 / (SH_N - 1), ICON_AUTO);
        fmt_int(v, dfd.notes);
        draw_column(3, "NOTES", v, "%", VAL(3u), dfd.notes * 10, ICON_AUTO);
    } else if (g == GR_DFD_DICE) {
        draw_column(0, "DRUMS", "ROLL", "", VAL(0u), -1, ICON_AUTO);
        draw_column(1, "ACID", "ROLL", "", VAL(1u), -1, ICON_AUTO);
        draw_column(2, "MELODY", "ROLL", "", VAL(2u), -1, ICON_AUTO);
        draw_column(3, "SOUND", "ROLL", "", VAL(3u), -1, ICON_AUTO);
    } else if (g == GR_DFD_LIVE) {
        draw_column(0, "MUTES", LV_MCOL[lv.mutes & 7u], "", VAL(0u), (int32_t)lv.mutes * 1000 / 7, ICON_AUTO);
        draw_column(1, "REPEAT", LV_REP[lv.rep & 3u], "", VAL(1u), (int32_t)lv.rep * 1000 / 3, ICON_AUTO);
        fmt_int(v, lv.build);
        draw_column(2, "BUILD", v, "%", VAL(2u), lv.build * 10, ICON_AUTO);
        draw_column(3, "THROW", LV_THR[lv.thr & 3u], "", VAL(3u), (int32_t)lv.thr * 1000 / 3, ICON_AUTO);
    } else if (g == GR_DFD_NEXT) {
        static const char *const TN[4] = {"T1", "T2", "T3", "T4"};
        draw_column(0, "PHRASE", NX_COL[nx.pick & 3u], "", VAL(0u), (int32_t)nx.pick * 1000 / 3, ICON_AUTO);
        draw_column(1, "TRACK", TN[nx.tgt % NTRK], "", VAL(1u), (int32_t)(nx.tgt - 1u) * 500, ICON_AUTO);
        draw_column(2, "-", "", "", VAL(2u), -1, ICON_AUTO);
        draw_column(3, "NEW", "TURN", "", VAL(3u), -1, ICON_AUTO);
    } else {
        draw_column(0, "SOURCE", SMP_ALL_NAMES[dfd.src % SMP_NALL], "", VAL(0u), (int32_t)dfd.src * 1000 / (SMP_NALL - 1), ICON_AUTO);
        draw_column(1, "TIME", dfd.stretch ? (dfd.stretch == 4u ? "1/2" : dfd.stretch == 1u ? "1" : dfd.stretch == 2u ? "2" : "4") : "FREEZE",
                    dfd.stretch ? "BAR" : "", VAL(1u), (int32_t)dfd.stretch * 250, ICON_AUTO);
        fmt_int(v, dfd.pitch);
        draw_column(2, "PITCH", v, "st", VAL(2u), (dfd.pitch + 12) * 1000 / 24, ICON_AUTO);
        fmt_int(v, dfd.grain * 100 / 127);
        draw_column(3, "GRAIN", v, "%", VAL(3u), dfd.grain * 1000 / 127, ICON_AUTO);
    }
}
static uint32_t dfd_sig(void)
{
    return (uint32_t)dfd.a * 3u + dfd.b * 37u + dfd.fusion * 101u + dfd.chaos * 1009u + dfd.mood * 7u + dfd.root * 17u + dfd.shape * 131u +
           dfd.notes * 1301u + dfd.src * 13u + dfd.stretch * 19u + (uint32_t)(dfd.pitch + 12) * 23u + dfd.grain * 29u + dfd.roll_ms +
           lv.mutes * 41u + lv.rep * 43u + lv.build * 47u + lv.thr * 53u + lv.drop_ms + nx.pick * 59u + nx.tgt * 61u + nx.seed * 67u +
           nx.on * 71u + track_sig(&trk[nx.tgt % NTRK]);
}
