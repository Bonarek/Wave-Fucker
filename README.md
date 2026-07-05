# WaveFucker 
**by BonarSound**

A rebellious, analog-modeled polyphonic synthesizer plugin built with the JUCE framework. 

WaveFucker combines a unique, hand-drawn "cartoon" aesthetic with serious, heavy-duty DSP under the hood. It serves not only as a gritty, character-rich instrument but also as a living showcase of synthesizer algorithm evolution—featuring everything from raw, naive waveforms to advanced mathematical anti-aliasing techniques.

---

## Key Features

### Oscillators & DSP Engines
WaveFucker features 3 core waveforms (Sawtooth, Triangle, Square) powered by **5 selectable synthesis methods**, allowing you to choose the exact sonic character and CPU footprint:
* **Naive:** Raw, digital, and fully aliased for lo-fi grit.
* **BLIT** (Band-Limited Impulse Train): Classic analog modeling with phase-synced harmonic generation.
* **DSF** (Discrete Summation Formulas): Complex harmonic equations for pure, rich tones.
* **PolyBLEP:** The industry standard-fast, highly optimized anti-aliasing.
* **DPW** (Differentiated Parabolic Wave): Smooth, modern, and CPU-efficient.

### Modulation, Filtering & Control
* **Resonant Low-Pass Filter:** 12dB/oct ladder filter with Cutoff and Resonance controls.
* **Dual Envelopes:** Independent ADSR for Amplitude and Filter Cutoff.
* **LFO:** Routable Low-Frequency Oscillator with Rate and Depth controls.
* **Glide (Portamento):** Features internal parameter smoothing to prevent mathematical clipping during rapid frequency changes.
* **White Noise Generator** for added texture.

### Visual Feedback & UI
* **Custom Hand-Drawn GUI:** A 100% bespoke, sprite-based interface powered by a custom `LookAndFeel_V4` class. No boring generic sliders.
* **Real-time Oscilloscope:** See the actual waveform shape dynamically reacting to your DSP method choice.
* **Spectrum Analyzer (FFT):** 512-band frequency visualization rendered directly onto the custom UI.

---

## Installation & Build Instructions

### Prerequisites
To build this plugin from the source code, you will need:
* [Git](https://git-scm.com/)
* [JUCE Framework](https://juce.com/) (specifically the **Projucer**)
* A C++ compiler (Visual Studio 2022 for Windows, Xcode for macOS)
## Project Status:
- [x] Naive synthesis method implementation
- [x] Oscilloscope
- [x] Spectrum Analyzer (FFT)
- [x] BLIT (Band-Limited Impulse Train) method implementation
- [x] DSF method implementation
- [x] PolyBLEEP method implementation
- [x] DPW method implementation

## Roadmap / To-Do:
- [x] Add waveform selection (Saw/Tri/Sqr)
- [ ] Optimize buffer memory usage
- [x] Finalize UI branding and aesthetics
- [x] Add LPF (Cutoff)
- [x] Add Amplifier
- [x] Add LFO (Low Frewuency Oscilator)
- [x] Add ADSR Amplitude
- [x] Add ADSR Cutoff
- [x] Add Glide 

### 1. Clone the Repository
Open your terminal or command prompt and run the following commands to clone the project:
```bash
git clone [https://github.com/Bonarek/WaveFucker.git](https://github.com/Bonarek/WaveFucker.git)
cd WaveFucker





