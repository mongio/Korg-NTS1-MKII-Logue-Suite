# STANZE v0.2 – riverbero a quattro stanze, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

Un solo riverbero con quattro modalità, che ricostruiscono il carattere di quattro riverberi logue per Prologue
(MIST, HAZE, THEATRE, CATHEDRAL). Si carica nello slot **REVERB**. Developer ID: MTXT – unit ID 19.

**Novità v0.2**: DEPTH ha ora lo stesso verso in tutte le modalità: **a destra più riverbero** (più corpo).
Nelle stanze chiare (NEBBIA, FOSCHIA) la corsa del passa alto è stata invertita rispetto all'originale per Prologue:
a sinistra resta solo la parte alta, a destra il riverbero si riempie. Un valore di DEPTH salvato con la v0.1 in NEBBIA o
FOSCHIA si legge rovesciato: va reimpostato.

## Le quattro stanze
| MODE | Carattere | Ricostruisce |
|---|---|---|
| **NEBBIA** | chiaro e arioso, stanza piccola | MIST |
| **FOSCHIA** | chiaro, stanza grande (coda doppia, echi più radi) | HAZE |
| **TEATRO** | scuro, stanza piccola | THEATRE |
| **CATTEDRALE** | scuro, stanza grande | CATHEDRAL |
Cambiando MODE tra due stanze della stessa grandezza (NEBBIA↔TEATRO, FOSCHIA↔CATTEDRALE) la coda continua e cambia
solo il colore. Cambiando grandezza la coda si spegne in circa 20 ms e la stanza riparte vuota: niente clic né sirene.

## Controlli
- **A – TIME**: lunghezza della coda, da circa 3 s a 37 s (stanze piccole) o da 7 s a 75 s (stanze grandi).
  **Al massimo la coda si congela** e resta a volume costante; si può suonare sopra la coda congelata.
- **B – DEPTH**: il colore. Nelle stanze chiare è un passa alto (da ~100 Hz a ~7,8 kHz: più giri, più resta solo l'aria);
  nelle stanze scure è un passa basso (da ~90 Hz a ~12,7 kHz: più giri, più si apre). Leggermente risonante, come l'originale.
- **MIX** (REV + knob B, il mix nativo): dry/wet a potenza costante.
- **MODE**: NEBBIA, FOSCHIA, TEATRO, CATTEDRALE.
- **MOD**: profondità del "wow" della coda (la leggera oscillazione d'intonazione che la rende viva). 50% = come l'originale,
  0% = coda ferma, 100% = doppia.
- **SIZE**: grandezza della stanza attorno all'originale (50%), da metà al doppio; la durata della coda resta quella di TIME.

## Ricette
- *Aria*: NEBBIA, TIME 40%, DEPTH 60%: solo la parte alta del suono si riverbera, il basso resta asciutto.
- *Velluto*: TEATRO, TIME 50%, DEPTH 40%, MIX al 30%.
- *Navata*: CATTEDRALE, TIME 80%, DEPTH 70%, MOD 70%.
- *Drone sospeso*: FOSCHIA o CATTEDRALE, TIME al massimo (coda congelata), suona sopra un accordo con MEE-NAH in HOLD.

## Come è stato fatto
I quattro riverberi originali sono distribuiti gratuitamente ma senza sorgenti né licenza: non è stato usato né tradotto
il loro codice. I binari sono stati eseguiti in un emulatore ARM solo per **misurarne il comportamento**; STANZE è scritto
da zero e tarato su quelle misure. Le misure hanno mostrato un solo motore in quattro varianti (grandezza x colore):
- due reti indipendenti, una per canale, di due linee ciascuna (sinistra 20,7 / 35,1 ms, destra 27,5 / 46,2 ms; doppie nelle
  stanze grandi), con l'ingresso dell'altro canale a −10 dB;
- un "wow" globale: tutti i ritardi si allungano e si accorciano insieme (±0,22% con due oscillatori a 0,32 e 0,16 Hz);
- bassi che durano di più (fino a ~1,3 volte nelle stanze piccole, ~1,07 nelle grandi);
- filtro d'uscita di secondo ordine con risonanza ~1 e compensazione di livello, mix a potenza costante.

Confronto con gli originali (stesse misure): livello entro 0,1 dB, prime riflessioni entro 0,05–0,15 ms, mix identico,
filtro di DEPTH entro 1,5 dB sullo spettro, decadimento quasi ovunque entro il 5% (CATTEDRALE a TIME alto fino a −11%,
zona in cui anche l'originale varia di qualche percento da una misura all'altra).

**Differenza nota**: sotto i 200 Hz la coda dell'originale è più "liscia" (copre tutte le note gravi allo stesso modo);
in STANZE ci sono risonanze più marcate, e le code di note gravi tenute sono in media ~7 dB più deboli, con differenze
da nota a nota. Su materiale vario (accordi, percussioni) la differenza è piccola; su un basso tenuto si può sentire.

## Prove d'ascolto
`stanze_nebbia_AB.wav`, `stanze_foschia_AB.wav`, `stanze_teatro_AB.wav`, `stanze_cattedrale_AB.wav`: la stessa frase passa
prima nell'originale (emulato) e poi in STANZE, con le stesse impostazioni (TIME e DEPTH a metà, MIX 50%).

## Note
- Dimensione caricata 13,0 KB su 24 KB; linee di ritardo (~257 KB) nella memoria esterna.
- Consumo leggero (banco di prova ARM, 10 s di audio): 0,60 s (RICORDO 2,08 s).
- Stabilità verificata: coda congelata costante entro ~1 dB su 5 minuti, nessun picco oltre 1 ai valori estremi, nessun clic ai cambi di MODE.
