#!/usr/bin/env python3
"""MEE-NAH: genera mee_bank.h da una registrazione vocale (voce sola).
Uso: python3 build_bank.py voce.mp3 [secondi_totali=6.0]"""
import sys, numpy as np, librosa
SR, HOP, MU = 6000, 64, 255.0
src = sys.argv[1]; total = float(sys.argv[2]) if len(sys.argv) > 2 else 6.0
y, _ = librosa.load(src, sr=SR, mono=True)
f0, _, _ = librosa.pyin(y, fmin=80, fmax=900, sr=SR, frame_length=512, hop_length=HOP)
rms = librosa.feature.rms(y=y, frame_length=256, hop_length=HOP)[0][:len(f0)]
st = 12*np.log2(f0/440)+69; rmsn = rms/np.percentile(rms, 95); fps = SR/HOP
taken = np.zeros(len(f0), bool)
def pick(fn, length, count, name):
    L = int(length*fps); c = []
    for a in range(0, len(f0)-L, 4):
        s = fn(a, a+L)
        if s is not None: c.append((s, a))
    c.sort(reverse=True); out = []; used = []
    for s, a in c:
        b = a+L; g = int(0.5*fps)
        if taken[max(0, a-g):b+g].any(): continue
        p = np.nanmedian(st[a:b])
        if name != 'sillabe' and any(abs(p-q) < 1.5 for q in used): continue
        taken[a:b] = True; used.append(p); out.append((a, b, name, float(p)))
        if len(out) >= count: break
    return out
def vowel(a, b):
    s = st[a:b]
    if np.isnan(s).any() or rmsn[a:b].min() < 0.25 or np.std(s) > 0.45: return None
    return rmsn[a:b].mean()-np.std(s)
def syll(a, b):
    s = st[a:b]; r = rmsn[a:b]
    if (~np.isnan(s)).mean() < 0.6 or r.max() < 0.5: return None
    dips = ((r[1:-1] < r[:-2]) & (r[1:-1] < r[2:]) & (r[1:-1] < 0.55*r.max())).sum()
    ss = s[~np.isnan(s)]; j = (np.abs(np.diff(ss)) > 0.8).sum()
    return None if dips < 1 else dips*0.6+min(j, 4)*0.4+r.mean()
def glide(a, b):
    s = st[a:b]
    if np.isnan(s).any() or rmsn[a:b].min() < 0.2: return None
    span = s[-3:].mean()-s[:3].mean()
    mono = np.mean(np.sign(np.diff(np.convolve(s, np.ones(5)/5, 'valid'))) == np.sign(span))
    return None if abs(span) < 2.5 or mono < 0.7 else abs(span)*mono
k = total/6.0
frags = pick(vowel, 0.55, 5, 'vocale')+pick(syll, 0.5, 6, 'sillabe')+pick(glide, 0.35, 3, 'glissato')
order = {'vocale': 0, 'sillabe': 1, 'glissato': 2}; target = {'vocale': 0.5*k, 'sillabe': 0.45*k, 'glissato': 0.3*k}
frags.sort(key=lambda f: (order[f[2]], f[3]))
enc = lambda x: np.round((np.sign(x)*np.log1p(MU*np.abs(np.clip(x, -1, 1)))/np.log1p(MU)+1)*127.5).astype(np.uint8)
lut = [np.sign(m)*((1+MU)**abs(m)-1)/MU for m in (np.arange(256)/127.5-1)]
bank, per, tab, pos = [], [], [], 0
for a, b, n, p in frags:
    c = (a+b)//2; h = int(target[n]*fps/2); fa, fb = c-h, c+h
    s = y[fa*HOP:fb*HOP].astype(np.float64); s -= s.mean(); s = s/np.max(np.abs(s))*0.92
    s[:24] *= np.linspace(0, 1, 24); s[-24:] *= np.linspace(1, 0, 24)
    q = enc(s); bank.append(q)
    pf = f0[fa:fb]; per.append(np.where(np.isnan(pf), 0, np.clip(np.round(SR/np.nan_to_num(pf, nan=1)*3), 0, 255)).astype(np.uint8))
    tab.append((pos, len(q), order[n])); pos += len(q)
bank = np.concatenate(bank); per = np.concatenate(per)
with open('mee_bank.h', 'w') as f:
    f.write('#pragma once\n#include <stdint.h>\n')
    f.write(f'#define MEE_SR {SR}\n#define MEE_HOP {HOP}\n#define MEE_NSAMP {len(bank)}\n#define MEE_NFRAG {len(tab)}\n')
    f.write('static const int16_t kMuLaw[256] = {'+','.join(str(int(round(v*32767))) for v in lut)+'};\n')
    f.write('struct MeeFrag { uint16_t start, len; uint8_t type; };\n')
    f.write('static const MeeFrag kFrags[MEE_NFRAG] = {'+','.join('{%d,%d,%d}' % t for t in tab)+'};\n')
    f.write('static const uint8_t kPeriod[%d] = {' % len(per)+','.join(map(str, per))+'};\n')
    f.write('static const uint8_t kBank[MEE_NSAMP] = {'+','.join(map(str, bank))+'};\n')
print(f"OK: {len(tab)} frammenti, {len(bank)/SR:.2f} s")
