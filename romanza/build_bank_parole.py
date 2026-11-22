#!/usr/bin/env python3
"""ROMANZA: genera mee_bank.h da una traccia vocale (voce sola), con frammenti tagliati sulle parole.
Uso: python3 build_bank_parole.py voce.mp3 [parole=6] [vocali=2]
Banco a 8 kHz, 8 bit mu-law, periodi x2. Restare sotto ~4 s di audio per i 48 KB dell'oscillatore."""
import sys, json, numpy as np, librosa
SR, HOP, MU, PS = 8000, 64, 255.0, 2
src = sys.argv[1]; NW = int(sys.argv[2]) if len(sys.argv) > 2 else 6; NV = int(sys.argv[3]) if len(sys.argv) > 3 else 2
y, _ = librosa.load(src, sr=SR, mono=True)
f0, _, _ = librosa.pyin(y, fmin=70, fmax=600, sr=SR, frame_length=512, hop_length=HOP)
rms = librosa.feature.rms(y=y, frame_length=256, hop_length=HOP)[0][:len(f0)]
st = 12 * np.log2(f0 / 440) + 69; fps = SR / HOP
env = np.convolve(rms, np.ones(5) / 5, 'same'); envn = env / np.percentile(env, 97)
S = np.abs(librosa.stft(y, n_fft=256, hop_length=HOP))[:, :len(f0)]
flux = np.r_[0, np.maximum(0, np.diff(S, axis=1)).sum(0)]; flux /= np.percentile(flux, 97)
cent = librosa.feature.spectral_centroid(S=S, sr=SR)[0]
N = len(env)
mins = np.array([i for i in range(3, N - 3) if env[i] <= env[i-3:i+4].min() and envn[i] < 0.35 * envn[max(0, i-40):i+40].max()])
cands = []
for a in mins:                                   # parole: tra due avvallamenti, 0,28-0,72 s, con attacchi di consonante
    for b in mins[(mins > a + int(0.28 * fps)) & (mins < a + int(0.72 * fps))]:
        seg = slice(a, b); vfrac = (~np.isnan(f0[seg])).mean()
        if vfrac < 0.45 or vfrac > 0.97 or envn[seg].max() < 0.45: continue
        edge = (envn[a] + envn[b]) / 2 / envn[seg].max()
        if edge > 0.3: continue
        sc = envn[seg].mean() * 1.5 + min((flux[seg] > 0.5).sum(), 4) * 0.25 + np.std(cent[seg]) / np.mean(cent[seg]) * 1.2 - edge * 2 - abs((b - a) / fps - 0.5) * 0.8
        cands.append((sc, int(a), int(b)))
cands.sort(reverse=True); taken = np.zeros(N, bool); words = []
for sc, a, b in cands:
    if taken[max(0, a - int(3 * fps)):b + int(3 * fps)].any(): continue
    taken[a:b] = True; words.append((a, b, 1, float(np.nanmedian(st[a:b]))))
    if len(words) >= NW: break
L = int(0.40 * fps); vc = []                     # vocali tenute: 0,4 s stabili
for a in range(0, N - L, 4):
    s = st[a:a+L]
    if np.isnan(s).any() or envn[a:a+L].min() < 0.35 or np.std(s) > 0.35: continue
    vc.append((envn[a:a+L].mean() - np.std(s), a))
vc.sort(reverse=True); vowels = []; used = []
for sc, a in vc:
    if taken[max(0, a - int(fps)):a + L + int(fps)].any(): continue
    p = np.nanmedian(st[a:a+L])
    if any(abs(p - q) < 2 for q in used): continue
    taken[a:a+L] = True; used.append(p); vowels.append((a, a + L, 0, float(p)))
    if len(vowels) >= 3: break
vowels.sort(key=lambda f: f[3])
if NV == 2 and len(vowels) == 3: vowels = [vowels[0], vowels[-1]]
frags = vowels[:NV] + sorted(words, key=lambda f: f[0])
enc = lambda x: np.round((np.sign(x) * np.log1p(MU * np.abs(np.clip(x, -1, 1))) / np.log1p(MU) + 1) * 127.5).astype(np.uint8)
lut = [np.sign(m) * ((1 + MU) ** abs(m) - 1) / MU for m in (np.arange(256) / 127.5 - 1)]
bank, per, tab, pos = [], [], [], 0
for a, b, t, p in frags:
    s = y[a*HOP:b*HOP].astype(np.float64); s -= s.mean(); s = s / np.max(np.abs(s)) * 0.92
    s[:24] *= np.linspace(0, 1, 24); s[-24:] *= np.linspace(1, 0, 24)
    q = enc(s); bank.append(q); pf = f0[a:b]
    per.append(np.where(np.isnan(pf), 0, np.clip(np.round(SR / np.nan_to_num(pf, nan=1) * PS), 0, 255)).astype(np.uint8))
    tab.append((pos, len(q), t)); pos += len(q)
bank = np.concatenate(bank); per = np.concatenate(per)
with open('mee_bank.h', 'w') as f:
    f.write('#pragma once\n#include <stdint.h>\n')
    f.write(f'#define MEE_SR {SR}\n#define MEE_HOP {HOP}\n#define MEE_PSCALE {PS}\n#define MEE_NSAMP {len(bank)}\n#define MEE_NFRAG {len(tab)}\n')
    f.write('static const int16_t kMuLaw[256] = {' + ','.join(str(int(round(v * 32767))) for v in lut) + '};\n')
    f.write('struct MeeFrag { uint16_t start, len; uint8_t type; };\n')
    f.write('static const MeeFrag kFrags[MEE_NFRAG] = {' + ','.join('{%d,%d,%d}' % t for t in tab) + '};\n')
    f.write('static const uint8_t kPeriod[%d] = {' % len(per) + ','.join(map(str, per)) + '};\n')
    f.write('static const uint8_t kBank[MEE_NSAMP] = {' + ','.join(map(str, bank)) + '};\n')
print(f"OK: {len(tab)} frammenti ({NV} vocali, {len(words)} parole), {len(bank)/SR:.2f} s")
for i, (a, b, t, p) in enumerate(frags): print(f"  {i+1:2} {'vocale' if t == 0 else 'parola'}  {a/fps:7.2f} - {b/fps:7.2f} s")
