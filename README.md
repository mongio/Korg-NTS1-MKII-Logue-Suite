# Mario Texture – unità logue per Korg NTS-1 digital kit mkII

Suite di unità per il **logue SDK** del Korg NTS-1 digital kit mkII: oscillatori, delay, riverberi e modulazioni.
Developer ID di tutte le unità: **MTXT** (0x4D545854, non registrato presso Korg).

Il progetto è in corso: la revisione attuale è la **Rev. 4**. Lo storico completo, la tabella delle unità e i prossimi passi sono in [`PROGETTO_MARIO_TEXTURE_rev4.md`](PROGETTO_MARIO_TEXTURE_rev4.md).

## Struttura

| Cartella | Contenuto |
|----------|-----------|
| `sorgenti/` | Sorgenti di ogni unità (27 cartelle). Ognuna ha `unit.cc`, `header.c`, il motore, `config.mk`, `Makefile` e un proprio `LEGGIMI.md` con controlli, ricette e note |
| `unita/` | I file `.nts1mkiiunit` compilati, pronti da installare |
| `test/` | Test in simulazione e banco di prova ARM (`test/banco_arm`) |
| `importate/` | Unità realizzate prima della suite (THALOS TURING V2.0, GHOST RADIO, ORBITAL V1.4), incluse senza modifiche |
| `esterne/` | Unità o materiali di terzi usati come riferimento (VOX INJECT, versioni precedenti) |
| `archivio/` | Versioni precedenti di alcune unità (es. RADIOLINA v0.2, v0.3, v0.4) |
| `documenti/` | Manuali delle unità |

## Unità principali

| Unità | Tipo | Descrizione breve |
|-------|------|-------------------|
| ARMONIA | modulazione | Armonizzatore denso: due voci trasposte, ciascuna raddoppiata da una copia scordata |
| ROMANZA | oscillatore | Frammenti di parole comprensibili |
| RADIOLINA | oscillatore | Frasi vocali, ispirata a GHOST RADIO; usata dal vivo |
| MEE-NAH | oscillatore | Oscillatore con HOLD per accumulare le note |
| LOLA | oscillatore | Oscillatore con effetto abbaio |
| TURING SEQ / TURING WAVE | oscillatore | Sequenziatore e oscillatore basati sulla macchina di Turing |
| SEAGULL | delay | Ripetizioni con gabbiani lontani |
| LOLA-D | delay | Abbai nelle ripetizioni |
| FRAMMENTI / FRAMMENTI-R | delay / riverbero | Trasposto granulare; la versione R aggiunge riverbero e shimmer |
| BOBINA / BOBINA-D | looper | Looper con trasporto a 4 posizioni, nello slot REVERB o DELAY |
| RICORDO / RICORDO-S / STANZE | riverbero | Riverberi con freeze, shimmer e modalità ambientali |
| RADIO / RADIO-W | modulazione | Sintonizzazione tra stazioni, con variante WHISTLE |
| SUPERFLANGER / SUPERPHASER / SUPERORBITAL | modulazione | Flanger, phaser e orbital, con versioni potenziate |
| BOCCA, GESTO, STORMO, TELAIO, MAREA, CAOS | modulazione | Modulazioni varie, incluse la macchina di Turing, un attrattore di Lorenz e onde che fanno ruotare più effetti |

Per il dettaglio di ogni unità (controlli, ricette, peso in memoria e stato di prova) vedi il LEGGIMI di ciascuna cartella e la tabella in `PROGETTO_MARIO_TEXTURE_rev4.md`.

## Installazione

I file in `unita/` (e quelli in `importate/`) si caricano sul NTS-1 mkII come qualsiasi unità logue, con il logue-cli o con il Korg Librarian. Le unità di tipo oscillatore vanno nello slot OSC, quelle delay nello slot DELAY, quelle riverbero nello slot REVERB e quelle di modulazione nello slot MOD.

## Compilazione

Serve il logue SDK di Korg. Ogni unità si compila dalla propria cartella:

```bash
cd sorgenti/armonia
make
```

Le unità sono compilate con `-O2`, tranne MEE-NAH, che resta `-Os` con le sole funzioni audio in `-O2` per stare nei 48 KB. Nell'ambiente di sviluppo i sorgenti compilano senza errori con gcc 13.2.

## Note

- Le unità dell'utente in `importate/` mantengono il codice, i nomi, gli unit ID e i file compilati originali: non vanno modificati.
- Il banco di prova ARM in `test/` serve a confrontare il consumo di CPU tra revisioni; i tempi misurati sono indicativi e non sostituiscono le prove sul NTS-1.

## Licenza

Da definire.
