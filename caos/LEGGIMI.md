# CAOS v0.2 – una modulazione guidata dall'attrattore di Lorenz, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

Tre equazioni (l'attrattore di Lorenz, la "farfalla") producono tre segnali caotici ma continui, senza scatti:
- **x** decide la posizione stereo: il sistema gira attorno a uno dei due "centri" (le due ali) e ogni tanto salta all'altro,
  e il suono passa da un lato all'altro;
- **y** muove il filtro;
- **z** muove un ritardo breve (flanger, chorus o vibrato).
Si carica nello slot **MOD**. Developer ID: MTXT – unit ID 26.

**Novità v0.2** (la v0.1 era troppo leggera): escursioni piene (filtro su tutta la banda e più risonante, ritardi più ampi,
panorama pieno), velocità su una corsa molto più ampia (da un salto ogni ~14 s a decine al secondo), flanger fino al "jet",
e due tipi nuovi in DTYPE: **RING** e **DRIVE**. Stessi parametri nello stesso ordine della v0.1 (i nuovi tipi sono in fondo).

## Controlli
- **A – SPEED**: velocità del sistema, da lentissima (un salto tra le ali ogni ~14 s) a frenetica (decine al secondo).
- **B – CHAOS**: da regolare a imprevedibile. A sinistra l'orbita si chiude su se stessa e si ripete identica (il suono va
  avanti e indietro tra le ali con regolarità, quasi un LFO); andando verso destra smette di ripetersi; al massimo è il caos
  classico: il suono resta a lungo su un'ala e salta all'altra senza preavviso.
- **DEST**: TUTTO (stereo + filtro + ritardo), RITARDO, FILTRO, PAN.
- **DEPTH**: quanto agisce.
- **DTYPE**: FLANGE (0,3–6,3 ms, feedback negativo fino al "jet"), CHORUS (8–28 ms), VIBRATO (solo la copia ritardata:
  l'intonazione ondeggia), **RING** (ring modulator con la portante, 30 Hz–3 kHz, guidata dal caos), **DRIVE** (saturazione
  la cui spinta segue z e la cui asimmetria segue le ali).
- **FEEDBK**: rigenerazione del ritardo (non agisce in VIBRATO, RING e DRIVE).
- **CUTOFF**: centro del filtro (100 Hz–12 kHz). **RESO**: risonanza.
- **WIDTH**: ampiezza stereo (e quanto le due ali allontanano i ritardi di sinistra e destra).
- **MIX**: dry/wet.

## Ricette
- *Farfalla lenta*: TUTTO, SPEED 20%, CHAOS al massimo, DTYPE CHORUS: il suono si sposta e cambia colore senza mai ripetersi.
- *Pendolo*: PAN, CHAOS 0, SPEED 50%: un autopan regolare, poi alza CHAOS per farlo "impazzire" piano.
- *Flanger nervoso*: RITARDO, DTYPE FLANGE, FEEDBK 70%, SPEED alto, CHAOS 70%.
- *Filtro che respira*: FILTRO, CUTOFF 40%, RESO 60%, SPEED basso.
- *Radio aliena*: DTYPE RING, SPEED 60%, CHAOS al massimo.
- *Valvola impazzita*: DTYPE DRIVE, DEPTH 100%, SPEED 40%.

## Note
- Dimensione caricata 8,5 KB su 16 KB; ritardo (16 KB) nella memoria esterna. Consumo (banco di prova ARM, 10 s): 0,64 s.
- Il parametro r di Lorenz va da 350 (CHAOS 0: orbita periodica) a 28 (CHAOS al massimo: caos classico), con curva
  esponenziale; le variabili sono normalizzate sulla scala dell'attrattore, così l'escursione resta uguale a qualunque CHAOS.
  Integrazione Runge-Kutta del secondo ordine con 8 sotto-passi (stabile anche alla velocità massima).
- Misure v0.1 → v0.2 (impostazioni iniziali, accordo): distanza dal suono originale −6 → −2 dB; escursione del colore
  ~4 → 13–14 dB; dello stereo ~7 → 26–27 dB; salti tra le ali da 0,07/s (SPEED 0) a 27/s (SPEED massimo) con CHAOS 75%.
- Verificato (v0.1): CHAOS 0 si ripete identico (correlazione 1,00), con CHAOS alto non si ripete e i salti tra le ali calano da 323
  a 40 in 3 minuti; integrazione stabile anche alla velocità massima; destinazioni; stabile con FEEDBK e RESO al massimo;
  uscita deterministica.
