# TURING SEQ v1.3 – oscillatore per Korg NTS-1 digital kit mkII

**Novità v1.3**: le scritte di RATE, OCTAVE e TRIG usavano caratteri non mostrabili dal display ("/" e "+"): ora si vedono.
**Rev. 3 – ottimizzazione**: compilata per la velocità e alleggerita senza cambiare il suono (verificato confrontando la versione nuova con la precedente).

Oscillatore che si sequenzia da solo con un registro a scorrimento in stile "Turing Machine".
Tieni premuta una nota: la sequenza parte da quella nota, in loop o in mutazione.
Developer ID: MTXT – versione 1.2.0.

**Novità v1.2**: registro a 32 passi; OCTAVE che traspone la sequenza mentre suona; slide alla 303 al posto di GLIDE;
scale riordinate (28: tolte le 8 meno musicali, aggiunte minore melodica, minore napoletana, ungherese minore, egiziana).
Attenzione: i preset salvati con la v1.1 vanno ricontrollati (la scala può cambiare e il vecchio GLIDE viene letto come ottava).

## Installazione
Carica `turing_seq.nts1mkiiunit` in uno slot OSC con KORG Kontrol Editor / Librarian (firmware NTS-1 mkII >= 1.0).
Se è caricata la v1.0 (ID sviluppatore diverso), eliminala prima. La v1.1 viene invece sostituita direttamente.

## Controlli
- **A – LOCK**: tutto a destra = loop bloccato; al centro = casuale; tutto a sinistra = loop doppio a bit invertiti.
- **B – RANGE**: escursione melodica da 0 a 36 semitoni.
- **LENGTH** (2–32): lunghezza del loop in passi.
- **RATE**: 1-32 … 1-1 a tempo (sul display "1-16" = un sedicesimo), oppure NOTE (un passo per ogni nota suonata).
- **SCALE**: 28 scale (elenco sotto).
- **WAVE**: 14 forme d'onda (elenco sotto).
- **TIMBRE**: carattere della forma d'onda. Anche lo **Shape LFO** del NTS-1 agisce qui.
- **T.MOD**: quanto il registro cambia il timbro a ogni passo (ogni nota con un colore diverso).
- **OCTAVE** (−3…+3, sul display -3 -2 -1 0 1 2 3): traspone la sequenza **mentre suona**, senza farla ripartire.
  Premere un tasto continua a cambiare tonalità (e, con RESET, a far ripartire il loop).
- **TRIG**: combinazioni di tre funzioni.
  - RESET: ogni nuova nota riparte dall'inizio del loop.
  - GATE: pause ritmiche generate dal registro.
  - SLIDE: scivolamento alla 303, solo sui passi scelti dal registro (circa metà), in circa un terzo della durata del passo.
  - Valori sul display: FREE, RESET, GATE, GTRS (gate+reset), SLIDE, SLRS (slide+reset), SLGT (slide+gate), SLGR (tutti e tre).

## Forme d'onda – cosa fa TIMBRE
| WAVE  | Suono | TIMBRE |
|-------|-------|--------|
| SAW   | dente di sega anti-aliasing | – |
| PULSE | onda rettangolare | larghezza impulso 50% → 5% |
| TRI   | triangolo | si deforma in rampa |
| SINE  | sinusoide | saturazione calda |
| SUPER | 3 saw scordate (supersaw) | detune fino a ±35 cents |
| SYNC  | hard sync aggressivo | rapporto slave 1× → 8× |
| FM    | FM 2 operatori 1:1 | indice di modulazione |
| BELL  | FM rapporto 3,5 (campane, metallo) | indice di modulazione |
| FOLD  | sinusoide ripiegata (west coast) | quantità di folding |
| CHIP  | 8-bit stile console | pulse 12,5/25/50% → triangolo 4 bit |
| VOX   | formante vocale / pulsar | posizione della formante |
| ORGAN | organo a drawbar | da morbido a brillante |
| BITS  | bitcrusher | meno bit e meno campioni |
| GRIT  | rumore intonato digitale | da ruvido a metallico |

## Scale (28)
- **Occidentali**: CHROM (cromatica), MAJOR, MINOR, DORIAN, PHRYGI (frigia), LYDIAN, MIXOLYD (misolidia),
  HARMMIN (minore armonica), MELMIN (minore melodica), NAPMIN (minore napoletana), PENTA, mPENTA, BLUES,
  WHOLE (toni interi), FIFTHS (tonica e quinta), OCTAVE (solo tonica).
- **Europa orientale e Mediterraneo**: HUNGMIN (ungherese minore), EGYPT (egiziana).
- **Giappone**: HIRAJO (hirajoshi), IN SEN, RYUKYU (Okinawa).
- **Indonesia (gamelan)**: PELOG (approssimata), SLENDRO (5 parti uguali).
- **India**: BHAIRAV (coincide con la doppia armonica, detta anche "araba").
- **Medio Oriente (maqam)**: HIJAZ (coincide con la frigia dominante), RAST, BAYATI (queste due con quarti di tono).
- **Microtonale**: JUST (intonazione naturale).

Tolte rispetto alla v1.1: IWATO, YO, TODI, SABA, HARMSER, 19-TET, 24-TET, BOHLEN.

## Compilazione dai sorgenti
Copia la cartella in `logue-sdk/platform/nts-1_mkii/` e lancia `make install`
(sviluppato con gcc-arm-none-eabi 13.2; l'SDK ufficiale indica la 10.3).
