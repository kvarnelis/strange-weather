# Strange Weather

A chaotic CV generator for VCV Rack 2 based on strange attractors.

![Strange Weather 2.0](strange-weather-2.0.jpg)
## Overview

Strange Weather generates continuously evolving control voltages using mathematical chaos. Four independent attractor banks produce deterministic but unpredictable modulation — not random, but impossible to predict.

## Features

- **4 Independent Banks** — Each runs its own strange attractor
- **4 Attractor Types** — Sprott B, Rossler, Thomas, Dadras
- **20 CV Outputs** — 4 per bank + 4 combined
- **Real-time Visualization** — Watch the attractors evolve
- **3 Display Modes** — Trace (lines), Lissajous (phosphor dots), Scope (waveforms)
- **3D Display Mode** — See the full three-dimensional structure with rotation
- **Adjustable Trail Length** — Control how much history is displayed
- **Extreme Rate Range** — From 20-minute cycles to sub-second modulation

## Installation

### From Release

1. Download the latest release for your platform
2. Extract the zip file
3. Copy the `StrangeWeather` folder to your VCV Rack 2 plugins directory:

| Platform | Plugins Directory |
|----------|-------------------|
| macOS (Apple Silicon) | `~/Library/Application Support/Rack2/plugins-mac-arm64/` |
| macOS (Intel) | `~/Documents/Rack2/plugins/` |
| Windows | `%USERPROFILE%\Documents\Rack2\plugins\` |
| Linux | `~/.Rack2/plugins/` |

4. Restart VCV Rack

### Building from Source

Requires [VCV Rack SDK](https://vcvrack.com/manual/Building#Building-Rack-plugins) v2.x.

```bash
git clone https://github.com/kvarnelis/strange-weather.git
cd strange-weather
make RACK_DIR=/path/to/Rack-SDK
```

To install after building:

```bash
# macOS (Apple Silicon)
mkdir -p ~/Library/Application\ Support/Rack2/plugins-mac-arm64/StrangeWeather
cp plugin.dylib plugin.json ~/Library/Application\ Support/Rack2/plugins-mac-arm64/StrangeWeather/
cp -r res ~/Library/Application\ Support/Rack2/plugins-mac-arm64/StrangeWeather/

# macOS (Intel) / Linux / Windows - adjust path accordingly
```

## Controls

### Per Bank (A, B, C, D)

| Control | Function |
|---------|----------|
| **RATE** | Attractor evolution speed within selected range |
| **RNG** | Range selector: Low (5-20 min), Med (1s-2min), High (0.1-10s) |
| **SHAPE** | Attractor type: Sprott B, Rossler, Thomas, Dadras |
| **VOLT** | Output voltage: +/-5V, +/-10V, 0-5V, 0-10V |
| **CHAOS** | Primary chaos parameter - affects attractor behavior |

### Outputs Per Bank
- **x, y, z** — Attractor coordinates (scaled per VOLT setting)
- **SUM** — x + y + z

### Combined Outputs
- **SUM** — Sum of all bank sums
- **RECT** — Rectified (absolute values)
- **INV** — Inverted sum
- **DIST** — Inverse distance from center

### Display Controls
- **CYCLE** — Cycles through views: A, B, C, D, Combined, All
- **MODE** — Cycles display style: Trace (lines), Lissajous (tiny dots), Scope (time-based waveforms)
- **3D** — Toggles 3D rotation view
- **TRAIL** — Adjusts trail length from ~1 second to ~68 seconds of history

## Attractor Types

| Type | Character |
|------|-----------|
| **Sprott B** | Minimal chaotic system with robust, reliable chaos |
| **Rossler** | Asymmetric spiral with occasional large excursions |
| **Thomas** | Cyclically symmetric, smooth rolling motion |
| **Dadras** | Multi-wing attractor with complex dynamics |

## License

GPL-3.0-or-later


