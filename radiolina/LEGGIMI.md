# RADIOLINA v0.5 – la radiolina che ogni tanto canta, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

Un oscillatore che suona come una piccola radio a transistor: tra fruscio, crepitii e ronzio, ogni tanto la stazione trasmette
una delle tue due frasi cantate ("ahimiamor"), e se vuoi, sotto, un pad di accordi. Ispirato a GHOST RADIO (che resta invariato).
Si carica nello slot **OSC**. Developer ID: MTXT – unit ID 21.

**Novità v0.5** (dopo la prova della v0.3/v0.4):
- la frase di Mina era troppo forte: ora è abbassata di 10 dB ed esce allo **stesso volume** della prima frase;
- nuovo parametro **PAD**: il volume degli accordi (il valore iniziale è ~3 dB più alto di prima);
- **accordo fisso**: in CHORDS, i valori **FIX** tengono fermo l'accordo di tonica;
- **progressioni più brevi**: il pad si muove quasi solo tra i gradi I, IV, V e VI (il II di rado; niente III e VII);
- per fare posto a PAD, **KEY e PHRASE sono uniti in PLAY** (vedi sotto).
La frase B è quella intonata sulla A (v0.4: +1,65 semitoni, centro tonale Sol).

**Attenzione, se vieni dalla v0.2–v0.4**: il NTS-1 conserva i valori dei parametri, e in questa versione due parametri hanno cambiato
significato. Il vecchio KEY = AUTO diventa PLAY = AUTO-A (solo la prima frase) e il vecchio PHRASE = ALT diventa PAD = 2% (accordi quasi
spenti). Reimposta **PLAY su AUTO-ALT** e **PAD intorno al 70%**.

## Controlli
- **A – TUNE**: la sintonia. La stazione è al centro; spostandoti la voce svanisce, compare il fischio di battimento e resta il fruscio.
- **B – EVERY**: ogni quanto torna la frase: da quasi di seguito fino a circa 30 secondi, con pause irregolari.
- **SIGNAL**: quanto la stazione emerge dal rumore.
- **RADIO**: quanto "radiolina": banda stretta, risonanza nasale, saturazione.
- **NOISE**: fruscio, crepitii, ronzio di rete, scariche.
- **DRIFT**: la sintonia che scivola, il segnale che va e viene.
- **GHOST**: un'eco fantasma della frase.
- **PLAY**: come reagisce alle note e quale frase suona, in un solo parametro:
  AUTO-A, AUTO-B, AUTO-ALT, AUTO-RND (la frase torna da sola secondo EVERY: solo A, solo B, alternate, a caso),
  TRIG-A … TRIG-RND (ogni nota fa partire la frase; con TRIG-ALT i tasti bianchi suonano la A e i neri la B),
  PITCH-A … PITCH-RND (le note traspongono la frase). Valore iniziale: AUTO-ALT.
- **PAD**: volume degli accordi (0 = spenti; 70% = valore iniziale; 100% ≈ +6 dB rispetto al 70%).
- **CHORDS**: OFF, oppure un modo (MAJ, MIN, DOR, MIX) e ogni quanto cambia l'accordo: **FIX** (fermo sulla tonica),
  1BAR, 2BAR, 4BAR, 8BAR, a tempo con il NTS-1. Per esempio "MIN FIX" o "MAJ 4BAR".
  **La tonalità è l'ultima nota che suoni.** Per stare con le frasi, suona un **Sol**.

## Ricette
- *La radio del bar*: CHORDS MAJ 2BAR, PAD 70%, suona un Sol e lascia andare.
- *Bordone*: CHORDS MIN FIX, PAD 80%, SIGNAL 60%, DRIFT 30%: un accordo fermo sotto le voci.
- *Notte in minore*: CHORDS DOR 8BAR, SIGNAL 50%, DRIFT 50%, NOISE 50%.

## Note
- Dimensione caricata 48,1 KB su 48 KB (al limite). Nessun simbolo esterno. Consumo (banco di prova ARM, 10 s, con accordi): 0,75 s.
- Verificato: le due frasi escono allo stesso volume (−23,0 / −22,9 dB nella banda della radio); FIX non cambia mai accordo;
  gradi delle progressioni I 30%, IV 24%, V 24%, VI 16%, II 5%; PAD da silenzio a +9 dB rispetto alla v0.4; PLAY nei 12 valori; stabile.
- Le versioni precedenti (v0.2, v0.3, v0.4) sono in `archivio/`: stesso unit ID, una sostituisce l'altra.
- Se le frasi provengono da un disco, per pubblicare brani in cui si riconoscono serve il permesso dei titolari dei diritti.
