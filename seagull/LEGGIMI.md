# SEAGULL v0.1 – delay con gabbiani lontani, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

Un delay che, a ogni giro delle ripetizioni, può far arrivare da lontano il verso di un gabbiano.
I versi entrano nell'anello del delay, quindi tornano anche nelle ripetizioni successive, sempre più consumati.
Deriva da VOX INJECT v1.1 (stesso delay e stessa logica di iniezione), con le voci sostituite dai gabbiani.
Si carica nello slot **DELAY**. Developer ID: MTXT – unit ID 14.

## I versi
Sei versi di gabbiano reale (*Larus argentatus*), puliti dal rumore di mare e vento (passa alto 500 Hz,
riduzione del rumore di circa 16 dB), a 10 kHz, 8 bit:
- ADULT1, ADULT2, ADULT3: richiami di adulto (le armoniche che ondeggiano, 0,34 s ciascuno);
- YOUNG1, YOUNG2: pigolii del giovane che scendono e risalgono (0,15–0,18 s);
- YOUNG3: richiamo discendente con armoniche (0,27 s).

**Fonte e licenza**: registrazione di **Stanislas Wroza, xeno-canto XC584347** (www.xeno-canto.org/584347),
Ouessant (Bretagna), licenza **CC BY-NC-SA 4.0**: va citato l'autore, non è utilizzabile per scopi commerciali,
e le opere derivate (questa unità compresa) vanno condivise con la stessa licenza.

## Controlli
- **A – TIME**: tempo del delay, da 50 a 675 ms.
- **B – FEEDBACK**: quante ripetizioni.
- **MIX** (DEL + knob B, il mix nativo): dry/wet.
- **CALL**: ADULT1–3, YOUNG1–3, oppure **RANDOM** (un verso diverso a ogni richiamo, come uno stormo).
- **INJECT**: volume dei gabbiani.
- **CHANCE**: probabilità che a ogni giro del delay arrivi un gabbiano.
- **PITCH**: velocità/altezza del verso (da 0,65× a 1,65×).
- **DEGRADE**: quanto si scuriscono le ripetizioni.
- **DISTANCE**: quanto sono lontani i gabbiani: meno acuti (fino a circa 2,4 kHz), meno livello
  (circa −13 dB al massimo) e una leggera diffusione. Al massimo sono ovattati e lontani, non spariscono.
- **SPREAD**: ogni richiamo nuovo arriva da un punto diverso dello spazio stereo; le ripetizioni restano al centro.

## Ricette
- *Porto all'alba*: CALL RANDOM, DISTANCE 60–70%, CHANCE 50%, FEEDBACK a metà, SPREAD 80%.
- *Gabbiano solitario*: CALL ADULT2, CHANCE 20%, DISTANCE 90%, TIME lungo.
- *Stormo che si allontana*: FEEDBACK alto, DEGRADE 40%, poi alza DISTANCE mentre suona.
- *Mare di Mario Texture*: MEE-NAH → RADIO in AM → SEAGULL → RICORDO in NASTRO.

## Note
- Dimensione caricata 21,1 KB su 24 KB. Buffer del delay (32 KB) e diffusori nella memoria esterna.
- Consumo leggero (banco di prova ARM: 0,42 s per 10 s di audio).
