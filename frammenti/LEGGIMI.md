# FRAMMENTI v0.2 – delay granulare per Korg NTS-1 digital kit mkII

**Novità v0.2**: le scritte di PITCH ora si vedono sul display (prima usavano "+").
**Rev. 3 – ottimizzazione**: compilata per la velocità e alleggerita senza cambiare il suono (verificato confrontando la versione nuova con la precedente).

Delay granulare / micro-loop con diffusione riverberante interna, ispirato ai pedali "texture".
Si carica nello slot **DELAY**, così lo slot REVERB resta libero per sommarci un riverbero.
Developer ID: MTXT – unit ID 3.

Registra in continuazione l'ingresso in un buffer (fino a ~11 s, si adatta alla memoria disponibile)
e ne ripesca frammenti in quattro modi diversi.

## Controlli
- **TIME** (knob A) e **DEPTH** (knob B): il loro significato dipende dal modo (vedi sotto).
- **MIX**: al centro dry e wet entrambi pieni; agli estremi solo dry / solo wet.
- **MODE**: CLOUD, LOOP, REVERSE, TAPS.
- **FEEDBK**: quanto il wet rientra nel buffer (con filtro anti-rimbombo sotto i 60 Hz).
- **SPACE**: diffusione riverberante sul wet, da asciutto a coda lunga.
- **TONE**: scurisce wet, feedback e coda (100% = pieno).
- **PITCH**: trasposizione dei grani: -12, -7, -5, 0, 5, 7, 12, OCTAVES (casuale tra -12, 0, +12).
- **SYNC**: TIME a suddivisioni del tempo (1/32 … 1/1, comprese terzine e puntate). In CLOUD non agisce.
- **HOLD**: congela il buffer. L'effetto continua a suonare il materiale catturato mentre suoni sopra.
- **SPREAD**: ampiezza stereo.

## I modi
| MODE | TIME | DEPTH |
|------|------|-------|
| CLOUD | dimensione dei grani (30–500 ms) | densità (2–40 grani/s) e dispersione nel tempo |
| LOOP | lunghezza del micro-loop (40 ms–2 s) | probabilità di variazioni a ogni giro: reverse, stutter, ottava sopra |
| REVERSE | lunghezza dei frammenti al contrario (80 ms–2 s) | dispersione, stereo e ottave sopra casuali |
| TAPS | tempo base (50 ms–1 s); 8 tap a 1/4, 2/4 … 8/4 del tempo | sceglie il pattern ritmico, da un tap singolo a tutti e otto |

In LOOP il frammento si ricattura ogni 4 giri; con HOLD resta fisso per sempre.
In TAPS, se cambi TIME mentre suona il tempo scivola come un nastro.

## Ricette
- *Nuvola ambient*: CLOUD, TIME alto, DEPTH alto, PITCH OCTAVES, SPACE 70–80%, poi HOLD.
- *Glitch ritmico*: LOOP, SYNC ON, TIME su 1/8 o 1/16, DEPTH 50–70%.
- *Coda onirica*: REVERSE, TIME ~1 s, FEEDBK 50%, SPACE 60%, TONE 60%.
- *Ping-pong ritmico*: TAPS, SYNC ON, SPREAD 100%, DEPTH a metà.

## Compilazione dai sorgenti
Copia la cartella in `logue-sdk/platform/nts-1_mkii/` e lancia `make install`
(sviluppato con gcc-arm-none-eabi 13.2; l'SDK ufficiale indica la 10.3).
