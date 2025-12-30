# 🎄 Christmas Tree Overlay

A festive, interactive Christmas tree overlay for your desktop. Built with C++ and Qt6, this cross-platform application brings holiday cheer to your Windows, macOS, or Linux desktop with procedural trees, falling gifts, and dynamic snow.

![Version](https://img.shields.io/badge/version-1.0.0-gold)
![License](https://img.shields.io/badge/license-MIT-green)
![Platforms](https://img.shields.io/badge/platforms-Windows%20|%20macOS%20|%20Linux-blue)

## ✨ Features

- **🌲 Procedural Tree Generation**: Every time you select "Prosedürel" (Procedural), a unique, symmetrical tree is generated just for you.
- **🎁 Falling Gift Boxes**: Choose from 9 combinations of colors (Red, Blue, Gold) and sizes (Small, Medium, Large). Watch them fall gracefully from your cursor to the floor.
- **🎨 Interactive Decoration**: Drag and drop ornaments, stars, and even cardboard text messages onto your tree.
- **❄️ Dynamic Snow Control**: Control the weather with right-click menu options to "Karı Artır" (Increase Snow) or "Karı Azalt" (Decrease Snow).
- **🚀 Ultra-Lightweight**: Frameless, transparent, and designed to sit subtly on your desktop.

## 📥 Installation

You can download the latest pre-compiled binaries for your operating system from the [Releases](https://github.com/maidis/christmas-overlay/releases) page.

- **Windows**: Download the `.zip`, extract, and run `ChristmasOverlay.exe`.
- **macOS**: Download the `.dmg`, open it, and move the app to your Applications folder.
- **Linux**: Download the `.AppImage`, make it executable (`chmod +x ...`), and run it.

## 🛠️ Build from Source

### Prerequisites
- Qt 6.5 or newer
- CMake 3.16 or newer
- A C++17 compatible compiler

### Steps
```bash
# Clone the repository
git clone https://github.com/maidis/christmas-overlay.git
cd christmas-overlay

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## 🖱️ Controls
- **Left Click & Drag**: Move the tree or placed ornaments.
- **Right Click (on tree/items)**: Access the context menu to change tree types, add gifts/ornaments, control snow, or remove items.
- **Right Click (anywhere)**: Exit or control snow intensity.

## 📄 License
This project is licensed under the MIT License - see the LICENSE file for details.

---
*Developed with ❤️ for the holiday season.*
