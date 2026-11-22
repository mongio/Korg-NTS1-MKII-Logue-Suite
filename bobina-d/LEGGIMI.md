# BOBINA-D v0.4 – looper a nastro per lo slot DELAY del Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture
**Novità v0.4**: trasporto riprogettato (4 posizioni: 0 CLEAR, 1 STOP, 2 PLAY, 3 REC/DUB) e registrazione automatica AUTO1–8.
**Rev. 3 – ottimizzazione**: compilata per la velocità e alleggerita senza cambiare il suono (verificato confrontando la versione nuova con la precedente).

Stesso looper di BOBINA (stessi controlli, stesso motore), ma per lo slot **DELAY**.
Developer ID: MTXT – unit ID 7. Fino a 14 s di loop stereo (si adatta alla memoria disponibile).

## Differenze rispetto a BOBINA (slot REVERB)
Catena del NTS-1: oscillatore → filtro → inviluppo → MOD → **DELAY (BOBINA-D)** → REVERB.
- Il loop **passa nel riverbero**: puoi annegare il loop in una coda, o cambiare riverbero mentre il loop gira.
- Registra oscillatore e MOD (per esempio MEE-NAH attraverso RADIO), ma **non** il riverbero.
- Occupa lo slot DELAY, quindi non si usa insieme a FRAMMENTI.
- **Due looper**: con BOBINA-D nel DELAY e BOBINA nel REVERB hai due loop indipendenti (per esempio uno breve e ritmico
  e uno lungo che registra anche il primo). Se la memoria esterna fosse condivisa tra gli slot, il secondo looper
  si adatta da solo a un buffer più corto (10, 7 o 5 s).

## Il trasporto: knob A (ACTION), v0.4
Il knob A ha **solo 4 posizioni**, ciascuna con il suo numero sul display:

| Display | Funzione |
|---------|----------|
| **0** | CLEAR: cancella il loop (va tenuto **1 secondo**) |
| **1** | STOP |
| **2** | PLAY |
| **3** | REC / DUB: se non c'è un loop **registra**, se c'è **sovraincide** |

- Entrare in **3** avvia subito la registrazione (o l'overdub); **uscire da 3** la chiude nello stesso istante.
- Le altre posizioni scattano dopo un attimo (120 ms): attraversarle girando il knob non fa danni.
- Richiamando un preset con il knob su 3 non parte nessuna registrazione.
- Flusso tipico: **1 → 3** (registra) → **2** (chiude e suona) → **3** (sovraincide) → **2** → **1** (stop).
- **Registrazione automatica**: con SYNC su **AUTO1, AUTO2, AUTO4 o AUTO8**, portando il knob su 3 BOBINA **aspetta**
  che tu inizi a suonare, registra esattamente 1, 2, 4 o 8 battute (al tempo del NTS-1) e poi parte in loop da sola.
  Restando su 3 non sovraincide: per l'overdub esci e rientra. Se lasci il 3 prima di suonare, l'attesa si annulla.

## Controlli
- **B – SPEED**: velocità del nastro da −2× a +2×. Al centro il nastro è fermo, a sinistra va al contrario. 1× = 75% del knob.
- **MIX**: al centro ingresso e loop entrambi pieni (consigliato).
- **SYNC**: OFF, BEAT, BAR (la lunghezza del loop viene arrotondata a battiti o battute del tempo del NTS-1),
  AUTO1, AUTO2, AUTO4, AUTO8 (registrazione automatica di 1, 2, 4 o 8 battute, vedi sopra).
- **DECAY**: quanto del loop resta a ogni passaggio di overdub (100% = tutto, 50% = si dimezza).
- **WEAR**: usura del nastro a ogni giro. Il suono si scurisce, si satura, perde volume e ogni tanto ha delle cadute.
- **WOW**: wow & flutter di riproduzione.
- **TONE**: scurisce il loop in uscita.
- **SPDMODE**: STEP (SPEED si ferma su −2, −1, −½, 0, ½, 1, 2: ottave pulite) o FREE (continuo, da nastro).
- **INERTIA**: inerzia del nastro. Con valori alti cambi di velocità, STOP e ripartenza scivolano come su un registratore vero.
- **FADE**: il loop si affievolisce da solo a ogni giro.

## Ricette (valgono anche qui)
- *Disintegration loop*: registra una frase di MEE-NAH, WEAR 60–85%, WOW 30%, e lascialo girare.
- *Nastro all'ottava sotto*: SPDMODE STEP, SPEED su ½ (circa 62% del knob), FRAMMENTI in CLOUD prima.
- *Tape stop a fine brano*: INERTIA 60–80%, porta ACTION su 1 (STOP).
- *Frippertronics*: DUB fisso, DECAY 70%, suona sopra: le vecchie frasi svaniscono mentre ne aggiungi di nuove.

## Note
- Dimensione caricata 9,6 KB sul limite di 24 KB. Buffer stereo a 16 bit nella memoria esterna (14, 10, 7 o 5 s).
- Giunzione del loop sfumata in 10 ms con l'audio immediatamente precedente all'inizio: niente clic.
