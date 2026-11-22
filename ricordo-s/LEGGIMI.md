# RICORDO-S v0.2 – variante di RICORDO con TUBO shimmer, LOWCUT e code fino a 100 s
### Un progetto di Mario Texture
**Novità v0.2**: si spegne da sola quando ingresso e coda sono silenziosi.
**Rev. 3 – ottimizzazione**: compilata per la velocità e alleggerita senza cambiare il suono (verificato confrontando la versione nuova con la precedente).

Un riverbero la cui coda **invecchia mentre decade**. L'attacco suona nella stanza, limpido;
man mano che la coda svanisce si stringe in banda, si satura, perde bit, prende soffio, wow e cadute,
come se il suono passasse da un nastro, un disco o una vecchia radio. Il suono diventa un ricordo.
Si carica nello slot **REVERB**. Developer ID: MTXT – unit ID 12.

**Differenze da RICORDO (unit ID 8)**:
- nuova epoca **TUBO**: risonanze di un tubo metallico + **shimmer** (la coda si accende di armonici all'ottava sopra);
- nuovo filtro **LOWCUT** (passa alto a 2 poli sulla coda) al posto di GRAIN (la grana resta, decisa da AGE secondo l'epoca);
- **DECAY** fino a **100 s** (RICORDO arriva a 30 s).
Stessi miglioramenti della v0.2: FREEZE a volume costante, saturazione senza perdita di livello.
Le due unità possono stare caricate insieme in due slot REVERB diversi per confrontarle.

## Come funziona
Una rete di 8 linee di ritardo (FDN con matrice di Hadamard) con diffusione d'ingresso e modulazione lenta.
La catena di invecchiamento sta **dentro l'anello**: la coda ci passa decine di volte, e a ogni passaggio
l'usura si somma. Nessuna automazione: più la coda è vecchia, più è consumata.

## Controlli
- **A – DECAY**: durata della coda, da 0,5 a 100 s (misurata entro il 2% fino a 20 s, entro il 6% a 100 s).
- **B – AGE**: quanto la coda invecchia a ogni giro. A zero è un riverbero pulito. **In TUBO regola lo shimmer.**
- **MIX**: al centro dry e wet pieni.
- **ERA**: il tipo di supporto.
  - NASTRO: banda che si chiude, saturazione calda, soffio, wow.
  - VINILE: crepitii che riverberano, rombo, deformazione a 33 giri (0,55 Hz).
  - RADIO: banda stretta da altoparlante, poco wow.
  - CERA: cilindro fonografico: banda strettissima, molto soffio, wow irregolare, pochi bit.
  - TUBO: la coda risuona in un tubo metallico (SIZE = lunghezza del tubo, da 1,5 a 9 ms) e,
    con AGE, si accende di shimmer all'ottava sopra, una coda di cristallo che sale.
- **SIZE**: dimensione dello spazio (in TUBO anche la lunghezza del tubo).
- **TONE**: brillantezza di partenza della coda.
- **WOW**: oscillazione d'intonazione della coda (si accumula con i giri).
- **LOWCUT**: passa alto a 2 poli sulla coda, da 20 Hz (spento) a circa 1,2 kHz: toglie il fango dei bassi.
- **HISS**: soffio, proporzionale alla coda (a riverbero spento non c'è rumore).
- **SINK**: la coda scende lentamente d'intonazione mentre svanisce, come un ricordo che affonda.
- **FREEZE**: congela la coda a volume costante. Con AGE > 0 il ricordo congelato continua a scurirsi e sporcarsi, ma resta.
  Con RADIO o CERA e AGE al massimo la banda è così stretta che la coda si consuma già prima di congelarla.

## Ricette
- *Cattedrale di vetro*: TUBO, AGE 70–90%, DECAY 20–60 s, LOWCUT 40–55%.
- *Drone infinito*: TUBO, AGE 50%, suona, poi FREEZE.
- *Stanza della memoria*: NASTRO, DECAY ~5 s, AGE 70%, HISS 25%.
- *Disco del nonno*: VINILE, AGE 85%, WOW 35%, MEE-NAH in MELODY.
- *Messaggio dal fondo*: CERA, AGE 90%, SINK 70%, DECAY lungo.
- *Ricordo sospeso*: suona un accordo, FREEZE ON con AGE 50–60%: resta sospeso e si consuma nel colore.
- *Catena completa*: MEE-NAH → RADIO → BOBINA-D (slot DELAY) → RICORDO: il loop svanisce in un ricordo.

## Note
- Dimensione caricata 11,0 KB su 24 KB; circa 290 KB di memoria esterna.
- È l'unità più pesante del progetto per la CPU: se senti crepitii, ridurre SIZE o disattivare SINK alleggerisce.
