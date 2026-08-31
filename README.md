# SSHCB (SSH Clipboard)

A secure, network-based clipboard utility powered by `libssh`. It enables seamless copy-paste operations between machines without relying on desktop environment (DE) clipboard managers.

## ✨ Features

- **DE-agnostic:** Operates completely independently of system/DE clipboard managers.
- **Multi-channel support:** 10 independent, isolated clipboard contexts.
- **Secure by default:** Authenticates using standard SSH keys (no extra passwords required).
- **Multithreaded architecture:** Strict separation of user I/O and network event processing.
- **Deterministic state management:** Thread-safe Finite State Machines (FSM) for session and context control.
- **High observability:** Detailed logging of state transitions for easy debugging.
- **Editor integration:** Test integration with Vim.

## 🛠️ Tech Stack

- **Core:** C, `libssh`, Linux
- **Build & Tooling:** CMake, Nix, ASan, GDB

## 🎬 Demos

### Demo 1: Basic Cross-Machine Copy/Paste
<video src="https://raw.githubusercontent.com/ObscureCodeV/sshcb/main/assets/untitled.mp4" controls autoplay loop muted playsinline width="100%"></video>
*(Copying text on the local machine and pasting it on the remote server via SSH)*

### Demo 2: Vim Integration & Multi-Channel Usage
<video src="https://raw.githubusercontent.com/ObscureCodeV/sshcb/main/assets/untitled2.mp4" controls autoplay loop muted playsinline width="100%"></video>
*(Using different clipboard contexts directly from the Vim editor)*

## 🚀 Quick Start

## 🚀 Quick Start

### Building from source
```Bash
git clone https://github.com/ObscureCodeV/sshcb.git
cd sshcb

git checkout v0.1.0

nix develop

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

### Pre-flight Checks
*Dependencies:* Review flake.nix to see the exact, locked dependency tree.
*Testing:* Ensure you have generated SSH keys locally if you plan to run the integration tests.

### Verification & Observability
One of the core features of SSHCB is detailed state logging. After building (or running),you can inspect the state transitions:
```Bash
vim ~/.sshcb.log
