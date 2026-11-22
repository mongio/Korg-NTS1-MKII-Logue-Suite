# FRAMMENTI-R v0.1 – granulare con riverbero a piastra e shimmer, per lo slot REVERB

FRAMMENTI per lo slot **REVERB** (l'ultimo della catena), con le fasi riverberate e shimmerose ampliate.
Developer ID: MTXT – unit ID 15. Si usa insieme a SEAGULL, FRAMMENTI o BOBINA-D nello slot DELAY.

**Differenze da FRAMMENTI** (slot DELAY, unit ID 3):
- la piccola diffusione diventa un **riverbero a piastra** vero (architettura di Dattorro), con coda da ~2 a ~20 s;
- nella piastra entrano **anche il suono in ingresso**, non solo i grani: è un riverbero con i grani dentro;
- **SHIMMER**: la coda si accende di armonici all'ottava sopra (al massimo l'ottava arriva quasi al livello della nota);
- la coda e lo shimmer **rientrano nei grani** (nei modi CLOUD, LOOP, REVERSE), che li sminuzzano di nuovo;
- **HOLD** congela insieme grani e coda: una nuvola infinita che non cresce né svanisce;
- SPREAD è fisso (70%) e il suo posto è preso da SHIMMER.

Registra in continuazione l'ingresso in un buffer (fino a ~11 s, si adatta alla memoria disponibile)
e ne ripesca frammenti in quattro modi diversi.

## Controlli
- **TIME** (knob A) e **DEPTH** (knob B): il loro significato dipende dal modo (vedi sotto).
- **MIX**: al centro dry e wet entrambi pieni; agli estremi solo dry / solo wet.
- **MODE**: CLOUD, LOOP, REVERSE, TAPS.
- **FEEDBK**: quanto il wet rientra nel buffer (con filtro anti-rimbombo sotto i 60 Hz).
- **SPACE**: quantità e lunghezza della coda della piastra (20% ≈ 2,6 s, 55% ≈ 4,4 s, 100% ≈ 22 s).
- **TONE**: scurisce grani e coda (smorza gli acuti della piastra).
- **PITCH**: trasposizione dei grani: -12, -7, -5, 0, 5, 7, 12, OCTAVES (casuale tra -12, 0, +12).
- **SYNC**: TIME a suddivisioni del tempo (1/32 … 1/1, comprese terzine e puntate). In CLOUD non agisce.
- **HOLD**: congela il buffer dei grani **e la coda della piastra**: una nuvola sospesa, a volume costante.
- **SHIMMER**: quantità di ottava sopra dentro la coda.
- **MIX** (REV + knob B, il mix nativo): dry/wet.

## I modi
| MODE | TIME | DEPTH |
|------|------|-------|
| CLOUD | dimensione dei grani (30–500 ms) | densità (2–40 grani/s) e dispersione nel tempo |
| LOOP | lunghezza del micro-loop (40 ms–2 s) | probabilità di variazioni a ogni giro: reverse, stutter, ottava sopra |
| REVERSE | lunghezza dei frammenti al contrario (80 ms–2 s) | dispersione, stereo e ottave sopra casuali |
| TAPS | tempo base (50 ms–1 s); 8 tap a 1/4, 2/4 … 8/4 del tempo | sceglie il pattern ritmico, da un tap singolo a tutti e otto |

In LOOP il frammento si ricattura ogni 4 giri; con HOLD resta fisso per sempre.
In TAPS, se cambi TIME mentre suona il tempo scivola come un nastro. In TAPS la coda non rientra nei tap (resta in uscita): otto tap dentro una piastra lunga si accumulerebbero.

## Ricette
- *Cattedrale granulare*: CLOUD, SPACE 80%, SHIMMER 60%, PITCH OCTAVES, poi HOLD.
- *Reverse nella piastra*: REVERSE, SPACE 60%, SHIMMER 30%, TONE 60%.
- *Doppio slot*: SEAGULL nel DELAY e FRAMMENTI-R nel REVERB: gabbiani che si perdono nella nebbia.
- *Glitch ritmico*: LOOP, SYNC ON, TIME su 1/8 o 1/16, DEPTH 50–70%.
- *Coda onirica*: REVERSE, TIME ~1 s, FEEDBK 50%, SPACE 60%, TONE 60%.
- *Ping-pong ritmico*: TAPS, SYNC ON, SPREAD 100%, DEPTH a metà.

## Compilazione dai sorgenti
Copia la cartella in `logue-sdk/platform/nts-1_mkii/` e lancia `make install`
(sviluppato con gcc-arm-none-eabi 13.2; l'SDK ufficiale indica la 10.3).
