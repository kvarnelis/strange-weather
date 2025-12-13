# Strange Weather

A chaotic CV generator for VCV Rack based on strange attractors.

## Overview

Strange Weather generates continuously evolving control voltages using mathematical chaos. Three independent attractor banks produce deterministic but unpredictable modulation — not random, but impossible to predict.

## Features

- **3 Independent Banks** — Each runs its own strange attractor
- **4 Attractor Types** — Lorenz, Rössler, Thomas, Halvorsen
- **16 CV Outputs** — 4 per bank + 4 combined
- **Real-time Visualization** — Watch the attractors evolve
- **Extreme Rate Range** — From 20-minute cycles to audio rate

## Controls

### Per Bank (A, B, C)
- **RATE** — Attractor speed (audio rate ↔ ~20 minutes)
- **SHAPE** — 4-position switch selecting attractor type

### Outputs Per Bank
- **x** — Attractor x coordinate (±5V)
- **y** — Attractor y coordinate (±5V)
- **z** — Attractor z coordinate (±5V)
- **Σ** — Sum: x + y + z

### Combined Outputs
- **Σ** — Sum of all bank sums
- **|Σ|** — Rectified sum (all absolute values)
- **−Σ** — Inverted sum
- **INV** — Inverse distance: 5V minus absolute values

### Display
- **CYCLE** — Button cycles through 5 views: A, B, C, Combined, All

## Attractor Types

- **Lorenz** — The original (1963). Two-lobed butterfly. Good fold-over behavior.
- **Rössler** — Asymmetric spiral with occasional large excursions. Gentler.
- **Thomas** — Cyclically symmetric. Smooth, rolling motion.
- **Halvorsen** — Sculptural, aggressive. Sharp transitions.

## Building

```bash
export RACK_DIR=/path/to/Rack-SDK
make
make install
```

## Known Issues / TODO

- CKSSFour (4-position switch) doesn't exist in standard VCV component library — needs custom widget or alternative
- Cycle button needs proper param-less implementation
- Output scaling may need tuning per attractor type
- DC blocking filter not yet implemented

## License

GPL-3.0-or-later

## Acknowledgments

- Edward Lorenz — for discovering deterministic chaos
- Worng Electronics Vector Space — output transform inspiration
- Synthesis Technology E352 — interface reference
