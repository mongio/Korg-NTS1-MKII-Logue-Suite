# SUPERFLANGER v0.1 – flanger stereo per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

Flanger stereo con quattro caratteri, interpolazione cubica (sweep lenti puliti) e livello pari al bypass.
Si carica nello slot **MOD**. Developer ID: MTXT – unit ID 16.

## Controlli
- **A – RATE**: velocità dello sweep, da 0,02 a 10 Hz.
- **B – DEPTH**: ampiezza dello sweep (fino a 7 ms).
- **MODE**
  - CLASSIC: il flanger a BBD, con feedback.
  - TAPE: il flanging a nastro "attraverso lo zero": anche il diretto è ritardato (3 ms) e la copia modulata gli passa
    davanti e dietro. Con **COLOR POS** le copie si sommano (lo sweep sale "all'infinito" e torna);
    con **COLOR NEG** la copia è invertita e all'incrocio il suono **si annulla** (misurato −25…−32 dB): il "buco" del jet a nastro.
  - BARBUP / BARBDN: barberpole, lo sweep sale (o scende) per sempre senza ricominciare; volume costante (±0,4 dB).
- **FEEDBK**: quantità di ritorno (risonanza delle tacche).
- **COLOR**: POS / NEG, polarità del feedback (NEG = suono cavo, "jet"); in TAPE sceglie somma o cancellazione.
- **MANUAL**: ritardo di partenza (0,2–8 ms): sposta le tacche.
- **STEREO**: sfasamento dell'LFO tra sinistra e destra.
- **SHAPE**: SINE, TRI, RANDOM (sweep casuale morbido), ENV (lo sweep segue il volume: si apre quando suoni forte).
- **AGE**: grana BBD, copia più scura e un po' satura (in TAPE scurisce entrambi i "nastri").
- **MIX**: 50% = flanging classico (tacche più profonde); più alto = più vibrato.

## Ricette
- *Jet*: CLASSIC, COLOR NEG, FEEDBK 80–90%, RATE lento.
- *Studio anni '60*: TAPE, COLOR NEG, FEEDBK 0, DEPTH alto, RATE molto lento.
- *Scala infinita*: BARBUP su un pad, FEEDBK 50%, RATE medio.
- *Flanger che respira*: SHAPE ENV, DEPTH alto, su una linea di basso.

## Note
- Dimensione caricata 8,3 KB su 16 KB; buffer (16 KB) nella memoria esterna.
- Tacche verificate: con 2,15 ms di ritardo la prima cade a 230 Hz (attesa 233). Stabile con FEEDBK al massimo, POS e NEG.
- Consumo (banco di prova ARM, 10 s di audio): 0,68 s.
