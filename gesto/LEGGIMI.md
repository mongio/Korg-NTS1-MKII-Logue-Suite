# GESTO v0.1 – il movimento della tua mano come modulazione, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

GESTO registra come muovi il **knob B** per qualche battuta e lo ripete in loop, a tempo con il NTS-1, come modulazione.
Non è un LFO: è la tua curva, con esitazioni, scatti e accelerazioni. Si carica nello slot **MOD**. Developer ID: MTXT – unit ID 23.

## Su cosa agisce (DEST)
- **FILTER**: un filtro passa basso risonante che si apre e si chiude seguendo la curva (un "wah" disegnato a mano).
- **PAN**: il suono si sposta tra sinistra e destra come l'hai mosso tu.
- **PITCH**: l'intonazione sale e scende con la curva (fino a ±12 semitoni con AMOUNT al massimo; knob al centro = altezza originale).
- **VOLUME**: un tremolo o un gate con la forma del tuo gesto.
- **FILTPAN**: filtro e panorama insieme.

## Come si usa (ACTION)
- **LIVE**: il knob B comanda direttamente la destinazione (per provare e per suonare dal vivo).
- **REC**: GESTO aspetta; **appena muovi il knob B comincia a registrare**, si ferma da solo dopo LENGTH battute e passa a PLAY
  (sul display ACTION diventa PLAY). Non devi premere niente al momento giusto.
- **PLAY**: ripete il gesto registrato. Per registrarne uno nuovo, torna su REC.

## Controlli
- **A – AMOUNT**: quanto la curva agisce (0 = nessun effetto).
- **B – GESTO**: il knob da muovere (in REC viene registrato; in LIVE comanda direttamente; in PLAY non agisce).
- **DEST**, **ACTION**: vedi sopra.
- **LENGTH**: 1BAR, 2BAR, 4BAR, 8BAR (misure in 4/4 al tempo del NTS-1).
- **DIR**: FWD (avanti), REV (indietro), PINGPONG (avanti e indietro).
- **SPEED**: 0.25X, 0.5X, 1X, 2X, 4X, sempre a tempo.
- **SMOOTH**: da scalini netti (0) a curva morbida (100).
- **WEAR**: a ogni giro il gesto si "consuma" come una copia di una copia su nastro: perde dettaglio, trema un po' e scivola
  lentamente verso il centro. Corsa progressiva: valori bassi consumano piano. (Il consumo modifica il gesto registrato.)
- **RESO**: risonanza del filtro (FILTER, FILTPAN).

La curva è registrata in battiti: se cambi il tempo del NTS-1 il gesto resta a tempo (a 60 bpm dura il doppio che a 120).

## Ricette
- *Wah a mano*: DEST FILTER, RESO 60%, LENGTH 1BAR, registra qualche "colpo" del knob a tempo.
- *Il suono che cammina*: DEST PAN, LENGTH 4BAR, SMOOTH 70%, un movimento lento da un lato all'altro.
- *Melodia di intonazione*: DEST PITCH, AMOUNT 50% (±6 semitoni), SMOOTH 0: scalini come una piccola sequenza.
- *Gesto che svanisce*: qualunque DEST, WEAR 60%: il movimento perde i dettagli giro dopo giro.

## Note
- Dimensione caricata 8,5 KB su 16 KB; curva e trasposizione nella memoria esterna (~64 KB).
- Consumo (banco di prova ARM, 10 s di audio): 0,30–0,73 s a seconda della destinazione (FILTER la più pesante).
- Verificato: LIVE, REC (parte al primo movimento, esattamente LENGTH battute), riproduzione fedele e a tempo anche cambiando
  il tempo, DIR e SPEED, destinazioni (filtro, panorama, ±12 semitoni, volume), WEAR progressivo, stabile con RESO al massimo,
  uscita deterministica.
- Da verificare sul NTS-1: che il display mostri PLAY quando la registrazione finisce.
