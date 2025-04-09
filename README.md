# EMG-driven FES System using Unity and C

This repository contains an experimental framework for **functional electrical stimulation (FES)** driven by **electromyography (EMG)** in real-time.  
It consists of two main components:

- A **Unity project** that simulates and processes EMG data
- An external **C program** that communicates with Unity via a named pipe and sends impulses using the [ScienceMode4 Library](https://github.com/muellerlab/ScienceMode4)

## 🧩 Project Structure

```
UnityProject/      → EMG simulation & processing, Named Pipe Client
CProgram/          → Connects with Unity, sends stimulation impulses (uses ScienceMode4)
```

## 🚀 How It Works

### Unity Component (`UnityProject/`)

1. **`EMG_Simulator.cs`**  
   Generates a list of 30 simulated EMG values every 50 ms.

2. **`EMG_Processor.cs`**  
   Uses calibration data (first 250 ms), calculates mA and µs values from the EMG signal.

3. **`NamedPipeClient.cs`**  
   Establishes a named pipe connection with the C program and sends stimulation parameters.

4. **`CSVReader.cs`**  
   Optionally reads real EMG log data from a CSV file (`Assets/EMG-data/emg_data.csv`) and emits values via event every millisecond.

### C Component (`CProgram/`)

- Builds a named pipe connection to Unity.
- Uses structs and functions from the **ScienceMode4 Library**:
  - `Smpt_ll_init`, `Smpt_ll_point`, `Smpt_ll_channel_config`, etc.
  - Functions such as `smpt_open_serial_port()` and `smpt_send_ll_channel_config()`

> The library must be available locally or added as a submodule (see below).

## 📥 Using the ScienceMode4 Library

The C component depends on the [ScienceMode4](https://github.com/muellerlab/ScienceMode4) library.

### ✅ Option 1: Manual Download
Download the library and place it in a folder like `CProgram/ScienceMode4/`.  
Make sure your compiler can find the headers and .lib/.dll files.

### 🔁 Option 2: Add as a Git Submodule

```bash
git submodule add https://github.com/muellerlab/ScienceMode4.git CProgram/ScienceMode4
git submodule update --init --recursive
```

## 🧪 Target Audience & Use Case

This project is aimed at developers and researchers working with **biofeedback**, **EMG**, and **FES** in real-time applications – especially in **VR/Unity** environments.

## 📌 To Do

- [ ] Add error handling (Unity ↔ C communication)
- [ ] Automatically configure COM port
- [ ] Add support for real EMG devices

## 📜 License

*(Add your license here, e.g., MIT, GPL, or CC BY-SA depending on your preferences.)*