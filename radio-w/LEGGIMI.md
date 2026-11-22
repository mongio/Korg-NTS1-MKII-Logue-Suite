# RADIO-W v0.2 – variante di RADIO con il fischio regolabile
### Un progetto di Mario Texture
**Rev. 3 – ottimizzazione**: compilata per la velocità e alleggerita senza cambiare il suono (verificato confrontando la versione nuova con la precedente).

Il suono passa attraverso una radio a valvole degli anni '60 mentre qualcuno gira la sintonia.
Si carica nello slot **MOD**. Developer ID: MTXT – unit ID 11.

**Differenza da RADIO (unit ID 5)**: il parametro HUM è sostituito da **WHISTLE**, il volume del fischio di battimento.
Il ronzio a 50 Hz diventa automatico e cresce solo con AGE (radio nuova = niente ronzio).
Tutto il resto è identico. Le due unità possono stare caricate insieme in due slot MOD diversi per confrontarle.
Catena del NTS-1: oscillatore → filtro → inviluppo → **MOD (RADIO)** → DELAY → REVERB,
quindi delay e riverbero arrivano "dopo la radio", come nella stanza in cui è accesa.

## Controlli
- **A – TUNE**: la sintonia. Al centro la stazione principale. Verso sinistra (~17%) e destra (~84%)
  due **stazioni fantasma**. In mezzo statico, fischi e voce che si sfalda.
- **B – AGE**: l'età della radio. Restringe la banda, satura le valvole, aggiunge crepitii e ronzio.
- **BAND**
  - FM: si aggancia o sparisce nel fruscio; fuori sintonia la voce si spezza (distorsione multipath).
  - AM (onde medie): fuori sintonia compare il fischio di battimento, che sale di frequenza allontanandosi.
  - SW (onde corte): banda stretta, fading profondo e la classica voce "a paperella" della banda laterale stonata.
    Girando a destra la voce sale, a sinistra scende.
  - POCKET: radiolina a transistor con altoparlantino risonante.
- **FADING**: evanescenza di propagazione (il volume respira) e notch mobile (il timbro "fa fase").
- **DRIFT**: la sintonia scivola da sola, lentamente: la stazione va e viene.
- **STATIC**: fruscio e crepitii, più forti tra una stazione e l'altra (il fischio ora ha il suo controllo).
- **GHOST**: livello della stazione fantasma, cioè il tuo suono di circa mezzo secondo prima, più lontano e stretto.
  Si sente forte sulle sue posizioni e in sottofondo tra le stazioni.
- **WHISTLE**: volume del fischio di battimento (AM, SW, POCKET) quando sei fuori sintonia. 0% = niente fischio,
  40% = come RADIO al default, 100% = circa 2,3 volte. Non dipende più da STATIC.
- (Il ronzio di rete a 50 Hz è automatico: cresce con AGE.)
- **SEEK**: ricerca automatica: la sintonia viaggia da sola tra stazioni e rumore, e si ferma un po' su ciascuna.
  TUNE resta attivo come piccolo spostamento.
- **MIX**: 100% = solo radio.

## Ricette
- *Voce dal passato*: MEE-NAH → RADIO in AM, TUNE leggermente fuori centro, AGE 60%, FADING 40%, FRAMMENTI in CLOUD.
- *Onde corte notturne*: SW, FADING 80%, DRIFT 50%, STATIC 50%, GHOST 40%.
- *Radio che cerca*: AM, SEEK 40–60%, DRIFT 30%: nessuna esecuzione è uguale all'altra.
- *Juke-box rotto*: POCKET, AGE alto (il ronzio sale da solo), TURING SEQ in CHIP o ORGAN.

## Note
- Dimensione caricata 10,2 KB sul limite di 16 KB dello slot MOD.
- La stazione fantasma usa un buffer di 128 KB nella memoria esterna; se non fosse disponibile, l'unità funziona lo stesso senza fantasma.
