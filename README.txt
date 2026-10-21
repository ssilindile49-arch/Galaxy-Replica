ORBIT TEXTURE — Windows VST3 project
====================================

This is a buildable JUCE/CMake project for a small original MIDI texture
instrument with a custom Windows-friendly UI and five automatable macros:
ORBIT, SPREAD, COLOR, NOVA, TAIL.

IMPORTANT
---------
This archive contains source code, not a precompiled .vst3 binary. A VST3
binary must be built for Windows with the Windows toolchain. This environment
cannot verify or compile a Windows VST3 binary, so do not treat the source as
a crash-tested release.

Requirements
------------
1. Windows 10/11 64-bit
2. Visual Studio 2022 with "Desktop development with C++"
3. CMake 3.22+
4. JUCE 7 or JUCE 8 source checkout

Build
-----
1. Extract this folder.
2. Install JUCE and note its folder, e.g. C:\JUCE.
3. Open Command Prompt in this folder:
     set JUCE_DIR=C:\JUCE
     build-windows.bat
4. Result should be in:
     build\OrbitTexture_artefacts\Release\VST3\Orbit Texture.vst3
5. Copy that .vst3 bundle into:
     C:\Program Files\Common Files\VST3
6. Rescan plug-ins in your DAW.

If you use a different Visual Studio generator, configure CMake manually:
  cmake -S . -B build -DJUCE_DIR="C:/JUCE" -G "Visual Studio 17 2022" -A x64
  cmake --build build --config Release --target OrbitTexture_VST3

What works in this initial build
--------------------------------
- VST3 and standalone targets
- MIDI note input and polyphonic voice pool
- Five automatable macro parameters
- Host BPM fallback and tempo-responsive rhythmic oscillator
- Color filter, stereo spread, reverb, parameter smoothing
- Plugin parameter state save/restore
- Custom macro GUI

Known limitations
-----------------
- This is an original Galaxy-inspired starter instrument, not a 1:1 clone.
- This build synthesizes a basic tonal/rhythmic texture; it does not yet load
  one-shot or loop WAV files.
- Loop engine is tempo-responsive synthesis, not audio loop warping.
- ORBIT modulates loop phase and stereo movement, not sample start points.
- Test in a DAW before relying on it in a session. A production release still
  needs real-time allocation tests, host compatibility tests, and DSP QA.
