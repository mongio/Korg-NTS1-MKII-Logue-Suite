GHOST RADIO V0.3 — distinct stations and embedded spoken PCM snippets
Independent project. DRONE is NOT touched.

Parameters: TUNE 0..100 (six station centers at 9,25,40,56,73,90), REALITY, SIGNAL, DRIFT, GHOST, SCAN 0..100 (0 off; 1-100 sweep speed).
Three offline-generated eSpeak English speech clips: 'station seven calling', 'do you copy', 'signal lost'. Rendered to 8kHz 8-bit PCM in voice_samples.h. They are actual embedded playback samples, not recordings of humans.

Copy folder to logue-sdk/platform/nts-1_mkii/GHOST_RADIO_V03.
From its PowerShell directory:
$base=(Resolve-Path '..\..\..\tools\gcc\gcc-arm-none-eabi-10.3-2021.10').Path
$env:CPLUS_INCLUDE_PATH="$base\arm-none-eabi\include\c++\10.3.1\arm-none-eabi\thumb\v7e-m+fp\hard"
make
make install
Output: ghost_radio_v03.nts1mkiiunit

Not hardware tested. Please report build errors and actual sonic behavior.
