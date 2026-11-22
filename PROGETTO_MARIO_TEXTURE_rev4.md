# Progetto Mario Texture – unità logue per Korg NTS-1 digital kit mkII
**Revisione 4 (Rev. 4, in corso)** – 13 ottobre 2026. Rev. 3: ottimizzazione e correzioni dopo le prove. Il progetto è aperto: nelle prossime revisioni si aggiungeranno altre unità. Developer ID di tutte le unità: **MTXT** (0x4D545854, non registrato presso Korg).

## Storico revisioni
| Rev. | Data | Contenuto |
|------|------|-----------|
| 1 | 12/10/2026 | Prime 4 unità: TURING SEQ 1.1, TURING WAVE 1.0, FRAMMENTI 0.1, MEE-NAH 0.1 |
| 2 | 12-13/10/2026 | Nuova unità MOD: RADIO 0.1 – Nuova unità REVERB: BOBINA 0.1 (looper) – BOBINA-D 0.1 (stesso looper per lo slot DELAY) – Nuova unità REVERB: RICORDO 0.1 – MEE-NAH 0.2 (VOICES: parafonia e accordi) – TURING SEQ 1.2 (32 passi, OCTAVE in corsa, slide, 28 scale) – BOBINA / BOBINA-D 0.2 (il knob A mostra CLR/STOP/PLAY/DUB/REC) – Nuova unità MOD: BOCCA 0.1 – Nuovo oscillatore: LOLA 0.1 (abbaio) – RADIO-W 0.1 (variante di RADIO con WHISTLE) – RICORDO 0.2 (FREEZE a volume costante, saturazione corretta) – RICORDO-S 0.1 (TUBO shimmer, LOWCUT, DECAY 100 s) |
| 3 | 13/10/2026 | Ottimizzazione di tutte le unità senza cambiare il suono (da −12% a −49% di calcolo); scritte del display corrette; BOBINA/BOBINA-D 0.3 (zone del knob come cifra intera); MEE-NAH 0.3 (HOLD2/3/4) |
| 4 | 13/10/2026 (in corso) | Nuovo oscillatore ROMANZA 0.1 (frammenti di parole comprensibili, banco a 8 kHz); nuovo delay SEAGULL 0.1 (gabbiani lontani nelle ripetizioni, da VOX INJECT); HOLD con accumulo delle note in MEE-NAH 0.4 e ROMANZA 0.2; FRAMMENTI-R 0.1 (REVERB, piastra + shimmer); SUPERFLANGER 0.1 e SUPERPHASER 0.1 (MOD); SUPERORBITAL 0.1 (MOD, da ORBITAL14) → 0.2 (XPHASER, XFLANGE, XJET); BOBINA / BOBINA-D 0.4 (trasporto a 4 posizioni, AUTO); MEE-NAH 0.5 e ROMANZA 0.3 (tolte le PARA, solo HOLD); aggiunte alla suite, senza modifiche, le unità dell'utente THALOS TURING V2.0, GHOST RADIO (v0.3, A, B, C, D) e ORBITAL V1.4, con i loro manuali; nuovo riverbero STANZE 0.1 (4 modalità NEBBIA/FOSCHIA/TEATRO/CATTEDRALE, ricostruito "in camera pulita" da quattro riverberi per Prologue); nuovo delay LOLA-D 0.1 (abbai di cane CC0, motore di SEAGULL); nuovo oscillatore RADIOLINA 0.1 (frase "ahimiamor", ispirato a GHOST RADIO) → 0.2 (seconda frase, PHRASE A/B/ALT/RANDOM, ADPCM); nuova modulazione STORMO 0.1 (chorus a stormo con falco); nuova modulazione GESTO 0.1 (il movimento del knob B come modulazione a tempo); RADIOLINA 0.3 (CHORDS) e 0.4 (frase B intonata); tolta la v0.1; RADIOLINA 0.5 (Mina allo stesso volume, PAD, CHORDS FIX, progressioni brevi, PLAY = KEY+PHRASE); nuova modulazione TELAIO 0.1 (macchina di Turing come modulazione); nuova modulazione MAREA 0.1 (onde che fanno ruotare sette effetti); nuova modulazione CAOS 0.1 (attrattore di Lorenz); MAREA e CAOS 0.2 (molto più incisive), STANZE 0.2 (DEPTH coerente); nuova modulazione ARMONIA 0.1 (armonizzatore denso) |

## Convenzioni per le nuove unità
- Developer ID sempre **MTXT**; unit ID progressivo per tipo di modulo. Usati finora: osc 1, 2, 4, 10, 13, 21 – delfx 3, 7, 14, 20 – modfx 5, 9, 11, 16, 17, 18, 22, 23, 24, 25, 26, 27 – revfx 6, 8, 12, 15, 19. revfx 19 STANZE, delfx 20 LOLA-D, osc 21 RADIOLINA, modfx 22 STORMO, modfx 23 GESTO, modfx 24 TELAIO, modfx 25 MAREA, modfx 26 CAOS, modfx 27 ARMONIA. Prossimo libero: **28**.
- Le unità dell'utente aggiunte alla suite (THALOS, GHOST RADIO, ORBITAL) mantengono i loro unit ID originali a 4 caratteri (es. 0x54555233 per THALOS), che non collidono con quelli progressivi della suite.
- Ogni unità ha la sua cartella in `sorgenti/` con header.c, unit.cc, motore .h, config.mk, Makefile e LEGGIMI.md.
- Ogni revisione del progetto produce un archivio `mario_texture_nts1_revN.zip` e aggiorna questo file (storico, tabella unità, da verificare, prossimi passi).
- Prima della consegna: test in simulazione (intonazione, livelli, DC, clipping, NaN, CPU), verifica dei parametri letti dal file compilato, controllo dei caratteri del display e misura nel banco di prova ARM (`test/banco_arm`).
- Compilazione: `-O2` (velocità) per tutte le unità; MEE-NAH resta `-Os` con `__attribute__((optimize("O2")))` sulle sole funzioni audio, per stare nei 48 KB.

## Unità (Rev. 4)
| Unità | Tipo | Unit ID | Versione | File | Peso / limite | Stato |
|-------|------|---------|----------|------|---------------|-------|
| TURING SEQ | oscillatore | 1 | 1.3.0 | turing_seq.nts1mkiiunit | 14,5 / 48 KB | v1.1 provata; v1.3 da provare (scritte del display) |
| TURING WAVE | oscillatore | 2 | 1.1.0 | turing_wave.nts1mkiiunit | 6,6 / 48 KB | **provato: si lascia così** (v1.1 solo ottimizzata) |
| FRAMMENTI | delay (slot DELAY) | 3 | 0.2.0 | frammenti.nts1mkiiunit | 11,6 / 24 KB (+ buffer SDRAM) | **provato: OK** (v0.2 ottimizzata, scritte PITCH) |
| MEE-NAH | oscillatore | 4 | 0.5.0 | mee_nah.nts1mkiiunit | 47,5 / 48 KB | HOLD provato (v0.4); v0.5: solo HOLD, PARA tolte |
| RADIO | modulazione (slot MOD) | 5 | 0.2.0 | radio.nts1mkiiunit | 12,0 / 16 KB (+128 KB SDRAM) | **provata: piace** (v0.2 ottimizzata) |
| BOBINA | looper (slot REVERB) | 6 | 0.4.0 | bobina.nts1mkiiunit | 11,4 / 24 KB (+ fino a 2,7 MB SDRAM) | v0.4: trasporto riprogettato (0 CLEAR / 1 STOP / 2 PLAY / 3 REC-DUB, AUTO1–8), da provare |
| BOBINA-D | looper (slot DELAY) | 7 | 0.4.0 | bobina_d.nts1mkiiunit | 11,4 / 24 KB (+ fino a 2,7 MB SDRAM) | come BOBINA (stesso motore) |
| RICORDO | riverbero (slot REVERB) | 8 | 0.3.0 | ricordo.nts1mkiiunit | 18,3 / 24 KB (+256 KB SDRAM) | v0.3 (FREEZE corretto + ottimizzazione) da provare |
| ROMANZA | oscillatore | 13 | 0.3.0 | romanza.nts1mkiiunit | 46,6 / 48 KB | HOLD provato (v0.2); v0.3: solo HOLD, PARA tolte |
| FRAMMENTI-R | granulare + riverbero (slot REVERB) | 15 | 0.1.0 | frammenti_r.nts1mkiiunit | 14,2 / 24 KB (+ ~2,2 MB SDRAM) | nuova (Rev. 4), da provare |
| SUPERFLANGER | modulazione (slot MOD) | 16 | 0.1.0 | superflanger.nts1mkiiunit | 8,3 / 16 KB (+16 KB SDRAM) | provata: non abbastanza "super"; si tiene così, la versione potenziata è XFLANGE/XJET in SUPERORBITAL |
| ARMONIA | modulazione (slot MOD) | 27 | 0.1.0 | armonia.nts1mkiiunit | 8,7 / 16 KB (+64 KB SDRAM) | nuova (Rev. 4), da provare; la più pesante delle MOD |
| CAOS | modulazione (slot MOD) | 26 | 0.2.0 | caos.nts1mkiiunit | 8,5 / 16 KB (+16 KB SDRAM) | v0.1 provata: troppo leggera, velocità impercettibile; v0.2 da provare |
| MAREA | modulazione (slot MOD) | 25 | 0.2.0 | marea.nts1mkiiunit | 11,6 / 16 KB (+150 KB SDRAM) | v0.1 provata: troppo leggera, velocità impercettibile; v0.2 da provare |
| TELAIO | modulazione (slot MOD) | 24 | 0.1.0 | telaio.nts1mkiiunit | 8,5 / 16 KB (+32 KB SDRAM) | nuova (Rev. 4), da provare |
| GESTO | modulazione (slot MOD) | 23 | 0.1.0 | gesto.nts1mkiiunit | 8,5 / 16 KB (+64 KB SDRAM) | nuova (Rev. 4), da provare |
| STORMO | modulazione (slot MOD) | 22 | 0.1.0 | stormo.nts1mkiiunit | 9,3 / 16 KB (+16 KB SDRAM) | nuova (Rev. 4), da provare |
| RADIOLINA | oscillatore | 21 | 0.5.0 | radiolina.nts1mkiiunit | 48,1 / 48 KB (al limite) | **v0.5 provata e usata in performance dal vivo: successo** |
| RADIOLINA v0.3 / v0.4 (archivio) | oscillatore | 21 | 0.3.0 / 0.4.0 | archivio/radiolina-v0.3, archivio/radiolina-v0.4 | 47,9 / 48 KB | v0.3 e v0.4 provate: Mina troppo forte, pad basso, progressioni troppo estese |
| RADIOLINA v0.2 (archivio) | oscillatore | 21 | 0.2.0 | archivio/radiolina-v0.2/radiolina_v0.2.nts1mkiiunit | 44,8 / 48 KB | due frasi, senza accordi |
| LOLA-D | delay (slot DELAY) | 20 | 0.1.0 | lola_d.nts1mkiiunit | 20,5 / 24 KB (+35 KB SDRAM) | **provata: abbai udibili** (con le impostazioni iniziali possono confondersi con un synth denso: alzare INJECT o ridurre DISTANCE) |
| STANZE | riverbero (slot REVERB) | 19 | 0.2.0 | stanze.nts1mkiiunit | 13,0 / 24 KB (+257 KB SDRAM) | nuova (Rev. 4), da provare |
| SUPERORBITAL | modulazione (slot MOD) | 18 | 0.2.0 | superorbital.nts1mkiiunit | 12,2 / 16 KB (+32 KB SDRAM) | v0.2 con XPHASER/XFLANGE/XJET, da provare |
| SUPERPHASER | modulazione (slot MOD) | 17 | 0.1.0 | superphaser.nts1mkiiunit | 7,1 / 16 KB (+4 KB SDRAM) | provata: poca risonanza, filtro poco incisivo; si tiene così, la versione potenziata è XPHASER in SUPERORBITAL |
| SEAGULL | delay (slot DELAY) | 14 | 0.1.0 | seagull.nts1mkiiunit | 21,1 / 24 KB (+35 KB SDRAM) | **provata sul NTS-1 (13/10/2026): piace** |
| RICORDO-S | riverbero (slot REVERB) | 12 | 0.2.0 | ricordo_s.nts1mkiiunit | 20,0 / 24 KB (+~290 KB SDRAM) | alternativa a RICORDO, da confrontare |
| BOCCA | modulazione (slot MOD) | 9 | 0.2.0 | bocca.nts1mkiiunit | 8,5 / 16 KB | da provare |
| LOLA | oscillatore | 10 | 0.2.0 | lola.nts1mkiiunit | 12,1 / 48 KB | da provare |
| RADIO-W | modulazione (slot MOD) | 11 | 0.2.0 | radio_w.nts1mkiiunit | 12,0 / 16 KB (+128 KB SDRAM) | alternativa a RADIO, da confrontare |

Ogni cartella in `sorgenti/` ha il suo LEGGIMI.md con controlli, ricette e note.

## Rev. 3 – ottimizzazione (stesso suono, meno calcolo)
Tempo per elaborare 10 s di audio nel banco di prova ARM (emulatore; vale come confronto, non come carico reale):

| Unità | prima | dopo | risparmio | interventi principali |
|-------|-------|------|-----------|------------------------|
| RICORDO | 3,74 s | 2,55 s | −32% | -O2; diffusori in RAM interna; wow a blocchi; un'estrazione casuale per campione; spegnimento a silenzio |
| RICORDO-S | 3,19 s | 2,44 s | −24% | come RICORDO |
| MEE-NAH (PARA 4) | 2,02 s | 1,77 s | −12% | finestre dei grani ricorsive (senza divisioni); -O2 sulle funzioni audio; costanti di TAPE per blocco |
| RADIO / RADIO-W | 1,49 s | 1,15 s | −23% | -O2; nitidezza delle stazioni ogni 16 campioni |
| FRAMMENTI | 1,27 s | 0,97 s | −24% | -O2; finestre dei grani ricorsive |
| BOCCA | 0,99 s | 0,51 s | −49% | -O2; coefficienti ogni 4 campioni; un solo canale se l'ingresso è mono |
| LOLA | 0,57 s | 0,41 s | −29% | -O2; coefficienti e altezza ogni 4 campioni; inviluppo per moltiplicazione |
| TURING SEQ | 0,50 s | 0,39 s | −21% | -O2 |
| BOBINA / BOBINA-D | 0,26 s | 0,17 s | −35% | -O2 |
| TURING WAVE | 0,25 s | 0,20 s | −19% | -O2 |

Verifiche: tutti i test funzionali ripetuti con risultati identici; confronto con la versione precedente
(BOCCA: differenza 44–61 dB sotto il segnale; LOLA: differenza spettrale media 0,03–0,37 dB, sotto la soglia di udibilità;
MEE-NAH: finestra matematicamente più esatta, timbro misurato invariato entro l'1–2%).
Sul NTS-1 il guadagno di RICORDO, FRAMMENTI e BOBINA dovrebbe essere maggiore: l'emulatore non riproduce la lentezza della SDRAM.

## Unità dell'utente nella suite (Rev. 4)
Unità realizzate dall'utente prima di questo progetto (dev ID MTXT). **Fanno parte della suite così come sono: codice, nomi, unit ID e
file compilati non vanno modificati.** La cartella `importate/` contiene lo zip originale completo, verificato byte per byte
(131 file, comprese le cartelle build/); i manuali dell'utente sono in `documenti/`. Per le unità installabili usare i
`.nts1mkiiunit` originali in `importate/<cartella>/`. (Nell'ambiente della suite i sorgenti compilano senza errori con gcc 13.2.)

### THALOS TURING V2.0 (oscillatore)
**Stato**: V2.0 confermata funzionante sullo strumento (versione di riferimento). Sviluppo fermato alla V2.0 per contenere il carico
DSP con effetti e riverberi. File `thalosturing20.nts1mkiiunit`, unit ID 0x54555233, peso 24,5 KB su 48.

**Che cosa suona**: fino a quattro note drone persistenti più una melodia OVERTONE. Le note ricevute vengono memorizzate
(massimo 4); il Note Off non le spegne, una quinta nota sostituisce la meno recente; si cancellano con EDIT → Reset → VALUE = Clear.
La Turing Machine (registro pseudo-casuale) può evidenziare una delle voci drone o scegliere la nota di partenza della melodia.
La sezione OVERTONE può sovrapporre fino a due inviluppi.

**Controlli**
| Parametro | Funzione |
|---|---|
| ALT (knob A) | Cadenza degli eventi OVERTONE (più alto = eventi più radi) |
| SHAPE (knob B) | Forma/timbro del motore |
| Slop | Scordatura casuale delle singole note drone, applicata alla pressione |
| Algo | 20 trattamenti timbrici: A1–E2, Soft, Hard, Asym, Sine, Tri, Cheby, Phase, Feed, Ring, Digital |
| Amount / Freq | Intensità e frequenza dell'elaborazione timbrica |
| Mod | Destinazione della modulazione: Freq, Amount, Shape, Algo |
| Scale | 11 scale: Original, Chromatic, Major, Minor, Pent Maj, Pent Min, Dorian, Phrygian, Whole, Dim, Random SH |
| Edit / Value | Editor delle funzioni secondarie (sotto) |

**Editor EDIT → VALUE**: Wave (18 forme OVERTONE: Triangle, Sine, Saw, Square, Pulse, SuperSaw, FM Bell, Wavetable, NoiseTone,
LoFi, Voice A/E/I/O/U, Choir, Talking, Random W), Octave (−2…+3, AUTO), Attack / Release, Env Mode (Gate, Pluck, Swell), Reset (Clear),
Drift, Sub, Spectral, Character, e i cinque parametri Turing: **T Mode** (OFF, SPOT, GATE, OVERTONE), **T Rate** (0–100, circa 0,1–8 Hz,
non sincronizzato al MIDI clock), **T Steps** (2–16 bit), **T Mutate** (0–100% di probabilità di cambiare un bit), **T Depth** (0–100%).

**Modalità Turing**: OFF (comportamento originale); SPOT (la Turing mette in evidenza una nota drone, le altre restano più basse;
T Depth = contrasto); GATE (le altre note vengono attenuate fino al silenzio, con transizioni smussate); OVERTONE (drone a livello pieno;
a ogni nuovo evento OVERTONE, T Depth è la probabilità di usare la nota scelta dalla Turing invece di quella casuale; la Turing sceglie
tra le note memorizzate, non inventa altezze nuove).

**Programmare una sequenza**: da 1 a 4 note per l'accordo; partire da T Steps 8, T Mutate 0, T Depth 80, T Rate a orecchio.
Con Mutate 0 la sequenza si ripete, aumentando Mutate si trasforma. T Steps è la lunghezza del registro, non il numero di note.
| Preset suggerito | T Mode | T Rate | T Steps | Mutate / Depth |
|---|---|---|---|---|
| Drone mobile | SPOT | 25 | 8 | 10 / 75 |
| Pulsazioni nette | GATE | 45 | 6 | 5 / 95 |
| Melodia evolutiva | OVERTONE | 35 | 8 | 15 / 85 |
| Pattern stabile | OVERTONE | 30 | 8 | 0 / 100 |

### GHOST RADIO v0.3, A, B, C, D (oscillatori)
**Che cosa sono**: quattro oscillatori radiofonici con gli stessi controlli e campioni vocali diversi (voci sintetiche eSpeak, nessun
problema di diritti). A e B usano campioni a 8 kHz, C e D campioni compatti a 6 kHz.
| Unità | Tema | File |
|---|---|---|
| A – Terrestrial | comunicazioni radio terrestri | ghost_radio_v045a.nts1mkiiunit |
| B – Unknown Signals | trasmissioni misteriose | ghost_radio_v045b.nts1mkiiunit |
| C – Numbers Station | sequenze numeriche e messaggi codificati | ghost_radio_v046c.nts1mkiiunit |
| D – Deep Space | comunicazioni extraterrestri | ghost_radio_v046d.nts1mkiiunit |

**Note tecniche**: le cartelle dello zip contengono due motori. v0.3 (base, 6 parametri, senza KeyMode, unit ID 0x47525333) e
v0.4.5/0.4.6 con KeyMode: A e B hanno codice identico (v0.4.5), come C e D (v0.4.6); tra 0.4.5 e 0.4.6 cambiano 6 righe.
Pesi 39,7 KB (v0.3) e 43,9–46,5 KB (A–D) su 48. Unit ID 0x47525333 (v0.3), 0x47523441 (A), 0x47523442 (B), 0x47523643 (C), 0x47523644 (D).

**Controlli** (comuni alle quattro)
| Controllo | Valori | Funzione |
|---|---|---|
| TUNE | 0–100 | Sintonia manuale: stazioni, interferenze, zone vuote (con SCAN attivo prevale la scansione) |
| REALITY | 0–100 | Da ricezione terrestre a segnali deformati, sintetici e alieni |
| SIGNAL | 0–100 | Presenza delle stazioni rispetto al fruscio |
| DRIFT | 0–100 | Deriva e instabilità: oscillazioni della sintonia, fluttuazioni, perdite di segnale |
| GHOST | 0–100 | Strato spettrale ed eco delle voci; aumenta la riapparizione dei frammenti |
| SCAN | 0–100 | 0 = sintonia manuale; oltre 0 = scansione automatica sempre più veloce |
| KEY MODE | 0–3 | 0 OFF (le note non comandano la radio); 1 STATION (ogni nota sceglie una delle sei stazioni, ciclo di sei note; con SCAN 0, TUNE è la sintonia fine); 2 PITCH (le note traspongono voci e componenti sintetiche); 3 TRIGGER (ogni nota sceglie una stazione e avvia un frammento vocale) |

**Prove rapide**: esplorazione manuale (SCAN 0, KEY MODE 0, TUNE lento, SIGNAL alto, REALITY basso); radio instabile (DRIFT da 0 a 100
su una stazione); presenze fantasma (confronto GHOST 0 / 100 su una stazione vocale); performance (KEY MODE 1 o 3).

### ORBITAL V1.4 – SPECTRAL MAX (MOD)
**Stato**: V1.4 scelta come versione di riferimento dopo la prova sull'NTS-1 mkII (file `orbital14.nts1mkiiunit`, unit ID 0x4F523134). Pensata per accompagnare DRONE V8.9T.
SUPERORBITAL della suite (unità separata) ne riprende i modi (modi 0–5, verificati contro il codice originale: 49–95 dB sotto il segnale fino al primo
giro della fase), che ne corregge i salti periodici di DEEP e GALAXY e aggiunge un limitatore. Unità originale in `importate/orbital_v14/` (e copia in `esterne/orbital14_originale/`);
ORBITAL e SUPERORBITAL sono unità distinte e convivono (unit ID diversi).

**Funzionamento**: quattro regioni di frequenza distribuite nello spazio stereo, equalizzazione dinamica delle bande e due filtri
parametrici automatizzati. SPEED 0,012–32 Hz (a velocità alte il movimento diventa colorazione timbrica); SWEEP fino a circa cinque ottave;
Q da filtri ampi a molto stretti; PEAK EQ fino a circa ±24 dB. Modi: DEEP SPACE (movimenti lenti e profondi, bande che respirano in modo
contrapposto), ORBIT (rotazioni stereo, anche in direzioni opposte), GALAXY (espansioni e contrazioni cicliche), QUANTUM (movimenti
irregolari e rapidi), EQ WAVE (equalizzazione ciclica sfasata), EQ RANDOM (equalizzazione pseudo-casuale con transizioni morbide).

**Preset suggeriti dal manuale** (valori ORBITAL 0–1000; in SUPERORBITAL SPEED e SPREAD sono sui knob, 0–1023)
| Parametro | DEEP AMBIENT | SPECTRAL DANCE | QUANTUM CHAOS |
|---|---|---|---|
| MODE | Deep Space | EQ Wave | Quantum |
| SPEED / SPREAD | 100 / 850 | 350 / 750 | 800 / 1000 |
| EQ AMT / MOTION | 400 / 350 | 900 / 700 | 850 / 1000 |
| MIX / DEPTH | 800 / 750 | 1000 / 500 | 1000 / 800 |
| SWEEP / Q / PEAK EQ | 450 / 300 / 400 | 1000 / 850 / 1000 | 900 / 950 / 900 |

**Uso con DRONE V8.9T**: DRONE nello slot OSC con EG su OPEN, quattro note MIDI per un accordo continuo, poi ORBITAL nello slot MOD.
Partire da EQ WAVE alzando gradualmente SWEEP, Q e PEAK EQ; passare a ORBIT o GALAXY per il movimento stereo.
**Attenzione al volume**: i picchi di equalizzazione possono alzare molto il livello (in SUPERORBITAL c'è un limitatore sopra 0,8).

## STANZE – riverbero a 4 modalità, revfx ID 19
Ricostruzione "in camera pulita" di quattro riverberi logue per Prologue (MIST, HAZE, THEATRE, CATHEDRAL; distribuiti gratuitamente
senza sorgenti né licenza): i binari sono stati eseguiti nell'emulatore ARM `test/stanze/emu.py` (Unicorn, firmware Prologue
simulato: tabelle LUT ricostruite, _fx_rand/_fx_white/_fx_get_bpm) solo per misurarli; il motore `prorev.h` è scritto da zero.
| Modalità | Originale | Carattere |
|---|---|---|
| NEBBIA | MIST | passa alto (chiaro), taglia piccola |
| FOSCHIA | HAZE | passa alto, taglia grande (ritardi doppi) |
| TEATRO | THEATRE | passa basso (scuro), taglia piccola |
| CATTEDRALE | CATHEDRAL | passa basso, taglia grande |
**Struttura ricavata dalle misure**: due reti indipendenti per canale (2 linee; base 991/1682 e 1315/2212 campioni, x2 nella taglia L;
mescolamento ortogonale), ingresso incrociato 0,32, retroazione per rete (L dura il 93% di R), tabelle g(TIME) tarate per bisezione
sul decadimento sopra 700 Hz (11 punti per taglia), TIME massimo = coda congelata con controllo automatico del livello;
**wow globale** (tutti i ritardi x(1 + ε), ε = 0,223% a 0,32 Hz fase 112° + 0,210% a 0,16 Hz fase 283°, uguale nei due canali:
è quello che dà alla coda ±20 cents e la fa variare nel tempo); bassi più lunghi (scaffale nell'anello sotto ~500 Hz, curva per taglia);
interpolazione passa-tutto (Thiran) delle linee modulate (l'Hermite smussava gli acuti a ogni giro); diffusore di 5–7 campioni per linea;
scaffale −3 dB sotto ~250 Hz sulla coda; filtro d'uscita di 2° ordine misurato (f0 e Q per 11 punti di DEPTH; HP 100 Hz–7,8 kHz Q≈1→0,76
con compensazione fino a +4,4 dB; LP 90 Hz–12,7 kHz Q 0,72–0,97 con +1,9 dB a DEPTH 0); parte diretta 0,0916 (un campione dopo nella
famiglia scura); mix a potenza costante. Cambio di MODE: stessa taglia = dissolvenza 10+10 ms con la coda che continua; taglia diversa =
20 ms fuori, azzeramento a pezzi (8192 campioni per blocco), 20 ms dentro.
**Confronto finale con gli originali**: livello entro 0,1 dB, prime riflessioni entro 0,05–0,15 ms, mix identico, spettro con DEPTH
entro ±1,5 dB, decadimento per banda quasi ovunque entro il 5% (CATTEDRALE a TIME 0,7–0,9: −7/−11%).
**Differenza aperta**: sotto 200 Hz l'originale ha una risposta liscia (±1–2 dB a 1,5 Hz di risoluzione, coerenza 0,08–0,16) mentre STANZE
ha risonanze a pettine (±11–12 dB, coerenza 0,5–0,7); code di note gravi tenute in media −7 dB, con differenze da nota a nota.
Il wow globale ha portato lo scarto da −21 a −7 dB; l'origine della liscezza residua non è stata isolata (non dipende da casualità del
firmware: l'originale non chiama _fx_rand/_fx_white né legge le LUT).
Consumo 0,60 s nel banco di prova (RICORDO 2,08 s).

## Unità esterne corrette
- **VOX INJECT v1.1** (delay, dev ID MTXT, unit ID 0x56584931, non realizzata in questo progetto): non si caricava ("wrong user payload size", 63 KB su 24).
  Corretta: buffer del delay in SDRAM, voci 8 bit senza silenzio (15,4 KB), voci 0 e 1 quasi mute rimosse (VOICE 0–3), MIX al terzo posto come da template.
  Ora 19,7 KB. File e note in `esterne/vox_inject_v1.1/`.

## Riassunto delle unità
- **TURING SEQ** (v1.2): oscillatore auto-sequenziato a registro "Turing Machine" a 32 bit. 10 parametri: LOCK, RANGE, LENGTH (2–32), RATE (1/32…1/1, NOTE), SCALE (28: occidentali incl. minore melodica e napoletana, ungherese minore, egiziana, giapponesi, gamelan, Bhairav/doppia armonica, maqam Hijaz/Rast/Bayati, just), WAVE (14 forme d'onda), TIMBRE, T.MOD, OCTAVE (−3…+3, sulla sequenza in corsa), TRIG (combinazioni di RESET, GATE, SLIDE alla 303 decisa dal bit 7). Shape LFO → TIMBRE. Rimosso GLIDE continuo (v1.1).
- **TURING WAVE**: il registro gira a frequenza audio e i bit sono la forma d'onda. LOCK (dx fermo e intonato, centro rumore, sx ottava sotto), TONE, LENGTH (2–32), READ (1BIT/4BIT/8BIT/SMOOTH), MODE (TURING/LFSR), NOTE (KEEP/NEW), BURST, SUB, DRIVE. PolyBLEP sui salti, limitatore morbido. Shape LFO → LOCK.
- **FRAMMENTI**: delay granulare ispirato ai pedali texture. Buffer mono in SDRAM (11 / 5,5 / 2,7 s secondo la memoria), modi CLOUD, LOOP, REVERSE, TAPS; FEEDBK, SPACE (diffusione interna), TONE, PITCH (incl. OCTAVES), SYNC, HOLD, SPREAD. TIME/DPTH/MIX identici al template KORG.
- **RADIO** (Rev. 2): radio a valvole anni '60. TUNE (stazione al centro, due fantasma a ~17% e ~84%), AGE, BAND (FM/AM/SW/POCKET), FADING, DRIFT, STATIC, GHOST (ingresso ritardato ~0,45 s), HUM (50 Hz), SEEK (ricerca automatica), MIX. Fischio di battimento in AM, banda laterale stonata con spostatore di frequenza Hilbert in SW, aggancio netto in FM, altoparlante risonante in POCKET. TUNE/AGE nel formato del template KORG modfx.
- **BOBINA** (Rev. 2): looper stereo a nastro nell'ultimo slot. Knob A = trasporto a zone (CLEAR 0–12% tenuto 0,5 s, STOP, PLAY, DUB, REC 82–100%), che dalla v0.2 mostra sul display CLR/STOP/PLAY/DUB/REC; REC parte ed esce all'istante, le altre zone dopo 150 ms; azioni solo al cambio di zona (preset sicuri). Knob B = SPEED −2×..+2× (1× al 75%). SYNC (OFF/BEAT/BAR), DECAY, WEAR (usura per giro), WOW, TONE, SPDMODE (FREE/STEP), INERTIA (tape stop/start), FADE. Giunzione con pre-roll di 10 ms. Fino a 14 s stereo int16.
- **BOBINA-D** (Rev. 2): stesso motore e parametri di BOBINA, compilato per lo slot DELAY: il loop passa nel riverbero; con BOBINA nel REVERB si hanno due looper. `bobina.h` è identico nelle due cartelle: le modifiche al motore vanno copiate in entrambe.
- **RICORDO-S** (Rev. 2): variante di RICORDO: ERA TUBO (2 comb per lato = tubo metallico, SIZE = 1,5–9 ms; AGE = shimmer, pitch shifter all'ottava sopra nell'anello), LOWCUT (passa alto 2 poli 20 Hz–1,2 kHz) al posto di GRAIN, DECAY 0,5–100 s con calibrazione a tabella (entro 6% a 100 s). In TUBO niente passa alto, cadute e secondo polo dell'invecchiamento.
- **RICORDO** (Rev. 2): riverbero FDN 8 linee (Hadamard) con catena di invecchiamento nell'anello: la coda si consuma a ogni ricircolo. DECAY 0,5–30 s (calibrato entro il 2%), AGE, MIX, ERA (NASTRO/VINILE/RADIO/CERA), SIZE, TONE, WOW, GRAIN, HISS (proporzionale alla coda), SINK (spostatore di frequenza: la coda scende), FREEZE. Rumore aggiunto prima dei filtri dell'epoca (a 2 poli), limitatore trasparente sotto 0,6. v0.2: FREEZE con controllo automatico di livello (memorizza il livello dopo 50 ms e lo mantiene, guadagno 0,85–1,3), saturazione cubica senza perdita di livello (la v0.1 comprimeva ~15% per giro), crepitii proporzionali alla coda.
- **RADIO-W** (Rev. 2): variante di RADIO con WHISTLE (volume del fischio: 0 = spento, 40% = come RADIO, 100% ≈ 2,3×) al posto di HUM; ronzio a 50 Hz automatico proporzionale ad AGE. Da confrontare con RADIO e tenerne una sola.
- **LOLA** (Rev. 2): oscillatore che abbaia, per sintesi, dedicato a Lola (bassotta a pelo duro, 7 mesi). SIZE (25% = Lola, ~630 Hz), ROUGH, CALL (BARK/YIP/ARF/GROWL/WHINE/HOWL/MIX), REPEAT (OFF, 0,5–8 Hz, a tempo), PITCH (KEY/NATURAL, C4 = naturale), LENGTH, VARY, TONE, PANT (fiatone a tasto tenuto), CRACK (stecche all'ottava). Shape LFO → SIZE. Livelli compensati per verso. Idea futura: versione a campioni con i versi veri di Lola (motore MEE-NAH).
- **BOCCA** (Rev. 2): filtro a formanti stereo (3 SVF passa-banda per canale), vocali U-O-A-E-I in morphing continuo. VOWEL, DEPTH (formato template KORG modfx), MOD (LFO/RANDOM/ENV/TURING/LOOP/CHAT), RATE (0–49 libero 0,05–12 Hz, 50–60 a tempo), VOICE (×0,78–×1,38), RESO, GLIDE, DRIVE, BODY, MIX. Livello compensato per vocale e risonanza (uscita ≈ ingresso).
- **SUPERORBITAL** (Rev. 4): estensione di ORBITAL14 dell'utente (sorgente originale in `esterne/orbital14_originale/`). Modi originali conservati (confronto con il codice originale: 46–67 dB sotto il segnale). Nuovi: DOPPLER (4 satelliti: pan, distanza → livello e passa basso, Doppler con ritardo Hermite normalizzato sulla velocità: DEPTH = fino a ±100 cents) e ROTARY (tamburo bande 0-1 / tromba bande 2-3, 0,67/0,8 Hz ↔ 5,9/6,8 Hz, inerzia 3 s / 0,8 s, AM, Doppler, pan). Corretto un difetto di ORBITAL14: multipli non interi della fase (0,35/0,27/0,7) saltavano a ogni giro → accumulatori di fase separati. Limitatore morbido sopra 0,8. Coseno delle campane come 1−2·sin²(w/2) (le campane basse a Q alto sono sensibilissime alla precisione). SPEED/SPREAD sui knob, MODE nel menu.
- **SUPERORBITAL v0.2**: modi XPHASER (4–16 stadi da DEPTH, risonanza EQ AMT×0,95 con saturazione, sweep esponenziale ±3 ottave da MOTION, centro da SWEEP, coefficienti interpolati per campione, campane EQ che inseguono lo sweep a fm e 2,2·fm), XFLANGE/XJET (Hermite, feedback ±0,97, campane sui picchi del pettine 1/d e 2/d, o 1/2d e 3/2d). Segnale trattato nelle 4 bande con traiettorie ORBIT. Misure: XPHASER picchi +32 dB / tacche −24 dB (SUPERPHASER +16/−7); XFLANGE +31 dB (SUPERFLANGER +24). Compilata -Os con O2 su process/xfx (16,6 KB a -O2 non ci stava).
- **BOBINA v0.4**: ACTION a 4 valori discreti (0 CLEAR tenuto 1 s, 1 STOP, 2 PLAY, 3 REC/DUB: registra se vuoto, sovraincide se c'è il loop; uscire da 3 chiude all'istante; altre posizioni dopo 120 ms); SYNC con AUTO1/2/4/8 (stato IN ATTESA fino al primo suono > 0,03, chiusura automatica a N battute). Ipotesi da verificare: il firmware adatta la corsa del knob all'intervallo del parametro anche negli slot DELAY/REVERB (lo fa nel MOD, come in ORBITAL14).
- **FRAMMENTI-R** (Rev. 4): FRAMMENTI per lo slot REVERB. Piastra di Dattorro a 48 kHz (plate.h, ~40 k campioni in SDRAM) con shimmer all'ottava (pitch shifter a 2 testine nella vasca, passa basso ~6 kHz e riduzione del decadimento per la stabilità), ingresso + grani nella piastra, coda che rientra nei grani (non in TAPS), limitatore morbido sul ritorno, HOLD = grani + piastra congelati (shimmer escluso in freeze). SPACE = coda 0,5→0,95 (≈2,6–22 s). SHIMMER al posto di SPREAD (fisso 70%).
- **SUPERFLANGER** (Rev. 4): CLASSIC (BBD + feedback), TAPE (dry a 3 ms, wet 0–6 ms; COLOR NEG = copia invertita, cancellazione −25…−32 dB all'incrocio; AGE su entrambe le copie), BARBUP/BARBDN (3 testine a dente di sega, finestre sin² sfasate di 1/3, volume ±0,4 dB). Hermite a 4 punti, compensazione di livello del mix, SHAPE SINE/TRI/RANDOM/ENV.
- **SUPERPHASER** (Rev. 4): ensemble a 3 linee BBD (7 ms ± 1,9 ms a 0,63 Hz ± 0,22 ms a 6,4 Hz, fasi 0/120/240) → phaser 4–12 stadi passa-tutto del primo ordine con scostamenti tra stadi (fino a ±12%), sweep esponenziale (±2,6 ottave), COLOR = feedback con saturazione, WARMTH. Tacche = stadi/2 (verificato). Nome neutro ("Solina" è un marchio).
- Slot DELAY e REVERB: API identica (TIME/DPTH/MIX, 11 parametri, ~24 KB, 3 MB SDRAM, nessuna nota); cambiano la posizione nella catena e il tasto del mix nativo (DEL/REV).
- **RADIOLINA** (Rev. 4): oscillatore ispirato a GHOST RADIO dell'utente (che resta invariato), scritto da zero per una sola stazione e una sola frase ("ahimiamor", fornita dall'utente: 3,6 s, 8 kHz μ-law, SNR 38 dB). TUNE (stazione al centro, ±~0,05 della corsa; fuori stazione fischio di battimento 80 Hz + 9 kHz × scostamento e fruscio), EVERY (pausa 0,25 s × 2^(6,9·EVERY), ±40% casuale: da subito a ~30 s), SIGNAL, RADIO (HP 120→450 Hz e LP 6,5→2,6 kHz a 2 poli, picco +7 dB a 1,3 kHz Q 1,4, saturazione morbida), NOISE (fruscio, crepitii casuali, ronzio 50 Hz con armoniche, scariche), DRIFT (deriva della sintonia, QSB, tremolio della voce), GHOST (eco decimata 4:1, ~140 ms modulata), KEY AUTO/TRIGGER/PITCH. Consumo 0,39 s. Se la frase viene da un disco: uso personale; per pubblicare serve il permesso.
- **ARMONIA** (Rev. 4): 2 voci × (principale + raddoppio scordato ±DETUNE/2, fino a 30 cents, opposto tra i canali × STEREO) × 2 canali = 8 trasposti a due testine (finestre sin² da tabella, somma 1). Aggancio alla ripartenza della testina (peso zero): 24 posizioni in 240 campioni, correlazione normalizzata su 48 campioni (uno su due) con il segmento dell'altra testina. INTERV 0–48 (24 = unisono; i knob mostrano sempre il numero), VOICE2 OFF/−24…24, SCALE come TELAIO, SHIMMER = voci nel buffer d'ingresso (scurite, saturazione morbida), GRANA 15–120 ms, REVERSE OFF/ON/MISTO, DRIFT fino a 35 cents (aggiornato ogni 64 campioni), MIX a potenza costante. Misure: intervalli ±2,5 cents; seno: ondeggiamento 22 → 3%, fuori nota ~99 → 2–3%; due note: fuori nota 38 → 22%. Consumo ~1,9 s (8 copie), ~0,85 s con VOICE2 OFF o DETUNE 0.
- **MAREA v0.2 / CAOS v0.2** (dopo la prova: "troppo leggere, poco carattere, i cambi di velocità non si sentono"). Misura di incisività (`test/incisivita_test.cc`: distanza dal dry, escursione 5–95% del colore e dello stereo su finestre di 50 ms, ritmo dominante). MAREA: I = AMOUNT·H (da zero), stazioni rinforzate, una stazione a ogni cresta, dissolvenza 0,12 + 0,2·P s, P = 0,5·2^(5·PERIODO), altezza d'onda per canale (l'onda attraversa lo stereo di ±1/8 di periodo × STEREO) e spostamento del wet che la segue; MIX 100%. Risultati: dry −12 → −3,4 dB, stereo 1–2 → ~10 dB, ritmo che segue PERIODO. CAOS: tempo 0,08·2^(8,5·SPEED), 8 sotto-passi, filtro ±6 ottave, ritardi più ampi, FLANGE con feedback negativo −0,92, panorama pieno, DTYPE RING (30 Hz–3 kHz) e DRIVE in coda. Risultati: dry −6 → −2 dB, colore 4 → 13–14 dB, stereo 7 → 26–27 dB, salti 0,07–27/s. Consumo MAREA 0,75 s (1,15 caso peggiore), CAOS 0,64 s.
- **STANZE v0.2**: DEPTH con lo stesso verso in tutte le modalità (a destra più corpo): nella famiglia chiara la corsa del passa alto è invertita rispetto all'originale (corpo 100 Hz–1 kHz in NEBBIA da −25 dB a DEPTH 0 a +2 dB a DEPTH 1). I valori salvati di DEPTH in NEBBIA/FOSCHIA si leggono rovesciati.
- **CAOS** (Rev. 4): attrattore di Lorenz (s = 10, b = 8/3), r = 28·2^((1−CHAOS)·3,644) da 350 (orbita periodica) a 28 (caos classico); RK2 con 4 sotto-passi ogni 16 campioni, scala dei tempi 0,05·2^(7·SPEED); normalizzazione x/(1,6·c), y/(2,2·c), (z−(r−1))/(0,55·r) con c = √(b(r−1)); x → panorama (le ali), y → SVF passa basso (100 Hz–12 kHz attorno a CUTOFF), z → ritardo Hermite (FLANGE 0,3–4 ms con feedback, CHORUS 5–23 ms, VIBRATO solo bagnato). Verifiche: CHAOS 0 periodico (correlazione 1,00), salti tra le ali 323 → 40 in 3 min, stabile a SPEED massimo. Consumo 0,63 s.
- **MAREA** (Rev. 4): mare = onda lunga (periodo 1,5–24 s) + onda secondaria (0,61×) + moto ondoso (rumore lisciato) + onde anomale (probabilità alla cresta 0,04 + 0,25·MARE²). Altezza H → intensità I = AMOUNT·(0,25 + 0,75·H). Sette stazioni (FLANGE, CHORUS, RISACCA, SALE, RING, ROVINA, ECO) in tre insiemi (TUTTE/PULITE/SPORCHE); ROTA ONDA (cresta con H > 0,62 o anomala; anomala = salto di due), MAREA (una ogni 6 periodi), CASO, FISSO. Dissolvenza a potenza costante con smoothstep (≈ mezza onda). Solo le due stazioni attive vengono calcolate; ogni linea riceve sempre l'ingresso. ROVINA: danno accumulato (bit 16→4, sample-and-hold ×1–16, passa basso) che si ripulisce in ~30 s. Correzioni dai test: pesi della dissolvenza invertiti (stazione ferma = suono diretto, clic a fine passaggio), quantizzazione di ROVINA, livello di SALE. Consumo 0,49 s (0,79 caso peggiore).
- **TELAIO** (Rev. 4): registro di Turing (2–16 bit, bit uscente reinserito e invertito con probabilità 0,5·(1−LOCK)²) → valore del passo dagli ultimi 8 bit. Passi agganciati al clock 4ppqn del NTS-1 se presente (impulsi negli ultimi 1 s), altrimenti al tempo impostato. RATE 1BAR…32ND, SLIDE con costante di tempo ∝ durata del passo (min 1,2 ms), PITCH quantizzato su CHROM/MAJOR/MINOR/PENTA/OCTAVE (±12 semitoni), OFFSET come centro. Parte sonora di GESTO (SVF, panorama, trasposizione a due testine, volume). Consumo 0,23–0,52 s.
- **GESTO** (Rev. 4): curva del knob B registrata in battiti (64 punti per battito, fino a 8 misure) → loop a tempo (bpm da unit_set_tempo; cambia tempo e resta a tempo). ACTION LIVE/REC/PLAY: REC armato, parte al primo movimento > 2%, registra LENGTH misure e passa a PLAY (unit_get_param_value restituisce lo stato vero). DEST FILTER (SVF passa basso 120 Hz–12 kHz esponenziale, RESO), PAN, PITCH (due testine, ±12 semitoni), VOLUME, FILTPAN. DIR FWD/REV/PINGPONG, SPEED 0,25–4×, SMOOTH 1–300 ms, WEAR (quadratico: livellamento verso i vicini + tremolio + deriva al centro, modifica la curva). Consumo 0,30–0,73 s.
- **STORMO** (Rev. 4): 2–8 copie del suono = uccelli in un cielo 2D (x → panorama, legge sqrt(1±pan); y → ritardo 6–30 ms e passa basso TONE). Volo ogni 32 campioni: coesione, allineamento, separazione (raggio 0,4), posatoio cubico + muro morbido oltre 0,9, vagare lisciato, virata d'insieme lenta, falco (fuga 1/d², durata ~0,7 s in tempo reale). Ritardo con rampa per campione e limite di velocità (±0,03 campioni/campione, max ~50 cents). SYNC BEAT/BAR: velocità × bpm/120, falco sui battiti/misure dai tick 4ppqn. Hermite, rigenerazione con saturazione morbida, wet 1,5/√N. Consumo 0,89 s (8 uccelli).
- **RADIOLINA v0.5** (dalla v0.4): frase B −10 dB in riproduzione (misura: era +8,7 dB nella banda della radio; ora −23,0 / −22,9 dB); KEY e PHRASE uniti in PLAY (12 valori, iniziale AUTO-ALT) per liberare il posto di PAD (volume degli accordi, guadagno ∝ PAD², 70% = +2,9 dB rispetto alla v0.4); CHORDS a 21 valori (OFF + 4 modi × FIX/1/2/4/8 BAR; FIX = tonica ferma); pesi della progressione ristretti a I/IV/V/VI (II raro, III e VII esclusi). 48,1 KB (resta ~1 KB). Consumo 0,75 s.
- **RADIOLINA v0.4 (alternativa alla v0.3)**: stesso motore della v0.3; frase B alzata di +1,65 semitoni con vocoder WORLD (pyworld: harvest/cheaptrick/d4c, timbro conservato) per portarla da Fa (+19 cents) a Sol come la frase A (−11 cents); verificato sull'uscita decodificata dell'unità (Sol, −11 cents, somiglianza con A +0,69). La v0.1 è stata tolta dal pacchetto su richiesta (la v0.2 suona uguale).
- **RADIOLINA v0.3**: parametro CHORDS (10°, 17 valori: OFF + MAJ/MIN/DOR/MIX × 1/2/4/8 BAR; un solo posto libero, quindi modo e tempo nello stesso parametro). Tonalità = ultima nota suonata (nuova nota → I grado). Progressione di Markov sui 7 gradi (pesi su 16), settima con probabilità 0,3, condotta delle voci a movimento totale minimo (24 assegnazioni, voci alte tra Mi3 e Mi5, basso tra Mi1 e Re#2), scivolamento ~20 ms. Pad: 5 voci × 2 seghe ±7 cents, passa basso 2 poli 500–1700 Hz che respira a 0,08 Hz, rigonfiamento al cambio, abbassamento −6 dB sotto la frase, livello ~−10 dB rispetto alla frase. Generatore casuale separato: CHORDS OFF identico alla v0.2 campione per campione. 47,9 KB (resta ~1,3 KB). Anteprima wav fornita su richiesta esplicita (fuori dal pacchetto).
- **RADIOLINA v0.2**: due frasi (A = "ahimiamor" 3,62 s; B = "ahimiamor frase 1" 4,31 s) in IMA ADPCM 4 bit a 8 kHz (31,7 KB), codificate con ricerca in avanti a 2 campioni (`test/adpcm_ricerca_in_avanti.c`: stesso formato, +1,5–2 dB; l'enfasi degli acuti non aiuta). SNR A 29 dB, B 21 dB (fiato e riverbero). Decodifica in avanti nell'unità, verificata identica al riferimento. PHRASE A/B/ALT/RANDOM (iniziale ALT); TRIGGER+ALT: tasti bianchi A, neri B. Scartate: tutte e tre le frasi a 6 kHz; frase B a 8 bit (servirebbero ~62 KB). Consumo 0,63 s.
- **LOLA-D** (Rev. 4): motore di SEAGULL con 8 abbai (BAU1–6 pastore tedesco da BigSoundBank n. 0112 di Joseph Sardin; BAU7–8 da OpenGameArt "Dog barking mono" di Brandon Morris; entrambi CC0), puliti (HP 120 Hz, riduzione rumore), 8 kHz 8 bit (15,5 KB). BARK BAU1–8 / RANDOM; DISTANCE ritarata per abbai gravi (LP 7 kHz → 1,2 kHz, livello fino a −12 dB). Consumo 0,25 s.
- **SEAGULL** (Rev. 4): delay derivato da VOX INJECT v1.1 (delay decimato 2:1, iniezione a ogni ciclo con CHANCE). Versi di gabbiano reale da xeno-canto XC584347 (Stanislas Wroza, CC BY-NC-SA 4.0: citare l'autore, niente uso commerciale, derivati con la stessa licenza), puliti (HP 500 Hz, riduzione rumore ~16 dB), 6 versi a 10 kHz 8 bit (16,2 KB). Parametri: TIME, FEEDBACK, MIX (template), CALL (ADULT1-3/YOUNG1-3/RANDOM), INJECT, CHANCE, PITCH, DEGRADE, DISTANCE (LP 2 poli 7→2,4 kHz, livello fino a −13 dB, 2 allpass di diffusione), SPREAD (pan casuale per richiamo; ripetizioni al centro).
- **VOICES (MEE-NAH 0.5, ROMANZA 0.3)**: elenco MONO, CHOIR, HOLD2, HOLD3, HOLD4, OCTAVE…MIN7 (12 valori, mappati sui modi interni); le PARA sono state tolte. Preset vecchi: 2-4 → HOLD, accordi invariati, vecchi 12-14 da reimpostare.
- **HOLD (MEE-NAH 0.4, ROMANZA 0.2)**: le note si accumulano anche dopo il rilascio e dopo aver rilasciato tutto; oltre il limite la nuova sostituisce la più vecchia (per ordine di accensione); una nota già accesa riparte senza doppioni; in HOLD le voci partono spente. LOLA è monofonica (nessuna parafonia).
- **ROMANZA** (Rev. 4): motore di MEE-NAH v0.3 con banco a 8 kHz (periodi ×2, f0 minima 63 Hz) e frammenti tagliati sulle pause tra le parole (6 parole + 2 vocali, 4,0 s); consonanti con spaziatura naturale; default MODE SEQ, PLAY ONESHOT, JITTER 0. Comprensibilità misurata come somiglianza dell'inviluppo spettrale nel tempo: 0,86–0,94 alla altezza originale (PITCH MELODY), 0,62–0,88 trasposta. Banco rigenerabile con `build_bank_parole.py`. Sorgente: traccia vocale di un disco commerciale (uso personale; licenza per pubblicare).
- **MEE-NAH v0.3**: aggiunte VOICES HOLD2/3/4 (in coda all'elenco: l'accordo resta anche dopo il rilascio, finché non se ne suona uno nuovo; pensato per l'inviluppo aperto). FORMANT mostra i positivi senza segno.
- **MEE-NAH**: oscillatore vocale granulare TD-PSOLA (formanti preservate). Banco di 14 frammenti (5 vocali, 6 sillabe, 3 glissati) da `meenah.mp3`, 5,5 s a 6 kHz 8 bit μ-law (v0.2; 6,0 s nella v0.1). SCAN, SPEED (freeze…2×), MODE (FRAG/SEQ/RAND/BABBLE), PLAY, PITCH (KEY/MELODY, C4 = originale), FORMANT ±12, JITTER, VIBRATO, VOICES (v0.2: MONO, CHOIR, PARA 2/3/4 con furto della voce più vecchia, accordi OCTAVE/FIFTH/MAJOR/MINOR/SUS4/MAJ7/MIN7), TAPE. In parafonia il pitch bend si calcola sulla nota corrente con priorità all'ultima nota. Shape LFO → SCAN. Banco rigenerabile con `build_bank.py`.

## Lezioni apprese (vincoli verificati)
- Errore "wrong unit parameter: min or max or center": negli oscillatori usare min = 0 e center/default dentro l'intervallo; mostrare valori come 2–16 o ±2 con parametri di tipo strings. Nei delay i primi 3 parametri (TIME, DPTH, MIX −1000..1000 drywet) vanno lasciati come nel template KORG.
- Campioni comprensibili in un oscillatore: 8 kHz (banda telefonica) è il minimo per capire le consonanti; nei 48 KB ci stanno ~4 s. Gli effetti non possono contenere campioni (codice 16–24 KB, la SDRAM è vuota all'avvio).
- Commenti `//` a fine riga in una modifica automatica possono "mangiare" il codice che segue sulla stessa riga (STANZE: azzeramento dei filtri saltato → valori casuali). Dopo ogni modifica, controllare che le uscite siano deterministiche (due esecuzioni identiche).
- Ricostruzioni da misure: confrontare sempre lo spettro **assoluto** (rispetto a un riferimento interno dell'originale), non solo relativo; e verificare anche con note tenute, non solo con rumore e impulsi.
- **Valori salvati e unit ID**: il NTS-1 ricorda i valori dei parametri per unit ID. Se una nuova versione cambia il significato o l'ordine dei parametri mantenendo lo stesso ID, i vecchi valori vengono reinterpretati (RADIOLINA v0.5: KEY=AUTO → PLAY=AUTO-A, solo la prima frase; PHRASE=ALT → PAD=2%). Prima di cambiare i parametri: mantenere l'ordine e il significato, oppure dare un unit ID nuovo, e comunque avvisare di reimpostare i valori.
- Display, caratteri: sono mostrabili solo lettere, cifre, spazio e "-._". "/" e "+" fanno sparire la scritta (errore della Rev. 2, corretto in Rev. 3).
- Display, knob A e B: il firmware mostra sempre il numero (anche con parametri di tipo strings, verificato su BOBINA v0.2). Per dare un significato al numero si usa la parte frazionaria: con frac = 8 il valore mostrato è raw/256, quindi la cifra intera può indicare una zona (BOBINA v0.3).
- Display: le unità non possono scrivere sul display in modo permanente; il firmware mostra solo il valore (o la stringa) del parametro mentre lo si tocca. Display a 7 segmenti, ~4 caratteri.
- Slot REVERB (revfx): come delfx (TIME, DPTH, MIX fissi nel formato, 11 parametri, 3 MB SDRAM) ma **senza eventi di nota**: per un looper il trasporto va messo su un knob.
- Slot MOD (modfx): TIME e DPTH nel formato del template (0..1023, frac 1), nessun MIX fisso, max 10 parametri.
- Limiti NTS-1 mkII (README SDK): osc ~48 KB senza memoria esterna; modfx ~16 KB + 256 KB; delfx/revfx ~24 KB + 3 MB SDRAM. Massimo 10 parametri per osc, 11 per delfx/revfx.
- Simboli esterni: il firmware fornisce wt_sine_lut_f e midi_to_hz_lut_f. Le funzioni libm (powf, sinf, floorf…) aggiungono i simboli newlib `__sf_fake_stdin/stdout/stderr`: THALOS TURING V2.0 e ORBITAL V1.4 li contengono e funzionano sullo strumento, quindi il firmware li tollera (regola precedente troppo severa). Nella suite si continuano a evitare per il **consumo di calcolo**: si usano approssimazioni veloci (seno, 2^x, log2, coseno esatto per angoli piccoli nei filtri).

## Ambiente di compilazione
logue-sdk (github.com/korginc/logue-sdk) con submodule CMSIS, toolchain gcc-arm-none-eabi 13.2 (l'SDK indica la 10.3).
Ogni cartella va copiata in `logue-sdk/platform/nts-1_mkii/` e si compila con `make GCC_BIN_PATH=/usr/bin install`.
In `test/` ci sono i simulatori da PC usati per verificare intonazione, livelli, DC, clipping e CPU.

## Riscontri aperti (13/10/2026)
- BOBINA / BOBINA-D: trasporto riprogettato nella v0.4 (da provare). Le due versioni hanno lo stesso motore: cambia solo la posizione nella catena.
- ~~Parafonia (MEE-NAH, ROMANZA)~~ **risolto (13/10/2026)**: le modalità HOLD2/3/4 della v0.4 funzionano come richiesto (erano state provate solo le PARA).

## Da verificare sul dispositivo (Rev. 3)
1. **Carico del processore**: ripetere le combinazioni di unità che prima mandavano in crisi il NTS-1.
2. Scritte del display ora visibili: TURING SEQ (RATE, OCTAVE, TRIG), FRAMMENTI (PITCH), MEE-NAH (FORMANT), BOCCA (RATE), LOLA (REPEAT).
3. BOBINA / BOBINA-D v0.4: il knob A deve avere 4 posizioni (0–3) con il numero sul display; flusso 1→3→2→3→2→1; CLEAR tenendo 0 per 1 s; AUTO1–8.
4. MEE-NAH v0.4 / ROMANZA v0.2: HOLD verificato sul dispositivo; tastiera interna: più note insieme?; pitch bend in PARA/HOLD.
5. RICORDO v0.3 / RICORDO-S: FREEZE deve tenere il volume; scegliere quale tenere.
6. RADIO / RADIO-W: scegliere quale tenere (fischio legato a STATIC o regolabile con WHISTLE).
7. BOCCA, LOLA: carattere a orecchio.
7d. FRAMMENTI-R, SUPERFLANGER, SUPERPHASER, SUPERORBITAL: carattere a orecchio, CPU in combinazione con altre unità.
7m. ARMONIA: densità a orecchio, qualità del trasposto su accordi, carico CPU insieme a delay e riverbero.
7l. MAREA v0.2 e CAOS v0.2: incisività e velocità a orecchio; STANZE v0.2: verso di DEPTH.
7k. CAOS: CHAOS da regolare a imprevedibile, destinazioni, DTYPE a orecchio.
7j. MAREA: carattere delle stazioni a orecchio, ROTA ONDA/MAREA/CASO, ROVINA, CPU con altre unità.
7i. TELAIO: LOCK dal vivo, aggancio al clock del NTS-1 (sequencer/arpeggiatore), PITCH sulle scale.
7h. GESTO: REC al primo movimento, display che passa a PLAY, destinazioni, WEAR, a tempo cambiando bpm.
7g. STORMO: carattere del volo a orecchio, SYNC BAR con il falco, numero di uccelli, CPU con altre unità.
7f-quater. RADIOLINA v0.5: reimpostare PLAY (AUTO-ALT) e PAD (70%) se ereditati dalle versioni precedenti; equilibrio tra le frasi, PAD, CHORDS FIX, carattere delle progressioni brevi, PLAY.
7f-ter. RADIOLINA v0.3: CHORDS (modi, tempi, tonalità dalla nota), equilibrio pad/frase, carico CPU.
7f. RADIOLINA v0.2: PHRASE (ALT, RANDOM, bianchi/neri in TRIGGER); eventuale granulosità della frase B con NOISE 0, SIGNAL 100, RADIO bassa.
7f-bis. RADIOLINA: sintonia con TUNE, ritmo delle ripetizioni con EVERY, quantità di RADIO e NOISE a orecchio, KEY TRIGGER/PITCH.
7e. LOLA-D: carattere degli abbai a orecchio, PITCH per "rimpicciolire" il cane, DISTANCE.
7c. SEAGULL: DISTANCE e SPREAD a orecchio, livello dei gabbiani rispetto al delay.
7b. ROMANZA: comprensibilità delle parole a orecchio (PITCH MELODY a C4 e PITCH KEY), scelta dei frammenti.
8. TURING SEQ v1.3: preset v1.1 da ricontrollare (scale riordinate, GLIDE → OCTAVE).
9. BOBINA / BOBINA-D: memoria allocata (anche con entrambe caricate insieme); SYNC.
9b. STANZE: confronto a orecchio con i quattro riverberi originali (prove A/B), cambi di MODE dal vivo, coda congelata, carico CPU.
10. Ricordarsi di eliminare TURING SEQ v1.0 (dev ID diverso) se ancora caricata.

## Prossimi passi
- FRAMMENTI v0.2 (facoltativa, la v0.1 va bene così): eventuali modi ARP, SHARD (glitch a blocchi), WARP (pitch che scivola a nastro).
- MEE-NAH: eventuale sostituzione di frammenti del banco (indicare numero + punto del file); `build_bank.py voce.mp3 5.55` per restare nei 48 KB con la parafonia.
- Riverberi proposti da fare: CORDE (coda intonata su scala/accordo, con registro Turing), riverbero granulare.
- Idee di modulazione: completato il trio TELAIO, MAREA, CAOS. ARMONIA (armonizzatore denso) costruita.
- STANZE: eventuale indagine sulla liscezza dell'originale sotto 200 Hz (differenza aperta).
- BOBINA: eventuale undo dell'ultimo overdub, loop multipli.
- RADIO: eventuale versione per lo slot REVERB (la radio che trasmette un suono già con eco).
- Altre modulazioni proposte: CAOS (LFO da attrattore di Lorenz), REGISTRO (modulazione Turing), FREQ SHIFT barberpole, VIBRATO A VALVOLE.
- Altre unità proposte: COMBO (organo combo anni '60), FISARMONICA, MANDOLINO (oscillatori); TAMBURO (eco a tamburo magnetico, delay); MOLLA (riverbero a molla).
- Altre idee in lista: risonatore modale, oscillatore ad accordi/strum, euclideo percussivo, effetto tape, ring mod/decimatore con registro Turing, riverbero shimmer/freeze.
