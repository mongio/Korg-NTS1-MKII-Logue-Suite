# ARMONIA v0.1 – un armonizzatore denso, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

Due voci trasposte sopra il suono originale, ciascuna raddoppiata da una copia leggermente scordata: quattro voci in tutto,
con gli scordamenti opposti tra sinistra e destra. Con le impostazioni iniziali una nota diventa un accordo pieno: **una quinta
sopra e un'ottava sotto**, con un filo di shimmer e di deriva. Si carica nello slot **MOD**. Developer ID: MTXT – unit ID 27.

## Controlli
- **A – INTERV**: intervallo della prima voce, da 0 a 48: **24 = altezza originale**, 31 = quinta sopra, 36 = ottava sopra,
  12 = ottava sotto (il display dei knob mostra sempre il numero). Con SCALE gli scatti saltano le note fuori scala.
- **B – MIX**: suono originale ↔ voci (a potenza costante; al massimo solo le voci).
- **VOICE2**: seconda voce, OFF oppure da -24 a 24 semitoni (il display mostra i semitoni). Iniziale: -12.
- **SCALE**: CHROM, MAJOR, MINOR, PENTA, OCTAVE: gli intervalli cadono solo sulle note della scala.
- **DETUNE**: quanto è scordato il raddoppio di ogni voce (fino a ~30 cents): da un raddoppio appena percepibile a un coro.
  A 0 il raddoppio si spegne.
- **SHIMMER**: le voci rientrano in se stesse: ogni giro sale (o scende) di un altro intervallo, la "cascata" degli shimmer.
- **GRANA**: dimensione dei frammenti del trasposto (15–120 ms): corti e nervosi, lunghi e morbidi.
- **REVERSE**: OFF, ON (frammenti al contrario), MISTO (il raddoppio al contrario, la voce principale dritta).
- **DRIFT**: lento ondeggiare casuale dell'intonazione delle voci (fino a ~35 cents), come un nastro.
- **STEREO**: quanto gli scordamenti si oppongono tra i canali (più largo = immagine più ampia).

## Ricette
- *Coro*: INTERV 24 (unisono), VOICE2 12, DETUNE 70%, DRIFT 40%, MIX 60%.
- *Accordo maggiore*: INTERV 28 (+4), VOICE2 7, SCALE MAJOR, DETUNE 30%.
- *Cattedrale*: INTERV 36 (+12), VOICE2 19, SHIMMER 70%, GRANA 80%, poi STANZE in CATTEDRALE.
- *Spettri*: REVERSE ON, GRANA 70%, SHIMMER 40%, INTERV 12 (-12).

## Qualità del trasposto
Due testine con finestre sin² (la somma è esattamente 1, nessun tremolio). Quando una testina riparte sceglie, nei ~5 ms successivi,
il punto più somigliante a quello che sta suonando l'altra testina. Su un suono tenuto semplice (seno) questo porta l'ondeggiamento
del volume dal 22% al 3% e l'energia fuori dalla nota giusta da quasi tutta al 2–3%; su due note insieme dal 38% al 22% (il limite
tipico dei trasposti a grani). Intervalli precisi entro 2,5 cents.

## Note
- Dimensione caricata 8,7 KB su 16 KB; buffer (64 KB) nella memoria esterna.
- **Consumo**: è l'unità più pesante tra le modulazioni (banco di prova ARM, 10 s di audio: ~1,9 s con tutte e 8 le copie,
  simile a RICORDO). Se insieme a delay e riverbero pesanti il NTS-1 fatica, **VOICE2 OFF** oppure **DETUNE 0** dimezzano il
  calcolo (~0,8–0,9 s).
- Verificato: intervalli, scale, display di VOICE2, cascata dello shimmer (stabile anche al 100%), REVERSE, densità con le
  impostazioni iniziali (ottava sotto e quinta sopra raddoppiate, stereo largo), stabile ai valori estremi, uscita deterministica.
