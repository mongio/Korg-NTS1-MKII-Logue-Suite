# TURING WAVE v1.1 – oscillatore per Korg NTS-1 digital kit mkII

**Rev. 3 – ottimizzazione**: compilata per la velocità e alleggerita senza cambiare il suono (verificato confrontando la versione nuova con la precedente).

Fratello di TURING SEQ: qui il registro a scorrimento gira a frequenza audio
e i suoi bit sono direttamente la forma d'onda. Il registro scorre LENGTH volte
per ogni ciclo, quindi l'onda bloccata è intonata esattamente sulla nota suonata.
Developer ID: MTXT – unit ID 2 (convive con TURING SEQ, che è unit ID 1).

## Installazione
Carica `turing_wave.nts1mkiiunit` in uno slot OSC libero con KORG Kontrol Editor / Librarian.

## Controlli
- **A – LOCK**: tutto a destra = onda ferma, intonata. Scendendo verso il centro l'onda muta
  sempre più spesso (il timbro "ribolle"); al centro è rumore intonato. A sinistra torna stabile,
  ma con i bit invertiti a ogni giro: suona un'ottava sotto. Lo **Shape LFO** del NTS-1 modula LOCK.
  Trucco: porta LOCK verso il centro per un attimo e poi tutto a destra: congeli un timbro nuovo.
- **B – TONE**: filtro passa-basso che segue la nota (timbro coerente su tutta la tastiera).
  Tutto a destra = filtro escluso.
- **LENGTH** (2–32): passi per ciclo. Pochi = onda grezza e povera di armoniche; tanti = più dettaglio.
- **READ**: come i bit diventano suono. 1BIT (onde a impulsi), 4BIT, 8BIT, SMOOTH (gradini interpolati, più morbido).
- **MODE**: TURING (rotazione classica) o LFSR (retroazione XOR come i generatori di rumore delle vecchie console:
  sequenze molto più lunghe, suoni sporchi e metallici).
- **NOTE**: KEEP (il registro prosegue) o NEW (pattern casuale nuovo a ogni nota: ogni nota un timbro diverso).
- **BURST**: a ogni nota il registro muta per un attimo e poi si ricompone (~150 ms). Con un inviluppo corto: percussioni.
- **SUB**: sub-oscillatore quadro un'ottava sotto.
- **DRIVE**: saturazione morbida a livello compensato.

## Ricette
- *Pad digitale vivo*: LOCK ~85–90%, TONE ~60%, READ SMOOTH, LFO lento su Shape.
- *Percussioni*: NOTE NEW, BURST 70–100%, inviluppo AD corto, LENGTH 16.
- *Basso chip*: MODE LFSR, LENGTH 4–8, SUB 70%, DRIVE 50%, TONE ~50%.
- *Hi-hat / rumori*: LOCK al centro, TONE aperto, note alte, inviluppo cortissimo.

## Compilazione dai sorgenti
Copia la cartella in `logue-sdk/platform/nts-1_mkii/` e lancia `make install`
(sviluppato con gcc-arm-none-eabi 13.2; l'SDK ufficiale indica la 10.3).
