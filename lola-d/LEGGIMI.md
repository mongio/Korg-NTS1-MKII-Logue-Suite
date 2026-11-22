# LOLA-D v0.1 – delay con i cani che abbaiano, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

Un delay che, a ogni giro delle ripetizioni, può far abbaiare un cane, vicino o lontano. Gli abbai entrano nell'anello
del delay, quindi tornano anche nelle ripetizioni successive, sempre più consumati. È la versione delay di LOLA
(l'oscillatore che abbaia), costruita sul motore di SEAGULL. Si carica nello slot **DELAY**. Developer ID: MTXT – unit ID 20.

## Gli abbai
Otto abbai di cane, puliti dal rumore di fondo (passa alto 120 Hz, riduzione del rumore), a 8 kHz, 8 bit:
- BAU1–BAU6: pastore tedesco (dal più scuro al più chiaro; BAU6 è un guaito più acuto che segue un abbaio);
- BAU7–BAU8: un secondo cane, abbai più corti e secchi.
Sono cani più grandi di un bassotto: con **PITCH** alto la voce si alza e il cane "rimpicciolisce".

**Fonti** (entrambe in pubblico dominio, CC0: nessun obbligo di citazione né limite d'uso):
- BigSoundBank n. 0112 "Barking dog inside", Joseph Sardin (bigsoundbank.com);
- OpenGameArt "Dog barking mono", Brandon Morris (opengameart.org).

## Controlli
- **A – TIME**: tempo del delay, da 50 a 675 ms.
- **B – FEEDBACK**: quante ripetizioni.
- **MIX** (DEL + knob B, il mix nativo): dry/wet.
- **BARK**: BAU1–BAU8, oppure **RANDOM** (un abbaio diverso ogni volta).
- **INJECT**: volume degli abbai.
- **CHANCE**: probabilità che a ogni giro del delay abbai un cane.
- **PITCH**: velocità/altezza dell'abbaio (da 0,65× a 1,65×): alto = cane piccolo, basso = cane grande e lento.
- **DEGRADE**: quanto si scuriscono le ripetizioni.
- **DISTANCE**: quanto è lontano il cane: suono più ovattato (fino a circa 1,2 kHz) e più basso (fino a circa −12 dB),
  come un cane dietro la porta o in giardino.
- **SPREAD**: ogni abbaio arriva da un punto diverso dello spazio stereo; le ripetizioni restano al centro.

## Ricette
- *Lola alla porta*: BARK RANDOM, PITCH 75–85%, DISTANCE 40%, CHANCE 30%.
- *Il vicinato*: RANDOM, DISTANCE 80–100%, SPREAD 90%, FEEDBACK a metà, CHANCE 50%: cani lontani da ogni parte.
- *Eco nel cortile*: BAU2, PITCH 50%, FEEDBACK alto, DEGRADE 40%, TIME lungo.

## Note
- Dimensione caricata 20,5 KB su 24 KB; buffer del delay (32 KB) e diffusori nella memoria esterna.
- Consumo leggerissimo (banco di prova ARM, 10 s di audio): 0,25 s.
- Verificato: tempo dell'eco esatto, livelli dei versi simili, RANDOM equilibrato, stabile con tutto al massimo, uscita deterministica.
