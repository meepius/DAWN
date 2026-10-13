# demo.dawn — launch session

Working title: **PUBLIC ENEMY, PRIVATE NETWORK**

This is the project DAWN should open the first time, before the user has saved anything. It is not a tutorial song. It is a finished 16-bar threat that happens to touch every part of the program a new user needs to trust.

## Reference

League of Legends' HEARTSTEEL, if the vocal had grown up in a club that still had a distorted 909 and a broken fluorescent tube. Pop-rap pocket, industrial EBM body. Think a swung trap hat pattern welded to a straight industrial kick, a bass that is mostly a saw being crushed, and a lead that answers one bar late because the delay is doing the harmony.

Mood: night bus, cheap chrome, expensive chorus. Confident, not cute. No lo-fi. No pad that exists only to fill a spectrum analyzer.

## Session

| | |
|---|---|
| Tempo | 142 BPM |
| Key | F minor |
| Loop | 8 bars in the session, 16 bars written in the arrangement |
| Quantize | 1 bar, so scene launches land on the downbeat |
| Master | just into the compressor, meters moving, not a sausage |

Scenes, top to bottom:

1. **IDLE** — hat and vocal chop only. The city before the drop.
2. **RIOT** — kick, snare, bass, chop. The hook.
3. **OVERLOAD** — RIOT plus the delayed lead and a noise sweep on the return.

Launching a scene is the demo. The audience should see clips go queued, then live, on the bar line.

## Tracks

### 1. KICK / DRUMS — Drum rack

One drum rack. Not five audio clips of loops pretending to be a beat.

- Kick on C1: short analog click into a sine, then a waveshaper. On the grid. Industrial, not 808-long. Tuned to F.
- Snare on D1: clap plus a noise burst, swung an 8th-note triplet against the straight kick. This is the HEARTSTEEL lean.
- Hat on F#1: closed hat, 16ths, every other one delayed 20 ms so the bar lurches.
- Break on G#1: a two-beat chopped amen slice, only in bars 7–8 and only in OVERLOAD. It should feel stolen, not decorative.

Route the rack to a drum bus. EQ: high-pass the hat at 180 Hz so it stops fighting the bass. Compressor on the bus, 4:1, slow enough that the kick's transient gets out.

### 2. BASS — Analog synth, audio not required

A single saw, two more an octave down, filter almost closed, envelope doing the talking.

- Pattern: F1 on the 1, C2 on the "and" of 2, Ab1 held across beat 4. Leave beat 3 empty. The hole is the hook.
- Device chain: analog synth, then a compressor with a fast release so the notes pump, then a hard clip. The clip is the sound. Do not hide it.
- No chorus. Chorus would make it polite.

This is the track that proves the built-in synth is an instrument and not a placeholder oscillator.

### 3. LEAD — Analog synth, pre-fader send to Delay

A higher voice, F minor pentatonic, mostly F4–C5–Eb5, one note per bar, off the downbeat by a 16th so it never sits on the kick.

- Delay: dotted 8th, feedback high enough that the second repeat is a chord tone, low enough that bar 4 is clean before the scene change.
- Send, not an insert. The dry note stays in the punch. The repeats live on the return, where the reverb can have them without washing the vocal.

Show the return strip in the session view while this clip is live. The delay tails are also the reason a plugin pull has to fade instead of click: if someone yanks the delay mid-phrase during the demo, the tail has to finish.

### 4. CHOP — Audio track

One recorded phrase, a human voice saying something short and mean. Chop it to 8 slices on the transients. Repitch three of them up a 5th. Do not autotune the rest. The grain of the voice is the point.

- Comp lanes: three passes. Lane 1 is the safe read. Lane 2 rushes the second word. Lane 3 is quieter and closer. The comp uses lane 3 for the first two beats of bar 1, lane 2 for the answer, lane 1 for the tail. The piano-roll / audio editor shows the lanes. The playhead only ever hears the comp. Say that out loud while you do it. That is the feature.
- Clip note in `userNotes`: "use the rushed word. the polite take is a lie."

### 5. AIR — Return

A short room, mostly early reflection, decay under 0.8 s. The chop sends a little. The lead's delay return sends a little more. The bass sends nothing. If the bass is in the room, the mix is already over.

## Arrangement

Bars 1–8 are IDLE into RIOT, the way a session recording would have printed them. Bars 9–12 drop the kick and leave the chop and the hat, so the visualizer has a thin signal and the FFT is not a solid wall. Bars 13–16 are OVERLOAD, then a one-bar tail where only the delay return is still speaking. Bounce that 16 bars. The WAV and the live output have to match, because that is the test the exporter is for.

## What the first launch should prove

- Scene launch quantizes.
- The drum rack, the analog bass, the analog lead, the delay, the compressor, and the EQ are all doing an audible job.
- The vocal comp is visible as lanes and audible as one part.
- The oscilloscope moves on the hat. The spectrum jumps when the bass enters.
- `userNotes` on the chop clip is already filled in, so the inspector is not an empty box.
- The project saves as `.dawn` and exports a `.dawproject` with the chop's audio stored relatively, next to the project, not as a path into someone else's home directory.

If a first-time user can launch RIOT, comp the word, and bounce the 16 bars without making an account, the demo did its job.
