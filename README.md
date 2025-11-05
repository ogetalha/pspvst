# PSPVST — A JUCE-Based Audio Plugin

## Overview

Think modern audio plugins are too complicated? Want something fresh, yet feel like a blast from the past?

Me too! That's why I created **PSPVST**.

**PSPVST** is a custom-built audio plugin developed using the **JUCE Framework** and **CMake**.  
This project was created as a personal exploration into **Digital Signal Processing (DSP)**, **C++ frameworks**, and **cross-platform build systems**, all while combining a passion for **music production** and **software engineering**.

The plugin is **VST3-compatible** featuring a functional three-band equalizer with real-time FFT spectrum analysis. It allows users to visualize and shape their audio signal through low-cut, peak, and high-cut filters with configurable slopes.

**DISCLAIMER:** While the **visual design and typography** are inspired by the **PlayStation Portable (PSP)** aesthetic, this project has **no affiliation, endorsement, or association with Sony Interactive Entertainment**.

The style was chosen purely as a creative tribute, with the long-term goal of developing a **handheld audio workstation** in the future.

---

## Learning Objectives

This project was designed as a practical way to deepen understanding of:
- C++ class structures and modular programming
- DSP fundamentals (filters, frequency analysis, gain shaping)
- The JUCE framework for audio and GUI development
- CMake build configuration and project structuring
- Real-time audio visualization and UI optimization

---

## Features

### Signal Processing
- **Peak Filter** with adjustable frequency, gain, and Q-factor  
- **Low-Cut and High-Cut Filters** with selectable 12, 24, 36, and 48 dB/oct slopes  
- **Real-time filter response curve** visualized within the main interface  

### FFT Spectrum Analysis
- Live stereo **FFT analyzer** using JUCE’s `dsp::FFT`  
- Custom path generation for smooth, high-performance rendering  

### User Interface
- **PSP-inspired pastel gradient theme**
- **Custom splash screen animation** during startup  
- **Rotary sliders** for precise parameter control  

---

## Technical Details

- **Framework:** JUCE  
- **Build System:** CMake (v3.24+)  
- **Plugin Formats:** VST3, AU, Standalone  
- **Language Standard:** C++17  
- **FFT Implementation:** JUCE `dsp::FFT`  
- **Parameter Management:** `AudioProcessorValueTreeState`  


---

## Installation

You can either **use the included VST3 file** or **build PSPVST from source**.

### Option 1: Use the Included Plugin
The repository includes a compiled **VST3 binary** (`PSPVST.vst3`) inside the `build` folder.

1. Copy `PSPVST.vst3` to your system’s VST3 directory:
   - **Windows:** `C:\Program Files\Common Files\VST3`
   - **macOS:** `/Library/Audio/Plug-Ins/VST3`
2. Open your DAW and perform a plugin rescan.
3. PSPVST should now be available for use.

Or simply add the `build` folder as a custom VST3 path in your DAW settings.

**NOTE**: I've only tested the plugin with Reaper until now. Not sure if it works with other DAWs. Try Reaper if you run into issues with your DAW.

### Option 2: Build from Source

#### Requirements
- CMake 3.24 or newer  
- A C++17-compatible compiler (MSVC, Clang, or GCC)  
- JUCE Framework (automatically fetched via CPM in CMake)

#### Build Steps
```bash
git clone https://github.com/ogetalha/pspvst.git
cd path-to-cloned-repo
cmake -B build -G "your-preferred-generator"
```

## What's Missing?

I haven't yet implemented:
- Additional plugin formats (AU, AAX)
- Preset management system
- Bypass functionality
- Some advanced DSP features (e.g., dynamic EQ, multi-band compression)

I do plan on working on this project further in the future, mostly in the form of an actual handheld audio workstation. Some code also needs to refactored and cleaned up, such as the splash
screen at launch. I also intend to make the GUI look more appealing and add more visual elements. 

---

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---
