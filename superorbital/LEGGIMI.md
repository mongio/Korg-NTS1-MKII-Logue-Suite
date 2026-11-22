# SUPERORBITAL v0.2 – spazio orbitale, Doppler, rotary e phaser/flanger risonanti per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

Estensione di **ORBITAL14** (l'effetto dell'utente: il suono diviso in 4 bande complementari che si muovono nello
stereo, con due campane di EQ parametrico animate). I sei modi originali sono conservati e suonano come ORBITAL14
(differenza misurata 49–95 dB sotto il segnale, prima delle due correzioni descritte sotto); in più ci sono DOPPLER, ROTARY
e, dalla v0.2, tre modi "super": XPHASER, XFLANGE e XJET.
Si carica nello slot **MOD**. Developer ID: MTXT – unit ID 18.

## Controlli
- **A – SPEED**: velocità del movimento, da 0,012 a 32 Hz (come ORBITAL14). In ROTARY: sotto il 50% lento, dal 50% veloce.
- **B – SPREAD**: ampiezza stereo del movimento.
- **MODE**
  - DEEP, ORBIT, GALAXY, QUANT, EQ WAVE, EQ RAND: i modi di ORBITAL14.
  - **DOPPLER**: le 4 bande diventano 4 satelliti in orbita, a velocità e versi diversi. Ognuno si sposta a sinistra/destra
    e avanti/indietro: avvicinandosi è più forte, più brillante e sale di intonazione (effetto Doppler), allontanandosi
    è più debole, più scuro e scende.
  - **ROTARY**: altoparlante rotante. Bassi (sotto ~850 Hz) nel tamburo, acuti nella tromba; passando da lento a veloce
    la tromba accelera in circa 1–2 s, il tamburo in 3–5 s, come in un Leslie.
  - **XPHASER** (v0.2): phaser da 4 a **16 stadi** con **risonanza fino a 0,95** e saturazione nell'anello; le due campane EQ
    **inseguono lo sweep** (sul punto a 90° e sul picco successivo) come una formante mobile. Al massimo i picchi sporgono
    di circa +32 dB e le tacche scendono a −24 dB (SUPERPHASER: +16 / −7 dB).
  - **XFLANGE** (v0.2): flanger con feedback positivo fino a 0,97; le campane si agganciano ai picchi del pettine (1/ritardo
    e la sua seconda armonica). Picchi fino a circa +31 dB (SUPERFLANGER: +24 dB).
  - **XJET** (v0.2): come XFLANGE ma con feedback negativo: il "jet" cavo; le campane vanno sui picchi dispari.
  - Nei tre modi X il suono trattato **gira nelle 4 bande orbitali** (traiettorie di ORBIT): tacche e picchi si muovono anche nello stereo.
- **EQ AMT**: quantità di guadagno delle bande e delle campane EQ. **Nei modi X: risonanza.**
- **MOTION**: ampiezza del movimento (in DOPPLER: quanto variano la distanza e la brillantezza; in ROTARY: modulazione d'ampiezza;
  **nei modi X: escursione dello sweep**, fino a ±3 ottave nel phaser, fino a 8 ms nel flanger).
- **DEPTH**: nei modi di ORBITAL14 il "respiro" delle bande; in DOPPLER e ROTARY la quantità di Doppler
  (in DOPPLER fino a circa ±100 cents, cioè un semitono, a qualunque velocità). **In XPHASER: numero di stadi** (4, 6, 8, 10, 12, 14, 16).
- **SWEEP**, **Q**, **PEAK EQ**: escursione, selettività e quantità delle due campane EQ animate (come ORBITAL14).
  **Nei modi X**: SWEEP = centro dello sweep (phaser, 90 Hz–4,4 kHz) o ritardo di partenza (flanger, 0,3–10 ms);
  Q e PEAK EQ = selettività e quantità (fino a +24 dB) delle campane che inseguono lo sweep.
- **SPREAD** nei modi X: sfasamento dello sweep tra sinistra e destra (fino a 180°) e ampiezza dell'orbita.
- **MIX**: dry/wet.

## Differenze rispetto a ORBITAL14
- MODE non è più sul knob A: sui due knob ci sono SPEED e SPREAD (più comodi dal vivo), MODE è nel menu.
- **Corretto un difetto dell'originale**: in DEEP e GALAXY le traiettorie usavano multipli non interi della fase e, a ogni giro,
  panorama e guadagni delle bande saltavano (piccolo clic periodico). Ora ogni traiettoria ha la sua fase continua:
  il movimento è identico fino al primo giro, poi prosegue fluido.
- Limitatore finale morbido sopra 0,8: ai valori estremi (EQ WAVE, EQ RAND) l'originale poteva superare 1 e andare in clipping.
- Funzioni trigonometriche ed esponenziali approssimate al posto della libreria matematica (errore del seno ~7·10⁻⁶;
  il coseno delle campane EQ è calcolato in modo esatto anche alle frequenze basse, dove il filtro è più sensibile).

## Ricette
- *Phaser che parla*: XPHASER, DEPTH 1000 (16 stadi), EQ AMT 850, PEAK EQ 650, Q 750, SPEED lento.
- *Phaser liquido a 4 stadi*: XPHASER, DEPTH 0, EQ AMT 700, PEAK EQ 300.
- *Flanger metallico sui tamburi*: XFLANGE, EQ AMT 900, SWEEP 300, PEAK EQ 600.
- *Jet*: XJET, EQ AMT 920, MOTION 800, SWEEP 200, su un pad.
- *Satelliti*: DOPPLER, SPEED 60%, MOTION 80%, DEPTH 70%, su un accordo tenuto.
- *Organo al bar*: TURING SEQ in ORGAN → ROTARY; alterna SPEED sotto e sopra il 50% per lento/veloce.
- *Galassia*: GALAXY con FRAMMENTI-R in CLOUD: le bande si espandono e collassano dentro la nuvola.

## Note
- Dimensione caricata 12,2 KB su 16 KB (compilata compatta, con le sole funzioni audio ottimizzate per la velocità);
  linee di ritardo del Doppler e del flanger (32 KB) nella memoria esterna.
- Consumo (banco di prova ARM, 10 s di audio): modi di ORBITAL14 0,63–0,79 s (originale 0,62 s); DOPPLER 1,24 s; ROTARY 1,18 s;
  XPHASER a 16 stadi 1,33 s; XFLANGE 0,97 s.
- Livelli dei modi X entro ±3 dB dall'ingresso a qualunque risonanza; stabili con tutti i parametri al massimo.
