# MEE-NAH v0.5 – oscillatore vocale granulare per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture
**Novità v0.5**: tolte le modalità PARA (erano un doppione); la parafonia è solo HOLD2/3/4, ora subito dopo CHOIR. Preset vecchi: PARA 2-4 diventano HOLD2-4, gli accordi restano agli stessi numeri; le vecchie HOLD (valori 12-14) vanno reimpostate.
**Novità v0.4**: HOLD2/3/4 con la logica a tastiera richiesta: le note si accumulano e, oltre il limite, la nuova sostituisce la più vecchia.
**Novità v0.3**: modalità HOLD2/3/4 (l'accordo resta con l'inviluppo aperto); le scritte di FORMANT ora si vedono sul display.
**Rev. 3 – ottimizzazione**: compilata per la velocità e alleggerita senza cambiare il suono (verificato confrontando la versione nuova con la precedente).

Oscillatore che risintetizza frammenti di voce (vocali, sillabe, glissati) alla nota che suoni.
Usa TD-PSOLA: la voce viene ricomposta periodo per periodo, così cambia l'altezza ma restano
le formanti, cioè il timbro della gola. Niente effetto "chipmunk".
Developer ID: MTXT – unit ID 4. Il banco contiene 5,5 s di voce a 6 kHz, 8 bit μ-law: grana lo-fi voluta.

**Novità v0.2**: parametro VOICES (mono, coro, parafonia fino a 4 note, accordi da un tasto solo). Sostituisce CHOIR.

## Controlli
- **A – SCAN**: posizione nel banco vocale. Il banco va da vocali tenute (sinistra) a sillabe (centro) a glissati (destra).
  Lo **Shape LFO** del NTS-1 modula SCAN: con SPEED a zero diventa un "wavetable vocale".
- **B – SPEED**: velocità di lettura. Primo 12% = freeze; poi da 0,1× a 2× (1× circa all'80%). L'altezza non cambia.
- **MODE**
  - FRAG: SCAN sceglie il punto di partenza; ogni nota riparte da lì. Con SPEED a zero, SCAN si muove dal vivo.
  - SEQ: ogni nota passa al frammento successivo (SCAN = punto di partenza dentro il frammento).
  - RAND: ogni nota pesca un frammento a caso.
  - BABBLE: salta in continuazione tra frammenti vicini a SCAN; JITTER decide quanto lontano.
- **PLAY**: ONESHOT (suona e si ferma), LOOP, BOUNCE (avanti e indietro).
- **PITCH**: KEY (la voce canta la nota premuta) o MELODY (conserva la linea originale con vibrato e portamenti, trasposta: C4 = altezza originale).
- **FORMANT**: sposta il timbro da -12 a +12 semitoni (sul display i positivi sono senza segno: 1 … 12) senza cambiare la nota (più scuro/maschile ↔ più chiaro/infantile).
- **JITTER**: dispersione casuale di posizione e intonazione dei grani.
- **VIBRATO**: vibrato da cantante anni '60 (5,6 Hz).
- **VOICES**: quante voci canta. Più voci = più calcolo.
  - MONO: una voce (il consumo minimo).
  - CHOIR: tre voci leggermente scordate e sfasate (coro).
  - **HOLD2 / HOLD3 / HOLD4**: la parafonia. Ogni tasto premuto aggiunge una nota che **resta accesa** (anche dopo il rilascio,
    anche se rilasci tutto e poi premi un tasto nuovo). Quando superi il limite (2, 3 o 4 note) la nota nuova
    **sostituisce la più vecchia**; una nota già accesa, se la ripremi, riparte senza duplicarsi. Le voci partono spente:
    suona solo quello che premi. Pensata per l'inviluppo aperto (con un inviluppo normale, rilasciando tutto si chiude comunque).
  - OCTAVE, FIFTH, MAJOR, MINOR, SUS4, MAJ7, MIN7: **accordi da un tasto solo** (funzionano anche dalla tastiera del NTS-1
    e con l'arpeggiatore).
- **TAPE**: wow, flutter, soffio, crepitii e saturazione da nastro.

## Il banco vocale (dal file meenah.mp3)
| # | tipo | posizione nel file | nota |
|---|------|--------------------|------|
| 1 | vocale | ~47.0 s | A3 |
| 2 | vocale | ~65.0 s | D4 |
| 3 | vocale | ~15.5 s | E4 |
| 4 | vocale | ~181.8 s | A#4 |
| 5 | vocale | ~195.0 s | F5 |
| 6 | sillabe | ~193.2 s | A#3 |
| 7 | sillabe | ~9.0 s | C4 |
| 8 | sillabe | ~116.1 s | D4 |
| 9 | sillabe | ~79.8 s | F4 |
| 10 | sillabe | ~26.1 s | F#4 |
| 11 | sillabe | ~153.9 s | F#5 |
| 12 | glissato | ~151.6 s | D4 |
| 13 | glissato | ~198.4 s | D#4 |
| 14 | glissato | ~58.7 s | G#4 |

Se vuoi cambiare un frammento, indica il numero e un nuovo punto del file: il banco si rigenera con `build_bank.py voce.mp3 5.55`.

## Ricette
- *Ballata*: FRAG, SPEED a zero, SCAN su una vocale, VIBRATO 40%, TAPE 30%. Suona lento.
- *Sillabe ritmiche*: SEQ, PLAY ONESHOT, inviluppo corto, arpeggiatore del NTS-1.
- *Nuvola corale*: BABBLE, VOICES CHOIR, JITTER 70%, inviluppo lento, FRAMMENTI in CLOUD sopra.
- *Coro anni '60*: VOICES HOLD4 da tastiera MIDI, FRAG con SPEED a zero su una vocale, VIBRATO 30%, TAPE 30%.
- *Melodia armonizzata*: VOICES MAJ7 o MIN7, SEQ, PLAY ONESHOT: ogni nota diventa un accordo di sillabe.
- *Eco della melodia originale*: RAND, PITCH MELODY, PLAY LOOP, TAPE alto.

## Note
- Dimensione caricata 47,3 KB (v0.3; 45,9 KB nella v0.2) sul limite di 48 KB dell'oscillatore (banco accorciato da 6,0 a 5,5 s per fare spazio alla parafonia: stessi 14 frammenti, ognuno qualche centesimo più corto).
- Per rigenerare il banco: `python3 build_bank.py voce.mp3` (richiede numpy, scipy, librosa) e poi `make install`.
