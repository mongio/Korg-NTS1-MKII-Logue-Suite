# LOLA v0.2 – l'oscillatore che abbaia, per Korg NTS-1 digital kit mkII
### Dedicato a Lola, bassotta a pelo duro di 7 mesi
**Novità v0.2**: le suddivisioni a tempo di REPEAT ora si vedono sul display (prima usavano "/").
**Rev. 3 – ottimizzazione**: compilata per la velocità e alleggerita senza cambiare il suono (verificato confrontando la versione nuova con la precedente).

Un oscillatore che abbaia, per sintesi (nessun campione): una sorgente vocale ruvida (jitter, sottoarmoniche,
rumore) passa da tre formanti, con la bocca che si apre e si chiude, e ogni verso ha il suo profilo di
altezza e di volume. Ogni nota fa partire un verso. Developer ID: MTXT – unit ID 10.

## Controlli
- **A – SIZE**: la taglia. 0% = cagnolino minuscolo (voce ~980 Hz), 25% = Lola (~630 Hz, default),
  100% = cane grande (~170 Hz). Cambia sia l'altezza sia la "bocca". Lo **Shape LFO** del NTS-1 modula la taglia.
- **B – ROUGH**: quanto è ruvida la voce (irregolarità, raddoppio di periodo, raspa).
- **CALL**: il verso.
  - BARK: il bau. Sale di colpo, cade, la bocca va da "u" ad "a" e si richiude.
  - YIP: guaito corto e acuto.
  - ARF: doppio bau.
  - GROWL: ringhio basso, con la vibrazione rapida.
  - WHINE: lamento acuto, con vibrato.
  - HOWL: ululato che sale e poi si chiude in "uuu".
  - MIX: a caso tra bau, guaito e doppio bau.
- **REPEAT**: con il tasto tenuto continua ad abbaiare. OFF (un verso per nota), da 0,5 a 8 Hz,
  oppure a tempo (sul display 1-1 … 1-32, con puntate e terzine: "1-8" = un ottavo).
- **PITCH**: NATURAL (la voce naturale della taglia, trasposta dalla tastiera: C4 = naturale) o KEY (abbaia sulla nota).
- **LENGTH**: durata dei versi (da metà a una volta e mezza).
- **VARY**: variazione tra un verso e l'altro (altezza, durata, bocca, volume): nessun cane abbaia due volte uguale.
- **TONE**: brillantezza.
- **PANT**: il fiatone tra un verso e l'altro, finché tieni premuto il tasto.
- **CRACK**: le stecche da cucciolo, cioè per un istante la voce salta all'ottava.

## Ricette
- *Lola al cancello*: CALL MIX, REPEAT 1-8, VARY 60%, CRACK 70%, PANT 70%.
- *Canzone abbaiata*: PITCH KEY, CALL BARK, VARY basso, suona una melodia (o usa il sequencer).
- *Notte di luna*: CALL HOWL, SIZE 60–80%, RICORDO in NASTRO con DECAY lungo.
- *Il cane della radio*: LOLA → RADIO in AM leggermente fuori sintonia → BOBINA.
- *Ringhio granulare*: CALL GROWL, FRAMMENTI in CLOUD con PITCH OCTAVES.

## Note
- Dimensione caricata 12,1 KB su 48 KB (v0.2).
- Idea per il futuro: con una registrazione dei versi veri di Lola si può fare una versione a campioni
  con il motore di MEE-NAH (la voce sarebbe la sua).
