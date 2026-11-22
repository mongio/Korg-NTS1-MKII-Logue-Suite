# BOCCA v0.2 – filtro a formanti per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture
**Novità v0.2**: le suddivisioni a tempo di RATE ora si vedono sul display (prima usavano "/"); calcola un solo canale quando l'ingresso è mono.
**Rev. 3 – ottimizzazione**: compilata per la velocità e alleggerita senza cambiare il suono (verificato confrontando la versione nuova con la precedente).

Fa "parlare" qualsiasi suono. Tre filtri risonanti riproducono le formanti della bocca umana (F1, F2, F3)
e scivolano tra le vocali **U – O – A – E – I**, disposte dalla più chiusa e scura alla più aperta e chiara.
Si carica nello slot **MOD**. Developer ID: MTXT – unit ID 9. Stereo.

## Controlli
- **A – VOWEL**: la vocale di partenza (sinistra U … centro A … destra I).
- **B – DEPTH**: quanto la modulazione muove la bocca attorno a quella vocale (0 = vocale fissa).
- **MOD**: chi muove la bocca.
  - LFO: oscillazione regolare.
  - RANDOM: movimento casuale lento e morbido.
  - ENV: segue il volume del suono: più suoni forte, più la bocca si apre (un wah che parla).
  - TURING: registro a scorrimento, vocali a gradini che cambiano.
  - LOOP: lo stesso registro bloccato: un sequencer di 8 vocali che si ripete.
  - CHAT: ritmo sillabico irregolare con piccole chiusure da consonante: una voce che chiacchiera.
- **RATE**: velocità della modulazione. Prima parte libera (da 0,05 a 12 Hz),
  poi a tempo, sul display 1-1, 1-2, 1-4., 1-4, 1-4T, 1-8., 1-8, 1-8T, 1-16, 1-16T, 1-32 ("1-8" = un ottavo).
- **VOICE**: dimensione della bocca, da voce maschile profonda (0%) a bambino (100%).
- **RESO**: quanto sono marcate le vocali (i livelli restano compensati).
- **GLIDE**: scivolamento tra le vocali nei modi a gradini (TURING, LOOP, CHAT).
- **DRIVE**: saturazione prima del filtro, arricchisce i suoni poveri di armoniche (sinusoidi, triangoli) così parlano meglio.
- **BODY**: quanto suono originale resta sotto le vocali (corpo, bassi).
- **MIX**: 100% = solo effetto.

## Ricette
- *Basso che parla*: MOD ENV, VOWEL verso U, DEPTH alto, RESO 70%, inviluppo corto.
- *Coro di robot*: TURING WAVE → BOCCA in LOOP a 1/8, DEPTH al massimo.
- *Chiacchiericcio*: MOD CHAT su un accordo tenuto, VOICE 80% (bambini) o 20% (vecchi).
- *Organo che canta*: TURING SEQ in ORGAN → BOCCA in LFO lento, BODY 40%.
- *Sinusoide parlante*: oscillatore sinusoidale, DRIVE 70–80%, MOD TURING.

## Note
- Dimensione caricata 8,5 KB (v0.2) sul limite di 16 KB dello slot MOD. Nessuna memoria esterna.
- Formanti verificate in simulazione: le prime due cadono entro un'armonica dai valori attesi per tutte le vocali.
