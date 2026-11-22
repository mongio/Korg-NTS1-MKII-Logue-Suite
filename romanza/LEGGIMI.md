# ROMANZA v0.3 – oscillatore vocale a frammenti di parole, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture
**Novità v0.3**: tolte le modalità PARA (erano un doppione); la parafonia è solo HOLD2/3/4, ora subito dopo CHOIR. Preset vecchi: PARA 2-4 diventano HOLD2-4, gli accordi restano agli stessi numeri; le vecchie HOLD (valori 12-14) vanno reimpostate.
**Novità v0.2**: HOLD2/3/4 con la logica a tastiera richiesta: le note si accumulano e, oltre il limite, la nuova sostituisce la più vecchia.

Oscillatore che suona **parole intere** (e alcune vocali tenute) prese da una traccia vocale, alla nota che premi.
Usa lo stesso motore di MEE-NAH v0.3 (PSOLA: cambia l'altezza ma non il timbro della voce), con due differenze
pensate per la comprensibilità:
- banco a **8 kHz** (banda telefonica: le consonanti si capiscono) invece di 6 kHz;
- frammenti **tagliati sulle pause tra le parole**, così ogni frammento è una parola o un pezzo di frase compiuto;
  le consonanti (non intonate) restano con la loro spaziatura naturale anche quando trasponi.
Developer ID: MTXT – unit ID 13. Banco: 8 frammenti, 4,0 s, 8 bit μ-law.

## Il banco (dalla traccia vocale caricata)
| # | tipo | posizione nel file | durata | nota |
|---|------|--------------------|--------|------|
| 1 | vocale | 183.84 – 184.24 s | 0.40 s | ~A3 |
| 2 | vocale | 91.55 – 91.95 s | 0.40 s | ~D#4 |
| 3 | parola | 50.01 – 50.58 s | 0.58 s | ~G3 |
| 4 | parola | 79.89 – 80.40 s | 0.51 s | ~C4 |
| 5 | parola | 128.46 – 128.94 s | 0.47 s | ~G#3 |
| 6 | parola | 134.99 – 135.54 s | 0.54 s | ~C#3 |
| 7 | parola | 145.54 – 146.10 s | 0.57 s | ~A#3 |
| 8 | parola | 235.11 – 235.66 s | 0.55 s | ~G3 |

(I frammenti sono indicati solo con la loro posizione nel file.)
Per cambiarne uno, indica il numero e un altro punto del file; per rigenerare il banco da un'altra traccia vocale:
`python3 build_bank_parole.py voce.mp3 [parole=6] [vocali=2]`, poi `make install`.

## Controlli
Gli stessi di MEE-NAH (vedi il suo LEGGIMI), con impostazioni iniziali pensate per far capire le parole:
**MODE SEQ** (ogni nota la parola successiva), **PLAY ONESHOT** (la parola si dice una volta), **SPEED** 1×, **JITTER** 0, **TAPE** 10%.
- **SCAN** (knob A): a sinistra le vocali tenute, poi le parole nell'ordine in cui compaiono nel brano.
- **SPEED** (knob B): 1× all'80% circa; a zero la voce si congela (ottimo sulle vocali).
- **MODE**, **PLAY**, **PITCH**, **FORMANT**, **JITTER**, **VIBRATO**, **VOICES** (MONO, CHOIR, HOLD2-4, accordi: vedi sotto), **TAPE**: come in MEE-NAH.

## VOICES
- MONO, CHOIR, **HOLD2 / HOLD3 / HOLD4** (parafonia: le note si accumulano e restano accese; oltre il limite la nuova sostituisce
  la più vecchia; una nota già accesa riparte senza doppioni), OCTAVE, FIFTH, MAJOR, MINOR, SUS4, MAJ7, MIN7.

## Per capire meglio le parole
- **PITCH MELODY** e nota **C4**: la parola esce alla sua altezza originale, ed è la forma più chiara
  (misura: somiglianza del disegno spettrale con l'originale 0,86–0,94 su 1).
- Con **PITCH KEY** le parole seguono la tastiera e restano riconoscibili entro qualche semitono dalla loro altezza;
  più ti allontani, più diventano texture (0,62–0,88 tra una quarta sotto e una quinta sopra).
- **SPEED** a 1× e **JITTER** a 0: velocità e dispersione alterano l'articolazione.
- **FORMANT** a 0: spostare le formanti cambia la "bocca" e rende le vocali meno riconoscibili.

## Ricette
- *Frase ricomposta*: MODE SEQ, PLAY ONESHOT, PITCH MELODY: suona tasti a ritmo e ricomponi le parole in un ordine nuovo.
- *Coro di parole*: VOICES MAJOR o MIN7, PITCH KEY: ogni nota diventa un accordo di parole.
- *Vocale sospesa*: SCAN su una vocale, SPEED a zero, VOICES HOLD3, inviluppo aperto.
- *Chiacchiericcio*: MODE BABBLE, JITTER 60%, VOICES CHOIR, poi RICORDO in NASTRO.

## Note
- Dimensione caricata 46,3 KB su 48 KB. Consumo di calcolo come MEE-NAH v0.3 (banco di prova ARM: 1,76 s per 10 s di audio in HOLD4).
- Il materiale è una voce registrata da un disco commerciale: per uso personale va bene; per pubblicare brani in cui i frammenti
  sono riconoscibili serve una licenza del campione.
