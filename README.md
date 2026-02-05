# GrooveSeq

GrooveSeq is a JUCE-powered VST3 drum sequencer built for fast beat sketching inside your DAW. Sixteen sample pads, a 32-step grid, and live-friendly controls let you dial in evolving percussion patterns with just a few clicks. Load your own samples by dropping them onto pads, audition sounds instantly with the built-in preview button, and shape the groove with swing, humanize, density, fills, velocity randomization, and per-pad ADSR envelopes.

> **Status:** Early-stage but fully playable. Designed for macOS with JUCE 7+ (VST3). Windows/Linux builds are on the roadmap once the JUCE toolchains are documented.

## Feature Highlights
- **16 Pad Layout** – Drag-and-drop WAV/AIFF/FLAC files onto any pad or click the magnifier button to open the file browser. Each pad stores its ADSR envelope and label.
- **Instant Preview** – Tap the play icon on any pad to audition the sample immediately, even if the host transport is stopped.
- **Deterministic Sequencer** – Generate 32-step patterns (two bars of 16ths) tailored to whichever pads have samples loaded. Density and fills parameters bias probabilities so the groove matches your material.
- **Human Performance Controls** – Swing, humanize (timing drift), and velocity randomization keep loops from feeling robotic.
- **Grid Editing** – Toggle any cell in the Sequencer Grid to fine-tune the pattern, watch the real-time playhead follow along.
- **Dark UI Theme** – Midnight background, muted greys, electric accents; adapts gracefully when resizing.

## Screenshots & Media
- _Coming soon_: Add animated GIFs or stills of GrooveSeq loaded in your DAW.

## Requirements
- **Operating System:** macOS 11+ (tested). Other platforms untested for now.
- **Toolchain:** CMake ≥3.15, Clang/AppleClang with C++17, JUCE checkout (default `JUCE/` sibling inside repo or set `JUCE_DIR`).
- **Plugin Host:** Any VST3-compatible DAW (Ableton, Reaper, Logic via AU wrapper, etc.).

## Repository Layout
- `Source/PluginProcessor.*` – audio engine, sequencing, sample playback, and parameter/state management.
- `Source/PluginEditor.*` – UI layout, pad wiring, slider attachments, file browser.
- `Source/SamplePad.*` – reusable pad component with drag/drop, browse/play buttons, selection visuals.
- `Source/Sequencer.*` – 16×32 boolean grid plus probability-based pattern generator.
- `Source/SequencerGrid.*` – paint + interaction logic for the step grid.
- `scripts/build_vst3.sh` – configure/build/install helper.
- `build/` – generated artifacts (never edit by hand).
- `AGENTS.md` – development guardrails for contributors and AI agents.

## Quick Start
```bash
git clone https://github.com/<your-account>/drum-machine.git
cd drum-machine

# Configure (Release by default) – update JUCE_DIR as needed
cmake -S . -B build -DJUCE_DIR=$HOME/dev/JUCE -DCMAKE_BUILD_TYPE=Release

# Build plugin binaries
cmake --build build --config Release

# Optional helper (configures, builds, installs to ~/Library/Audio/Plug-Ins/VST3)
BUILD_TYPE=Release JUCE_DIR=$HOME/dev/JUCE ./scripts/build_vst3.sh
```

Artifacts land in `build/GrooveSeq_artefacts/<Config>/VST3/GrooveSeq.vst3`. The helper script copies the bundle into `~/Library/Audio/Plug-Ins/VST3/GrooveSeq.vst3` for you. If you prefer manual installs, copy the `.vst3` folder into your DAW’s plugin path.

## Loading the Plugin
1. Build and install as above.
2. Launch your DAW, rescan VST3 plugins if required.
3. Insert “GrooveSeq” on an instrument track (it generates audio internally via JUCE’s `Synthesiser`).
4. Press play in the DAW to drive the sequencer, or stay stopped and use pad preview buttons to audition sounds.

## Working with Pads & Samples
- **Load from Button:** Click the magnifier icon on any pad to open a file chooser (filters `.wav`, `.wave`, `.aiff`, `.aif`, `.flac`).
- **Drag and Drop:** Drop files directly onto pads to assign them instantly.
- **Preview:** Hit the play icon to fire the loaded sample immediately. Works even when the transport is idle.
- **Selection:** Clicking a pad highlights it and syncs ADSR sliders + labels in the header.
- **Naming:** Pad labels automatically adopt the file name (sans extension). Empty pads show “Pad N”.

## Sequencer & Controls
- **Generate** – Produces a new 32-step pattern. Pads without samples stay empty; active pads get probability-weighted rhythms plus fills near the end of bar two.
- **Swing** – Percent swing applied to odd 16ths.
- **Humanize** – Milliseconds of random timing offset per hit.
- **Fills** – Controls how busy the last four steps of the loop become.
- **Density** – Governs how many hits each pad receives overall.
- **Velocity Rand** – Adds ± randomization around base velocity.
- **ADSR (Attack/Decay/Sustain/Release)** – Per-pad envelope controls; updates apply immediately to the selected pad’s `juce::SamplerSound`.

## Development Workflow
- Read `AGENTS.md` before coding. It documents style, threading rules (never block the audio thread), locking strategy, and manual QA expectations.
- Keep `Sequencer` logic deterministic; randomness is centralized in `generate` with passed-in seeds.
- UI components rely on `juce::AudioProcessorValueTreeState::SliderAttachment` – never let attachments go out of scope.
- Use `std::array` for fixed-size pad/step data, `std::unique_ptr` for UI children.
- Follow include ordering (self header → JUCE → STL → project) and 4-space indentation with braces on the same line for functions.
- `build/` contains generated files; ignore it in commits.

## Testing & QA
- Automated unit tests are not yet in place. To add them later, integrate Catch2 or JUCE’s test harness via CTest and document commands in `AGENTS.md`.
- Manual verification checklist:
  - Build succeeds in Debug/Release.
  - Load GrooveSeq in a DAW, ensure pads trigger audio and sequencer advances in sync.
  - Drag-and-drop plus browse-based sample loading both work.
  - Swing/density/fills/humanize knobs respond and update playback.
  - Session save/load restores pad assignments and sequencer state (via ValueTree serialization).

## Troubleshooting
- **JUCE Not Found:** Set `JUCE_DIR=/path/to/JUCE` during configure or create a `JUCE/` submodule next to the repo.
- **No Audio During Preview:** Ensure your DAW keeps calling the plugin when transport is stopped (most do). The preview MIDI is injected into `processBlock` even when not playing.
- **Mismatched Architectures:** Delete `build/` (with approval) and reconfigure if Apple Silicon vs Intel issues arise.
- **Code Signing Warnings:** macOS may warn that the VST3 bundle is ad-hoc signed; sign it with your developer ID for distribution.

## Roadmap Ideas
- Windows/Linux build instructions.
- Parameter automation smoothing to reduce zipper noise.
- Per-pad swing/humanize overrides.
- Step probability editing and accent lanes.
- Sample browser favorites / tag filtering.

## Contributing
1. Fork + branch (`feature/<name>`).
2. Follow coding standards from `AGENTS.md`.
3. Keep PRs focused; describe manual QA steps (DAW used, parameters tested).
4. Run `cmake --build build --config Release` before opening a PR.
5. Document new scripts or behaviors in both `README.md` and `AGENTS.md`.

## License
- _Choose a license (MIT/BSD/GPL/etc.) and drop it here. If undecided, state “License pending”._

## Credits
- Built with [JUCE](https://juce.com).
- Created by @rodrwan and contributors. Contributions welcome!
