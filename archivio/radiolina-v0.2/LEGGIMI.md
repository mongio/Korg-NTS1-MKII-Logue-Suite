# RADIOLINA v0.2 – la radiolina che ogni tanto canta, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

Un oscillatore che suona come una piccola radio a transistor: tra fruscio, crepitii e ronzio, ogni tanto la stazione
trasmette una delle tue due frasi cantate ("ahimiamor").
**Novità v0.2**: seconda frase e parametro PHRASE. Ispirato a GHOST RADIO (sintonia su una stazione, rumore fuori stazione,
deriva, eco fantasma), riscritto per una sola stazione e una sola frase. Si carica nello slot **OSC**.
Developer ID: MTXT – unit ID 21. (GHOST RADIO originale non è stato modificato.)

## Controlli
- **A – TUNE**: la sintonia. La stazione è al centro della corsa; spostandoti la voce svanisce, compare il fischio di
  battimento (più acuto quanto più ti allontani) e resta il fruscio.
- **B – EVERY**: ogni quanto torna la frase. A zero si ripete quasi di seguito; a metà corsa circa ogni 5–6 secondi;
  al massimo ogni 25–30 secondi. Le pause sono un po' irregolari, come una trasmissione vera.
- **SIGNAL**: quanto la stazione emerge dal rumore.
- **RADIO**: quanto "radiolina": banda sempre più stretta (a 0% ~120 Hz–5 kHz; a 60% ~460 Hz–2,5 kHz come una radio a
  transistor; a 100% ~0,7–1,8 kHz, un altoparlantino minuscolo), risonanza nasale a ~1,3 kHz e un po' di saturazione.
- **NOISE**: rumori di fondo: fruscio, crepitii, un filo di ronzio di rete a 50 Hz, qualche scarica. A 0 il fondo è muto.
- **DRIFT**: la sintonia che scivola, il segnale che va e viene, un leggero tremolio della voce.
- **GHOST**: un'eco fantasma della frase (~140 ms, con un ritardo che ondeggia).
- **PHRASE**: A (la frase originale), B ("frase 1"), **ALT** (si alternano a ogni ripetizione; è l'impostazione iniziale),
  RANDOM (ogni volta una delle due a caso). In KEY TRIGGER con ALT, i tasti bianchi fanno partire la A e i neri la B.
- **KEY**: AUTO (la frase torna da sola secondo EVERY), TRIGGER (ogni nota fa partire la frase da capo),
  PITCH (le note traspongono la frase: un'ottava sotto dura il doppio; C4 = altezza originale).

## Ricette
- *Radio in cucina*: TUNE al centro, EVERY a metà, RADIO 60%, NOISE 30%, SIGNAL 80%.
- *Da lontano, di notte*: SIGNAL 35%, DRIFT 60%, NOISE 60%, GHOST 30%.
- *Cercando la stazione*: KEY AUTO, EVERY basso, e muovi TUNE lentamente verso il centro.
- *Coro di radio*: KEY PITCH con un accordo arpeggiato, RADIO 80%, poi STANZE in TEATRO.

## Note
- Le frasi (fornite dall'utente): A 3,62 s, B 4,31 s, entrambe a 8 kHz in IMA ADPCM a 4 bit, codificate con ricerca in avanti
  (rapporto segnale/rumore A 29 dB, B 21 dB: la frase B contiene più fiato e riverbero, che l'ADPCM segue meno bene).
  Il fruscio della radiolina con le impostazioni iniziali è molto più forte dell'errore di compressione; solo con NOISE a 0,
  SIGNAL al massimo e RADIO bassa la frase B può suonare leggermente granulosa.
  Se le frasi provengono da un disco, per pubblicare brani in cui si riconoscono serve il permesso dei titolari dei diritti.
- Dimensione caricata 44,8 KB su 48 KB. Nessun simbolo esterno. Consumo leggero (banco di prova ARM, 10 s di audio): 0,63 s.
- Verificato: decodifica identica al riferimento campione per campione, PHRASE (A, B, ALT, RANDOM, bianchi/neri in TRIGGER), ripetizioni secondo EVERY, sintonia, banda di RADIO, rumori, modi KEY, stabilità ai valori estremi, uscita deterministica.
