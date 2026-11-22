# RICORDO v0.3 – il riverbero che invecchia, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture
**Novità v0.3**: si spegne da sola quando ingresso e coda sono silenziosi.
**Rev. 3 – ottimizzazione**: compilata per la velocità e alleggerita senza cambiare il suono (verificato confrontando la versione nuova con la precedente).

Un riverbero la cui coda **invecchia mentre decade**. L'attacco suona nella stanza, limpido;
man mano che la coda svanisce si stringe in banda, si satura, perde bit, prende soffio, wow e cadute,
come se il suono passasse da un nastro, un disco o una vecchia radio. Il suono diventa un ricordo.
Si carica nello slot **REVERB**. Developer ID: MTXT – unit ID 8.

**Novità v0.2** (correzioni dopo le prove):
- FREEZE ora tiene il volume costante: con AGE alto la coda congelata cambia colore ma non svanisce.
- La saturazione dell'anello non "mangia" più la coda: con AGE alto la durata resta vicina a DECAY.
- I crepitii di VINILE si affievoliscono insieme alla coda (prima la allungavano).

## Come funziona
Una rete di 8 linee di ritardo (FDN con matrice di Hadamard) con diffusione d'ingresso e modulazione lenta.
La catena di invecchiamento sta **dentro l'anello**: la coda ci passa decine di volte, e a ogni passaggio
l'usura si somma. Nessuna automazione: più la coda è vecchia, più è consumata.

## Controlli
- **A – DECAY**: durata della coda, da 0,5 a 30 s (misurata entro il 2%).
- **B – AGE**: quanto la coda invecchia a ogni giro. A zero è un riverbero pulito.
- **MIX**: al centro dry e wet pieni.
- **ERA**: il tipo di supporto.
  - NASTRO: banda che si chiude, saturazione calda, soffio, wow.
  - VINILE: crepitii che riverberano, rombo, deformazione a 33 giri (0,55 Hz).
  - RADIO: banda stretta da altoparlante, poco wow.
  - CERA: cilindro fonografico: banda strettissima, molto soffio, wow irregolare, pochi bit.
- **SIZE**: dimensione dello spazio.
- **TONE**: brillantezza di partenza della coda.
- **WOW**: oscillazione d'intonazione della coda (si accumula con i giri).
- **GRAIN**: meno bit e meno campioni nell'anello: la coda si sgrana.
- **HISS**: soffio, proporzionale alla coda (a riverbero spento non c'è rumore).
- **SINK**: la coda scende lentamente d'intonazione mentre svanisce, come un ricordo che affonda.
- **FREEZE**: congela la coda a volume costante. Con AGE > 0 il ricordo congelato continua a scurirsi e sporcarsi, ma resta.
  Con RADIO o CERA e AGE al massimo la banda è così stretta che la coda si consuma già prima di congelarla.

## Ricette
- *Stanza della memoria*: NASTRO, DECAY ~5 s, AGE 70%, HISS 25%.
- *Disco del nonno*: VINILE, AGE 85%, WOW 35%, MEE-NAH in MELODY.
- *Messaggio dal fondo*: CERA, AGE 90%, SINK 70%, DECAY lungo.
- *Ricordo sospeso*: suona un accordo, FREEZE ON con AGE 50–60%: resta sospeso e si consuma nel colore.
- *Catena completa*: MEE-NAH → RADIO → BOBINA-D (slot DELAY) → RICORDO: il loop svanisce in un ricordo.

## Note
- Dimensione caricata 9,2 KB su 24 KB; 272 KB di memoria esterna per le linee.
- È l'unità più pesante del progetto per la CPU: se senti crepitii, ridurre SIZE o disattivare SINK alleggerisce.
