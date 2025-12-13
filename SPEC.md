# Strange Weather

## CV Generator Module for VCV Rack

A chaotic CV generator based on strange attractors. Three independent attractor banks produce continuously evolving, deterministic-but-unpredictable control voltages. No randomness — pure mathematical chaos.

---

## Concept

Strange Weather takes its name from Edward Lorenz's 1963 discovery of deterministic chaos while modeling weather systems. The equations are simple and fully determined, yet long-term prediction is impossible. Small differences in initial conditions lead to completely different trajectories — the butterfly effect.

Unlike random voltage sources (Buchla 266, etc.), Strange Weather produces voltages that are:
- Deterministic (same equations, same evolution)
- Sensitive to initial conditions (practically unpredictable)
- Bounded (stay within the attractor basin)
- Continuous (no steps or discontinuities)
- Correlated across outputs (x, y, z move together meaningfully)

The result is organic, evolving modulation that never repeats but never feels random.

---

## Architecture

### Three Independent Banks (A, B, C)

Each bank runs its own strange attractor continuously. The attractor's x, y, z state variables become CV outputs. Banks do not interact.

### Combined Section

Four outputs derived from all three banks together, using Vector Space-style transforms.

---

## Interface

### Screen (top left)
- Real-time XY plot visualization of attractor trajectories
- No menu, no interaction — pure monitoring
- Display cycle button steps through 5 modes:
  1. Bank A
  2. Bank B
  3. Bank C
  4. Combined
  5. All four views simultaneously

### Panel Layout (horizontal banks)

```
┌────────────────────────────────────────────────────────────┐
│ ┌─────────┐                                                │
│ │         │  [RATE A]  [SHAPE A]   (x) (y) (z) (Σ)        │
│ │ SCREEN  │                                                │
│ │         │  [RATE B]  [SHAPE B]   (x) (y) (z) (Σ)        │
│ └─────────┘                                                │
│   [CYCLE]    [RATE C]  [SHAPE C]   (x) (y) (z) (Σ)        │
│                                                            │
│              COMBINED              (1) (2) (3) (4)        │
└────────────────────────────────────────────────────────────┘
```

### Controls

**Per Bank (×3):**

| Control | Type | Function |
|---------|------|----------|
| RATE | Knob | Attractor time scale. Full CCW = ~20 minute cycle. Full CW = audio rate. Center detent at ~1 Hz. |
| SHAPE | 4-position switch | Attractor type selection (see below) |

**Display:**

| Control | Type | Function |
|---------|------|----------|
| CYCLE | Momentary button | Steps through 5 display modes |

---

## Outputs

### Per-Bank Outputs (×3 banks = 12 outputs)

| Output | Signal |
|--------|--------|
| x | Attractor x state variable, scaled to ±5V |
| y | Attractor y state variable, scaled to ±5V |
| z | Attractor z state variable, scaled to ±5V |
| Σ | Sum: x + y + z |

### Combined Outputs (4 outputs)

| Output | Signal |
|--------|--------|
| 1 | Sum of bank sums: AΣ + BΣ + CΣ |
| 2 | Rectified sum: \|AΣ\| + \|BΣ\| + \|CΣ\| |
| 3 | Inverted sum: −(AΣ + BΣ + CΣ) |
| 4 | Inverse distance: 5V − \|AΣ\| − \|BΣ\| − \|CΣ\| |

**Total: 16 outputs**

---

## Attractor Shapes

Each bank can run one of four attractor types:

### 1. Lorenz
The original (1963). Two-lobed butterfly shape. Good fold-over behavior — tends to orbit one lobe then flip to the other unpredictably.

```
dx/dt = σ(y − x)
dy/dt = x(ρ − z) − y
dz/dt = xy − βz

σ = 10, ρ = 28, β = 8/3
```

### 2. Rössler
Asymmetric single spiral with occasional large excursions. Gentler than Lorenz — spends more time in smooth orbits.

```
dx/dt = −y − z
dy/dt = x + ay
dz/dt = b + z(x − c)

a = 0.2, b = 0.2, c = 5.7
```

### 3. Thomas
Cyclically symmetric. All three axes behave similarly. Smooth, rolling motion.

```
dx/dt = sin(y) − bx
dy/dt = sin(z) − by
dz/dt = sin(x) − bz

b = 0.208186
```

### 4. Halvorsen
Sculptural, aggressive. Sharper transitions and more extreme excursions than the others.

```
dx/dt = −ax − 4y − 4z − y²
dy/dt = −ay − 4z − 4x − z²
dz/dt = −az − 4x − 4y − x²

a = 1.89
```

---

## Output Scaling

All attractor state variables are normalized to ±5V range regardless of the natural scale of each attractor type. The module internally tracks the attractor's bounding box and rescales continuously.

Combined outputs may exceed ±5V when multiple banks align. Maximum theoretical range on combined outputs is approximately ±15V, but typical operation stays within ±10V.

---

## Rate Scaling

The RATE knob controls the integration time step, affecting how fast the attractor evolves:

| Knob Position | Approximate Cycle Time |
|---------------|------------------------|
| Full CCW | ~20 minutes |
| 9 o'clock | ~2 minutes |
| Center (detent) | ~1 second |
| 3 o'clock | ~10 ms |
| Full CW | Audio rate (~1 kHz+) |

At audio rates, attractor outputs become complex waveforms suitable for use as oscillator sources or audio-rate modulation.

---

## Display Modes

The screen shows XY phase plots (x horizontal, y vertical) with fading trails.

| Mode | Display |
|------|---------|
| 1 - Bank A | Single large plot of attractor A |
| 2 - Bank B | Single large plot of attractor B |
| 3 - Bank C | Single large plot of attractor C |
| 4 - Combined | XY plot using combined outputs 1 & 2 |
| 5 - All | Four quadrants showing A, B, C, Combined simultaneously |

Trail length is fixed. Older points fade to background. Current position shown as bright point.

---

## Technical Notes

### Implementation
- Attractors computed using 4th-order Runge-Kutta integration
- Internal state maintained at double precision
- Output scaling uses rolling min/max with slow decay to handle varying attractor bounds
- DC blocking filter on outputs (very slow HPF, ~0.01 Hz) to prevent stuck offsets

### Initial Conditions
- Each bank initializes to a slightly randomized starting point within the attractor basin
- Different every time the module is instantiated
- No reset function — attractors run free forever

### CPU Considerations
- Three simultaneous ODE integrations
- At audio rate, significant CPU load
- Consider reducing integration quality at extreme speeds if needed

---

## Panel Specifications (VCV Rack)

- Width: 20 HP (tentative)
- Screen: 128×128 px or similar, top left
- Knobs: Medium size, good resolution for rate control
- Switches: 4-position rotary or slider
- Jacks: Standard 3.5mm, grouped by bank
- Button: Small momentary, below screen

---

## Name

**Strange Weather** — after Lorenz's meteorological origins and the unpredictable-yet-deterministic nature of the outputs.

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 0.4 | 2025-01-XX | Added display cycle button, finalized layout |
| 0.3 | 2025-01-XX | Screen as pure visualization, no menu |
| 0.2 | 2025-01-XX | Removed CV inputs and LFOs, three free-running attractors |
| 0.1 | 2025-01-XX | Initial concept |

---

## License

TBD

---

## Acknowledgments

- Worng Electronics Vector Space — inspiration for output transform topology
- Synthesis Technology E352 Cloud Terrarium — interface reference
- Edward Lorenz — for discovering deterministic chaos
- Rössler, Thomas, Halvorsen — for their respective attractors
