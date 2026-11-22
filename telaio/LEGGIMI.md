# TELAIO v0.1 – la macchina di Turing applicata alla modulazione, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

Un registro di bit gira a tempo con il NTS-1, come in TURING SEQ: a ogni passo il bit che esce rientra dall'altra parte e,
con una probabilità che decide LOCK, viene invertito. Gli ultimi 8 bit danno il valore del passo, che muove filtro, panorama,
intonazione o volume. Ne escono modulazioni ritmiche che si ripetono o mutano lentamente. Si carica nello slot **MOD**.
Developer ID: MTXT – unit ID 24. (La parte sonora — filtro, panorama, intonazione, volume — è quella di GESTO.)

## Controlli
- **A – AMOUNT**: quanto agisce la modulazione.
- **B – LOCK**: a destra il disegno si ripete identico; verso il centro cambia ogni tanto (al 75% circa un passo su quattro
  a ogni giro); a sinistra è sempre nuovo.
- **DEST**: FILTER (passa basso risonante), PAN, PITCH (intonazione quantizzata sulla scala), VOLUME (gate/tremolo a passi), FILTPAN.
- **RATE**: un passo ogni 1BAR, 2BEAT, 1BEAT, 8TH (ottavo), 16TH (sedicesimo), 32ND (trentaduesimo).
- **LENGTH**: lunghezza del disegno, da 2 a 16 passi.
- **SLIDE**: da scalini netti (0) a scivolate lunghe quanto il passo (100).
- **SCALE** (per PITCH): CHROM, MAJOR, MINOR, PENTA, OCTAVE. I passi d'intonazione cadono solo sulle note della scala
  (fino a ±12 semitoni con AMOUNT al massimo).
- **OFFSET**: il punto di partenza della modulazione: taglio del filtro, posizione nello stereo, intonazione centrale.
- **RESO**: risonanza del filtro (FILTER, FILTPAN).
- **MIX**: dry/wet.

**Il tempo**: quando il NTS-1 invia il suo clock (sequencer, arpeggiatore, clock MIDI) i passi sono agganciati agli impulsi;
altrimenti TELAIO segue il tempo impostato.

## Ricette
- *Filtro che parla*: FILTER, RATE 16TH, LENGTH 8, LOCK 90%, RESO 60%.
- *Melodia che muta*: PITCH, SCALE PENTA, RATE 8TH, LOCK 75%, SLIDE 15%, MIX 50% (la voce originale più la sua ombra).
- *Gate ritmico*: VOLUME, RATE 16TH, LENGTH 16, LOCK 100%, SLIDE 0: un ritmo fisso ricavato dal registro.
- *Pioggia stereo*: PAN, RATE 32ND, LOCK 30%.

## Note
- Dimensione caricata 8,5 KB su 16 KB; trasposizione nella memoria esterna (32 KB).
- Consumo (banco di prova ARM, 10 s di audio): 0,23–0,52 s a seconda della destinazione.
- Verificato: LOCK (ripetizione esatta al massimo, casualità al minimo), passi a tempo (0,250 / 0,500 / 2,000 s a 120 bpm) e
  agganciati al clock del NTS-1, intonazione sempre sulla scala, SLIDE, stabile in tutte le destinazioni, uscita deterministica.
