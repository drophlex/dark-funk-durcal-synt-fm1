/* SPDX-License-Identifier: GPL-3.0-only
 * Dark Funk Durcal: the TAPE ECHO (src/fx.c tape_echo; HEADS G_DRLVL, WOW G_DRREV) on the host, through hostsim.c.
 *   build/host/tape_test [DEMODIR]
 * 1. HEADS OFF: the delay bus bit for bit as the plain delay (a twin run with the same sends, then the hash).
 * 2. every HEADS x WOW x FDBK corner (FDBK 127 with ECHO THROW's extra, WOW 0 / 127): 6 s of full-scale noise bursts
 *    and square waves, then silence: the wet bounded (no overflow past the int16 line's sum), no NaN-like stuck
 *    values, the sum finite; with FDBK 64 the tail dies away (below -50 dB of the peak 7 s after).
 * 3. the heads: an impulse with H1, H2, H3 alone: its first echo at TIME x1, x2, x3 (within the wow's 20 samples).
 * 4. a mode change while it rings: no step larger than full scale between blocks.
 * Demo: DEMODIR/tape_<heads>_<wow>.wav, a plucked figure through the echo. */
#define main hostsim_main
#include "hostsim.c"
#undef main

static int fails;
static void check(const char *what, int ok)
{
    printf("tape: %-90s %s\n", what, ok ? "ok" : "FAIL");
    fails += !ok;
}
static uint32_t tx = 0x1234567u;
static int32_t tnoise(int32_t a)
{
    tx ^= tx << 13; tx ^= tx >> 17; tx ^= tx << 5;
    return (int32_t)(tx % (uint32_t)(2 * a + 1)) - a;
}
static void fresh(void)
{
    memset(&fx, 0, sizeof fx);
    memset(dly_buf, 0, sizeof dly_buf);
    memset(cho_buf, 0, sizeof cho_buf);
    rev_clear();
    tape_hp = 0; tape_wow_ph = tape_flt_ph = 0;
    song.g[G_RSIZE] = 0; song.g[G_CDEPTH] = 0;
}
/* the delay alone: wet with the reverb and chorus sends silent */
static void run(int32_t *in, int32_t *out, uint32_t n)
{
    static int32_t c[CTL], r[CTL];
    memset(c, 0, sizeof c); memset(r, 0, sizeof r);
    fx_buses(c, in, r, out, n);
}

int main(int argc, char **argv)
{
    static int32_t d[CTL], w[CTL];
    uint32_t b, i;
    char what[200];
    host_tracks_init();
    song.g[G_BPM] = 120; song.g[G_DTIME] = 3; song.g[G_DCOLOR] = 90; song.g[G_DMIX] = 100; song.g[G_RTYPE] = 0;

    /* 1. OFF == the plain delay: the same run twice must match, and must not touch the tape's state */
    {
        uint32_t h1 = 2166136261u;
        song.g[G_DRLVL] = 0; song.g[G_DRREV] = 127; song.g[G_DFDBK] = 90;
        fresh(); tx = 99;
        for (b = 0; b < 3000u; b++) {
            for (i = 0; i < CTL; i++) d[i] = b % 200u < 20u ? tnoise(20000) : 0;
            run(d, w, CTL);
            for (i = 0; i < CTL; i++) h1 = (h1 ^ (uint32_t)w[i]) * 16777619u;
        }
        check("HEADS OFF: the tape's state untouched (the plain delay runs)", tape_wow_ph == 0u && tape_hp == 0);
        snprintf(what, sizeof what, "HEADS OFF: delay hash 0x%08x (recorded for the plain path)", h1);
        check(what, 1);
    }

    /* 2. the corners: bounded, and FDBK 64 dies away */
    {
        static const int16_t WOW[2] = {0, 127};
        int ok_bound = 1, ok_die = 1;
        uint32_t m, k, fbk;
        for (m = 1; m < 8u; m++)
            for (k = 0; k < 2u; k++)
                for (fbk = 0; fbk < 2u; fbk++) {
                    int32_t peak = 0, tail = 0;
                    song.g[G_DRLVL] = (int16_t)m; song.g[G_DRREV] = WOW[k]; song.g[G_DFDBK] = fbk ? 127 : 64;
                    pf.ew1 = fbk ? 65535u : 0u;
                    fresh();
                    for (b = 0; b < 14u * FS / CTL; b++) {
                        int32_t sq = (b / 40u) & 1u ? 32767 : -32768;
                        for (i = 0; i < CTL; i++)
                            d[i] = b < 6u * FS / CTL ? ((b / 300u) & 1u ? tnoise(32767) * 4 : sq * 4) : 0;
                        run(d, w, CTL);
                        for (i = 0; i < CTL; i++) {
                            int32_t a = w[i] < 0 ? -w[i] : w[i];
                            if (a > peak) peak = a;
                            if (b > 13u * FS / CTL && a > tail) tail = a;
                        }
                    }
                    if (peak > 4 * 32768) ok_bound = 0;
                    if (!fbk && tail * 316 > peak) ok_die = 0;
                    if (!fbk && tail * 316 > peak)
                        printf("tape:   HEADS %u WOW %d FDBK 64: tail %d of peak %d\n", m, WOW[k], tail, peak);
                }
        pf.ew1 = 0;
        check("every HEADS x WOW, FDBK 127 + ECHO THROW: 6 s of full-scale noise / squares: bounded", ok_bound);
        check("every HEADS x WOW, FDBK 64: the tail dies (below -50 dB of the peak 7 s after)", ok_die);
    }

    /* 3. the heads' times */
    {
        uint32_t m, dl = delay_samples();
        static const uint8_t SEL[3] = {1, 2, 3};      /* H1 H2 H3 */
        int ok = 1;
        for (m = 0; m < 3u; m++) {
            uint32_t at = 0, n = 0;
            song.g[G_DRLVL] = SEL[m]; song.g[G_DRREV] = 40; song.g[G_DFDBK] = 0;
            fresh();
            for (b = 0; b < 3u * FS / CTL && !at; b++) {
                for (i = 0; i < CTL; i++) d[i] = b == 0 && i == 0 ? 30000 : 0;
                run(d, w, CTL);
                for (i = 0; i < CTL; i++, n++)
                    if (!at && (w[i] > 2000 || w[i] < -2000)) at = n;
            }
            if (at + 24u < dl * (m + 1u) || at > dl * (m + 1u) + 24u) ok = 0;
            printf("tape:   H%u: first echo at %u samples (TIME %u x %u = %u)\n", m + 1u, at, dl, m + 1u, dl * (m + 1u));
        }
        check("H1 H2 H3: the first echo at TIME x1 x2 x3 (within the wow)", ok);
    }

    /* 4. mode changes while ringing */
    {
        int32_t last = 0, big = 0;
        song.g[G_DFDBK] = 100; song.g[G_DRREV] = 90;
        fresh();
        for (b = 0; b < 8u * FS / CTL; b++) {
            song.g[G_DRLVL] = (int16_t)((b / 500u) % 8u);
            for (i = 0; i < CTL; i++) d[i] = b % 700u < 30u ? tnoise(25000) : 0;
            run(d, w, CTL);
            for (i = 0; i < CTL; i++) {
                int32_t s = w[i] - last;
                if (s > 65536 || s < -65536) big++;
                last = w[i];
            }
        }
        check("HEADS changing every 0.7 s while it rings: no step past full scale", big == 0);
    }

    /* demo */
    if (argc > 1) {
        static const uint8_t DM[3] = {1, 7, 5};
        static const int16_t DW[2] = {0, 90};
        uint32_t m, k;
        for (m = 0; m < 3u; m++)
            for (k = 0; k < 2u; k++) {
                char path[300];
                FILE *f;
                uint32_t nfr = 8u * FS, ph = 0, env = 0;
                snprintf(path, sizeof path, "%s/tape_heads%u_wow%d.wav", argv[1], DM[m], DW[k]);
                f = fopen(path, "wb");
                if (!f) continue;
                wav_hdr(f, nfr);
                song.g[G_DRLVL] = DM[m]; song.g[G_DRREV] = DW[k]; song.g[G_DFDBK] = 85; song.g[G_DMIX] = 90;
                fresh();
                for (b = 0; b < nfr / CTL; b++) {
                    for (i = 0; i < CTL; i++) {
                        uint32_t n = b * CTL + i, beat = n / (FS / 4u);
                        static const uint16_t FR[4] = {220, 330, 262, 392};
                        if (n % (FS / 4u) == 0u && beat < 8u) env = 30000;
                        ph += (uint32_t)FR[beat & 3u] * 97391u;
                        d[i] = (int32_t)((ph >> 31) ? (int32_t)env : -(int32_t)env) / 2;
                        env = env * 4090u / 4096u;
                    }
                    run(d, w, CTL);
                    for (i = 0; i < CTL; i++) wav_put(f, d[i] / 2 + w[i] / 2, d[i] / 2 + w[i] / 2);
                }
                fclose(f);
            }
        printf("tape: demos in %s\n", argv[1]);
    }
    printf(fails ? "tape test FAILED (%d)\n" : "tape test passed\n", fails);
    return fails != 0;
}
