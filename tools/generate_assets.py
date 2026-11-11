#!/usr/bin/env python3
import os
import math
import struct
import random

SAMPLE_RATE = 22050

def make_note(freq, duration_sec, amplitude=0.4):
    """Synthesizes a warm acoustic piano/chime note with exponential decay."""
    num_samples = int(SAMPLE_RATE * duration_sec)
    samples = []
    decay_rate = 3.0 / duration_sec
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        env = math.exp(-decay_rate * t)
        # Fundamental + gentle harmonics (piano timbre)
        val = (math.sin(2.0 * math.pi * freq * t) * 0.65 +
               math.sin(4.0 * math.pi * freq * t) * 0.20 +
               math.sin(6.0 * math.pi * freq * t) * 0.10 +
               math.sin(8.0 * math.pi * freq * t) * 0.05)
        sample = int(val * env * amplitude * 32767.0)
        # Clamp
        if sample > 32767: sample = 32767
        if sample < -32768: sample = -32768
        samples.append(sample)
    return samples

def generate_ambient_track(filename, duration_sec, scale_freqs, chord_progressions):
    """Generates an ambient relaxing piano/drone music track."""
    total_samples = int(SAMPLE_RATE * duration_sec)
    buffer = [0.0] * total_samples

    # Background ambient drone (warm continuous pad)
    drone_freq = scale_freqs[0] / 2.0  # Deep root note
    for i in range(total_samples):
        t = i / SAMPLE_RATE
        # LFO modulation
        lfo = 0.5 + 0.5 * math.sin(2.0 * math.pi * 0.08 * t)
        drone = (math.sin(2.0 * math.pi * drone_freq * t) * 0.08 +
                 math.sin(2.0 * math.pi * (drone_freq * 1.5) * t) * 0.05) * lfo
        buffer[i] += drone

    # Play sparse piano chords & notes
    t_cursor = 1.0
    random.seed(42)

    while t_cursor < duration_sec - 6.0:
        chord = random.choice(chord_progressions)
        # Play chord notes with slight strum delay
        for note_idx in chord:
            freq = scale_freqs[note_idx % len(scale_freqs)]
            note_dur = random.uniform(3.5, 6.0)
            note_samples = make_note(freq, note_dur, amplitude=0.25)
            start_sample = int(t_cursor * SAMPLE_RATE)

            for j, s in enumerate(note_samples):
                if start_sample + j < total_samples:
                    buffer[start_sample + j] += (s / 32767.0)

            t_cursor += random.uniform(0.1, 0.35) # strum spacing

        # Pause between phrases
        t_cursor += random.uniform(4.0, 7.5)

    # Convert buffer to 16-bit signed PCM
    with open(filename, 'wb') as f:
        for val in buffer:
            # Soft master limiter
            if val > 0.95: val = 0.95
            if val < -0.95: val = -0.95
            sample_i16 = int(val * 32767.0)
            f.write(struct.pack('<h', sample_i16))

def generate_sfx(filename, sfx_type):
    """Generates 16-bit PCM sound effects."""
    samples = []
    if sfx_type == 'step_grass':
        dur = 0.12
        n = int(SAMPLE_RATE * dur)
        for i in range(n):
            t = i / SAMPLE_RATE
            env = math.exp(-25.0 * t)
            noise = random.uniform(-1.0, 1.0) * 0.4
            thud = math.sin(2.0 * math.pi * 90.0 * t) * 0.6
            samples.append(int((noise + thud) * env * 24000.0))

    elif sfx_type == 'step_stone':
        dur = 0.08
        n = int(SAMPLE_RATE * dur)
        for i in range(n):
            t = i / SAMPLE_RATE
            env = math.exp(-40.0 * t)
            clack = (math.sin(2.0 * math.pi * 380.0 * t) * 0.5 +
                     random.uniform(-1.0, 1.0) * 0.5)
            samples.append(int(clack * env * 26000.0))

    elif sfx_type == 'dig':
        dur = 0.18
        n = int(SAMPLE_RATE * dur)
        for i in range(n):
            t = i / SAMPLE_RATE
            env = math.exp(-18.0 * t)
            crunch = random.uniform(-1.0, 1.0) * 0.7 + math.sin(2.0 * math.pi * 120.0 * t) * 0.3
            samples.append(int(crunch * env * 28000.0))

    elif sfx_type == 'place':
        dur = 0.15
        n = int(SAMPLE_RATE * dur)
        for i in range(n):
            t = i / SAMPLE_RATE
            env = math.exp(-20.0 * t)
            thump = math.sin(2.0 * math.pi * 85.0 * t) * 0.8 + random.uniform(-0.5, 0.5) * 0.2
            samples.append(int(thump * env * 29000.0))

    elif sfx_type == 'pop':
        # Item pickup chirp: frequency sweep 450Hz -> 1100Hz
        dur = 0.09
        n = int(SAMPLE_RATE * dur)
        for i in range(n):
            t = i / SAMPLE_RATE
            freq = 450.0 + (1100.0 - 450.0) * (t / dur)
            env = math.sin(math.pi * (t / dur))
            val = math.sin(2.0 * math.pi * freq * t)
            samples.append(int(val * env * 28000.0))

    elif sfx_type == 'click':
        dur = 0.04
        n = int(SAMPLE_RATE * dur)
        for i in range(n):
            t = i / SAMPLE_RATE
            env = math.exp(-60.0 * t)
            tick = math.sin(2.0 * math.pi * 800.0 * t)
            samples.append(int(tick * env * 26000.0))

    elif sfx_type == 'craft':
        # Success chime: two harmonic notes
        dur = 0.35
        n = int(SAMPLE_RATE * dur)
        for i in range(n):
            t = i / SAMPLE_RATE
            env = math.exp(-8.0 * t)
            chime = (math.sin(2.0 * math.pi * 523.25 * t) * 0.5 +   # C5
                     math.sin(2.0 * math.pi * 659.25 * t) * 0.5)    # E5
            samples.append(int(chime * env * 28000.0))

    with open(filename, 'wb') as f:
        for s in samples:
            if s > 32767: s = 32767
            if s < -32768: s = -32768
            f.write(struct.pack('<h', s))

def main():
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    nitro_dir = os.path.join(base_dir, "nitrofiles")
    music_dir = os.path.join(nitro_dir, "music")
    sfx_dir = os.path.join(nitro_dir, "sfx")

    os.makedirs(music_dir, exist_ok=True)
    os.makedirs(sfx_dir, exist_ok=True)

    print("Generating sound effects...")
    sfx_list = ['step_grass', 'step_stone', 'dig', 'place', 'pop', 'click', 'craft']
    for sfx in sfx_list:
        path = os.path.join(sfx_dir, f"{sfx}.raw")
        generate_sfx(path, sfx)
        print(f"  -> {path} ({os.path.getsize(path)} bytes)")

    # Pentatonic notes for ambient soundtrack (C4, D4, E4, G4, A4, C5, D5, E5)
    c_pentatonic = [261.63, 293.66, 329.63, 392.00, 440.00, 523.25, 587.33, 659.25]
    progressions_1 = [
        [0, 2, 4],    # C - E - A (Am7 feel)
        [2, 4, 5],    # E - A - C
        [0, 3, 5],    # C - G - C
        [1, 3, 6],    # D - G - D
    ]

    print("\nGenerating Ambient Calm Soundtrack (~5.7 MB)...")
    track1_path = os.path.join(music_dir, "calm.raw")
    # 130 seconds * 22050 * 2 = ~5,733,000 bytes
    generate_ambient_track(track1_path, 130.0, c_pentatonic, progressions_1)
    print(f"  -> {track1_path} ({os.path.getsize(track1_path)} bytes)")

    # F major pentatonic for second track (F3, G3, A3, C4, D4, F4, G4, A4)
    f_pentatonic = [174.61, 196.00, 220.00, 261.63, 293.66, 349.23, 392.00, 440.00]
    progressions_2 = [
        [0, 2, 4],    # F - A - D (Dm feel)
        [1, 3, 5],    # G - C - F
        [0, 3, 6],    # F - C - G
        [2, 4, 7],    # A - D - A
    ]

    print("\nGenerating Ambient Dusk Soundtrack (~5.7 MB)...")
    track2_path = os.path.join(music_dir, "dusk.raw")
    # 130 seconds * 22050 * 2 = ~5,733,000 bytes
    generate_ambient_track(track2_path, 130.0, f_pentatonic, progressions_2)
    print(f"  -> {track2_path} ({os.path.getsize(track2_path)} bytes)")

    total_size = sum(os.path.getsize(os.path.join(root, file))
                     for root, _, files in os.walk(nitro_dir)
                     for file in files)
    print(f"\nTotal NitroFS Asset Size: {total_size / (1024 * 1024):.2f} MB")

if __name__ == '__main__':
    main()
