# MAREA v0.2 – una modulazione che va a onde e cambia effetto, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

Il mare decide due cose: **quanto** agisce l'effetto del momento (l'altezza dell'onda) e **quale** effetto agisce.
Gli effetti sono sette "stazioni" disposte in cerchio lungo la riva: le onde le fanno ruotare, con una dissolvenza morbida
da una all'altra. Il mare è fatto di un'onda lunga, un'onda secondaria, un moto ondoso irregolare e, ogni tanto, un'onda
anomala più grande (che può far saltare una stazione). Si carica nello slot **MOD**. Developer ID: MTXT – unit ID 25.

**Novità v0.2** (la v0.1 era troppo leggera): intensità che va da zero al massimo seguendo l'onda; stazioni molto più decise
(flanger profondo con feedback fino al "jet", filtro su tutta la banda e risonante, saturazione vera, ring modulator pieno,
ROVINA che si consuma in pochi secondi, eco più presente); una stazione nuova a ogni cresta (ROTA ONDA); PERIODO più rapido
(0,5–16 s); l'onda attraversa lo stereo (con STEREO, la cresta passa da un lato all'altro). MIX iniziale al 100%.
Stessi parametri nello stesso ordine della v0.1.

## Le stazioni
- **FLANGE**: flanging profondo (0,25–7,75 ms) con feedback negativo fino al "jet"; il ritardo segue l'onda.
- **CHORUS**: allargamento e leggero ondeggiare d'intonazione.
- **RISACCA**: filtro risonante su tutta la banda (150 Hz nel cavo, ~14 kHz sulla cresta), come l'acqua che si ritira.
- **SALE**: saturazione asimmetrica decisa, da nastro spinto o valvola (livello contenuto).
- **RING**: ring modulation piena; la frequenza segue l'onda, da 20 Hz (ruvido) a ~2,5 kHz (metallico).
- **ROVINA**: degrado progressivo (meno bit, campionamento più grossolano, acuti che si perdono). Più a lungo la marea resta
  su questa stazione, più il suono si consuma (in pochi secondi con mare grosso, fino a ~3 bit); quando la marea passa oltre
  si ripulisce in una quindicina di secondi.
- **ECO**: un eco (60–300 ms, più lungo con PERIODO alto) come una parete di scogli; con STEREO rimbalza tra sinistra e destra.

## Controlli
- **A – MARE**: da bonaccia (onde piccole e regolari) a burrasca (onde alte, irregolari, più onde anomale).
- **B – PERIODO**: durata delle onde, da mezzo secondo a 16 secondi (la velocità dell'ondeggiare si sente chiaramente).
- **ROTA**: come ruotano le stazioni:
  **ONDA** (una stazione nuova a ogni cresta; un'onda anomala ne salta una), **MAREA** (un giro più lento, una stazione
  ogni tre onde), **CASO** (la prossima la sceglie il mare), **FISSO** (si resta sulla stazione STAZ e l'onda
  ne muove solo l'intensità).
- **STAZIONI**: quali entrano nel giro: TUTTE, PULITE (FLANGE, CHORUS, RISACCA, ECO), SPORCHE (SALE, RING, ROVINA).
- **STAZ**: la stazione di partenza (o quella fissa con ROTA FISSO).
- **AMOUNT**: intensità degli effetti.
- **STEREO**: quanto l'onda attraversa lo stereo: la cresta arriva prima a sinistra e poi a destra, e il suono la segue.
- **MIX**: dry/wet.

## Ricette
- *Bonaccia*: MARE 20%, PERIODO lungo, STAZIONI PULITE, ROTA MAREA: un giro lentissimo di colori.
- *Burrasca*: MARE 100%, PERIODO corto, STAZIONI SPORCHE, ROTA ONDA: saturazioni, ring e rovina che si danno il cambio.
- *Relitto*: ROTA FISSO, STAZ ROVINA, MARE 70%: il suono si consuma onda dopo onda.
- *Costa*: TUTTE, ROTA CASO, AMOUNT 60%, MIX 60%.

## Note
- Dimensione caricata 11,6 KB su 16 KB; linee di ritardo (~150 KB) nella memoria esterna.
- Consumo (banco di prova ARM, 10 s di audio): 0,75 s in uso normale, 1,15 s nel caso peggiore (ECO e CHORUS insieme).
- Si calcolano solo le due stazioni attive (quella che svanisce e quella che arriva).
- Misure v0.1 → v0.2 (impostazioni iniziali, accordo): distanza dal suono originale −12 → −3,4 dB; escursione dello stereo
  1–2 → ~10 dB; ritmo dell'onda che segue PERIODO (1,01 / 0,35 / 0,12 Hz misurati con onde attese di 1,00 / 0,35 / 0,13 Hz).
- Verificato (v0.1): mare (altezza 0,31–0,65 in bonaccia, 0–1 in burrasca; più onde anomale con MARE alto), rotazione nei quattro
  modi e nei tre insiemi, effetto di ogni stazione, ROVINA che si consuma e si ripulisce, passaggi senza clic, stabile ai valori
  estremi, uscita deterministica.
