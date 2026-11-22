# VOX INJECT v1.1 – correzioni

## Perché non si caricava ("wrong user payload size")
L'unità richiedeva **63 KB** di memoria, mentre lo slot DELAY ne accetta circa **24**.
1. Il buffer del delay (`int16_t delay[16384]`, 32 KB) stava nella RAM interna dell'unità.
   → Ora è allocato nella memoria esterna (SDRAM) all'avvio con `sdram_alloc`.
2. I campioni vocali occupavano 26,4 KB (6 × 4.400 byte), più del limite da soli.
   → Le voci restano a 8 bit, identiche, ma senza il silenzio iniziale e finale (10 ms di margine): 15,4 KB.
Risultato: **19,7 KB** caricati su 24.

## Altri difetti trovati
3. **Due voci erano quasi mute**: le voci 0 e 1 avevano un picco di 4 su 127 (circa 30 dB sotto le altre).
   Sono state rimosse: VOICE ora va da 0 a 3 (corrispondono alle voci originali 2, 3, 4, 5).
4. **La voce 5 originale suonava pochissimo**: il suono cominciava 210 ms dentro l'estratto e, con i delay brevi,
   veniva ritriggerata prima di arrivarci. Togliendo il silenzio iniziale ora parte subito.
5. **Il terzo parametro era VOICE**: negli effetti DELAY il firmware collega il terzo parametro al knob B
   tenendo premuto DEL (il MIX nativo). Ora l'ordine è TIME, FEEDBACK, **MIX**, VOICE, INJECT, CHANCE, PITCH, DEGRADE,
   con MIX nel formato del template KORG (-1000…1000, dry/wet). TIME e FEEDBACK usano 0…1023 come i knob del template.

## Piccola ottimizzazione
- Il trigger a ogni ciclo del delay usava `sample % delayFrames` a ogni campione (una divisione per campione):
  ora usa un contatore, con lo stesso risultato.

## Verifiche (in simulazione)
- Tempo dell'eco esatto (50–675 ms), quattro voci udibili con livelli simili, stabile con FEEDBACK e INJECT al massimo,
  MIX nativo funzionante, consumo uguale all'originale.
- Da notare: con FEEDBACK al massimo il segnale arriva al limite (picco 1,0) per via del `clip` nell'anello: è il comportamento
  del progetto originale, non è stato cambiato.
