# NTU miniScope 2.0

Welcome to the NTU miniScope!! Read instructions before using this software.
Author: Chung-En Huang, Yu-Shin Huang

[![Language](https://img.shields.io/badge/Language-C%2B%2B%2FCLI-blue.svg)](https://learn.microsoft.com/en-us/cpp/dotnet/dotnet-programming-with-cpp-cli-visual-cpp)
[![Framework](https://img.shields.io/badge/Framework-.NET%208.0%20%7C%204.8-green.svg)](https://dotnet.microsoft.com/download)
[![CUDA](https://img.shields.io/badge/CUDA-11.x%20%7C%2012.x-76B900.svg)](https://developer.nvidia.com/cuda-toolkit)

This repository contains the core system control software for the **NTU miniScope 2.0**, an advanced, lightweight, integrated miniaturized head-mounted two-photon functional imaging system designed specifically for deep-brain imaging in freely moving mice.

The software platform is built using a **C++/CLI framework**, seamlessly bridging high-performance hardware communication with a user-friendly Graphical User Interface (GUI). It handles real-time hardware coordination, multi-mode scanning signal generation, high-speed data acquisition, and GPU-accelerated image processing.

---

## 🚀 Key Features

The software architecture is divided into three main operational layers:

### 1. MEMS Scanning Signal Drive & Control
* **Multi-Mode Scanning**: Supports **Raster scan**, **UltraFast Raster scan** (high frame rate via reduced line count), and **Lissajous scan** to adapt to various spatial-temporal resolution requirements.
* **Bidirectional Waveform Generation**: Utilizes a bidirectional strategy with retrace inversion along a continuous sinusoidal X-axis and a staircase-like triangular Y-axis to eliminate idle return time and maximize scanning efficiency.
* **Dynamic FOV Control**: Provides real-time adjustment of scan amplitudes (determining the field of view), line frequency, and the number of lines per frame directly from the GUI.

### 2. High-Speed PMT Data Acquisition & Mapping
* **Real-Time Streaming**: Automatically streams digitized fluorescence readouts from the high-speed AlazarTech digitizer.
* **Double-Buffered Memory Management**: Efficiently organizes raw serial PMT voltage data streams into structured 2D images in real time without data loss.

### 3. GPU-Accelerated Real-Time Image Processing Pipeline
* **Low-Latency Processing**: Powered by **NVIDIA CUDA** to handle low signal-to-noise ratio (SNR) data streams without latency.
* **Built-in Filters & Enhancements**: Features Gaussian/Median filtering for noise reduction, CLAHE (Contrast-Limited Adaptive Histogram Equalization), contrast/brightness adjustment, gamma correction, and histogram normalization.

### 4. Synchronized Auxiliary Hardware Control
* **Trigger TTL Synchronization**: Generates precise hardware timing clocks and trigger signals via NI DAQ to lock and synchronize all peripheral hardware devices.
* **Adaptive Excitation Modulation**: Manages an external clock signal to drive an optical chopper, dynamically modulating the laser phase to restrict excitation windows, minimizing tissue phototoxicity while boosting image contrast.

---

## 🛠 System Configuration

The software operates as the central command unit coordinating the following interconnected hardware sub-systems:
<img width="755" height="361" alt="螢幕擷取畫面 2026-06-06 154104" src="https://github.com/user-attachments/assets/68765edd-5a3b-4ad5-beae-e1c17c0aa5ae" />


* **Excitation & Optical Path**: Coordinates the femtosecond laser source modulated by a synchronized optical chopper, delivered via a hollow-core fiber into the 3D-printed miniScope imaging module.
* **Scanning Control**: High-precision waveform commands are output from the National Instruments (NI) PCIe card to the MEMS driver to actuate the internal MEMS scanning mirror.
* **Fluorescence Detection Chain**: Emitted fluorescence is captured through an implanted GRIN lens, sent via a flexible fiber bundle through filtering sets, collected by a Photomultiplier Tube (PMT) in a thermoelectric cooling housing, and amplified via a PMT amplifier.
* **Digitization**: The amplified analog voltage readout is fed directly into a high-speed AlazarTech digitizer for raw image data construction.

---

## 📋 Requirements & Specifications

To maintain stable operation, high-throughput buffering, and low-latency image rendering, the following software and hardware configurations are required:

### Hardware Prerequisites
| Component | Model / Specification |
| :--- | :--- |
| **CPU** | AMD Ryzen Threadripper 3960X 24-Core Processor (or equivalent high-thread CPU) |
| **GPU** | NVIDIA GeForce RTX 3080 Ti (Required for CUDA real-time image processing) |
| **Data Acquisition (DAQ)** | NI PCIe-6341 Data Acquisition Card with SCB-68A Shielded Connector Block |
| **Digitizer** | AlazarTech ATS9146 High-Speed PCI Express Digitizer |

### Software & SDK Environments
| Dependency | Version Requirement |
| :--- | :--- |
| **Framework** | .NET v8.0 & .NET Framework v4.8 |
| **Digitizer Driver** | AlazarTech ATS9146 SDK / Driver v7.13.8 |
| **DAQ Driver** | NI-DAQmx |
| **Computer Vision** | OpenCV & OpenCV-contrib v4.9.0 |
| **GPU Computing** | NVIDIA CUDA & cuDNN v8.9.7 |
| **Motion Control** | Zaber.Motion NuGet Package v4.8.0 |

---

## 💡 Implementation Notes

Before data acquisition begins, the system executes an automated **Warm-up Procedure** on the GPU. By passing an empty buffer through the CUDA kernel during initialization, the framework pre-allocates memory and warms up the CUDA contexts. This mitigates runtime overhead and execution spikes, ensuring stable per-frame processing times during continuous high-speed acquisition.

---

## 📝 Citation

If you use this software or system design in your research, please cite the following thesis:

### APA Format
```text
Huang, Y.-S. (2025). Advancing Miniaturized Two-Photon Microscope for Mouse Deep Brain Imaging (Master's thesis). National Taiwan University, Taipei, Taiwan.

