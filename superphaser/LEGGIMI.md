# SUPERPHASER v0.1 – phaser vintage con ensemble, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

Il suono degli archi delle "string machine" anni '70 passati in un phaser: prima un **ensemble** (tre linee BBD modulate da
un LFO lento a 0,63 Hz e da uno veloce a 6,4 Hz, sfasate di 120°), poi un **phaser** da 4 a 12 stadi con stadi leggermente
diversi come nei circuiti veri, sweep esponenziale (il "whoosh" analogico), risonanza con saturazione morbida e calore.
Si carica nello slot **MOD**. Developer ID: MTXT – unit ID 17.

## Controlli
- **A – RATE**: velocità dello sweep, da 0,02 a 6 Hz.
- **B – DEPTH**: ampiezza dello sweep (fino a ±2,6 ottave).
- **STAGES**: 4, 6, 8, 10, 12 stadi (2…6 tacche). 4 = morbido e vocale (alla Small Stone), 12 = denso e liquido.
- **COLOR**: risonanza (quanto le tacche "cantano").
- **CENTER**: centro dello sweep, da 100 Hz a ~4 kHz.
- **ENSEMBLE**: il coro "string machine": apre e allarga lo stereo (correlazione L/R fino a 0,78).
- **STEREO**: sfasamento dello sweep tra sinistra e destra.
- **SHAPE**: SINE, TRI, RANDOM.
- **WARMTH**: saturazione morbida e acuti arrotondati.
- **MIX**: 50% = phaser classico; 100% = solo la copia sfasata (vibrato).

## Ricette
- *Archi alla Oxygène*: pad di archi (o MEE-NAH in CHOIR), ENSEMBLE 80–100%, STAGES 4, COLOR 50%, RATE lento.
- *Phaser liquido*: STAGES 12, COLOR 70–80%, CENTER 40%, RATE molto lento.
- *Organo vintage*: TURING SEQ in ORGAN, STAGES 6, ENSEMBLE 30%, WARMTH 50%.
- *Solo coro*: MIX 0 e ENSEMBLE al massimo: solo l'ensemble, senza phaser.

## Note
- Dimensione caricata 7,1 KB su 16 KB; buffer dell'ensemble (4 KB) nella memoria esterna.
- Tacche verificate: il numero è sempre la metà degli stadi. Stabile con COLOR al massimo in tutte le combinazioni.
- Consumo (banco di prova ARM, 10 s di audio, 12 stadi + ensemble): 1,20 s.
- "Solina" è un marchio registrato: il nome dell'unità è neutro.
