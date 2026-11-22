# STORMO v0.1 – un chorus che vola come uno stormo, per Korg NTS-1 digital kit mkII
### Un progetto di Mario Texture

Il suono viene moltiplicato in un piccolo stormo di copie (da 2 a 8): ogni copia è un uccello che vola in un cielo a due
dimensioni. La posizione **orizzontale** decide dove suona nello stereo; la **profondità** decide il ritardo (6–30 ms) e quanto
arriva scuro. Quando un uccello si avvicina o si allontana il suo ritardo cambia, e nasce da sé un leggero Doppler: l'intonazione
ondeggia come in un chorus, ma seguendo il volo, non un LFO. Si carica nello slot **MOD**. Developer ID: MTXT – unit ID 22.

Il volo segue le regole degli stormi veri: **coesione** (verso il centro del gruppo), **allineamento** (la direzione dei vicini),
**separazione** (nessuno troppo vicino), più un vagare individuale e una lenta virata d'insieme. Ogni tanto arriva il **falco**:
lo stormo si apre, si divide, gira attorno al predatore e si ricompone.

## Controlli
- **A – SPEED**: velocità del volo.
- **B – DEPTH**: profondità dello stormo, cioè escursione dei ritardi: da un chorus leggero a una nuvola larga.
- **BIRDS**: 2–8 uccelli. Pochi = movimento leggibile, tanti = nuvola densa.
- **COHESION**: quanto lo stormo resta compatto (basso = sparpagliato e irrequieto).
- **PREDATOR**: quanto spesso arriva il falco (0 = mai; circa 6 attacchi al minuto al 30%, ~38 al massimo).
- **WIDTH**: ampiezza stereo.
- **SYNC**: OFF, oppure **BEAT** / **BAR**: la velocità del volo segue il tempo del NTS-1 e il falco arriva sui battiti o sulle
  misure (con probabilità PREDATOR: al 100% a ogni battito o misura).
- **TONE**: quanto gli uccelli lontani si scuriscono.
- **FEEDBK**: un po' di rigenerazione, per uno stormo più denso e metallico.
- **MIX**: dry/wet (il livello resta vicino al bypass).

## Ricette
- *Storni al tramonto*: 8 uccelli, COHESION 60%, PREDATOR 40%, SPEED lento, DEPTH alto, WIDTH 100%.
- *Chorus vivo*: 3–4 uccelli, DEPTH 30%, PREDATOR 0, MIX 40%: un chorus che non si ripete mai.
- *Il falco a tempo*: SYNC BAR, PREDATOR 100%: a ogni misura lo stormo esplode e si ricompone.
- *Sciame*: 8 uccelli, COHESION 0, SPEED alto, FEEDBK 50%.

## Note
- Dimensione caricata 9,3 KB su 16 KB; buffer (16 KB) nella memoria esterna.
- Consumo (banco di prova ARM, 10 s di audio, 8 uccelli): 0,89 s.
- Verificato: stormo sempre dentro il cielo, coesione e falco che agiscono come previsto, SYNC preciso, ritardi 6–30 ms,
  Doppler del volo ~9 cents (anche un cambio brusco di DEPTH scivola entro ~50 cents), stereo largo, stabile con FEEDBK al
  massimo, uscita deterministica.
