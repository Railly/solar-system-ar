# 🌌 AR Solar System

An interactive **Augmented Reality Solar System** that displays a miniature solar system (Sun, Earth, Moon) floating above an ArUco marker. Built with **OpenGL**, **OpenCV**, and **ImGui** for real-time AR visualization and interactive controls.

![AR Solar System Demo](assets/demo.gif)
*AR Solar System with real-time orbital mechanics and lighting*

## ✨ Features

### 🎯 **Augmented Reality**
- **ArUco marker detection** using OpenCV (DICT_6X6_250, ID: 0)
- **Real-time camera feed** background rendering
- **Accurate pose estimation** for 3D object placement
- **Smooth fade in/out** effects based on marker visibility

### 🪐 **Realistic Solar System**
- **Hierarchical orbital mechanics**: Moon orbits Earth, Earth orbits Sun
- **Proper scaling**: Visible but proportional object sizes
- **Rotation and revolution**: Each object spins and orbits independently
- **Orbital plane tilts**: 20° inclination for better visibility

### 💡 **Advanced Lighting**
- **Point light source** positioned at Sun's center
- **Physically-based rendering** with diffuse, specular, and ambient lighting
- **Hemisphere lighting** for realistic fill light
- **Emissive Sun rendering** (unlit shader, depth-masked)

### 🎮 **Interactive Controls**
- **Real-time parameter tuning** via ImGui interface
- **Orbital speed control**: Adjust rotation and revolution speeds
- **System scaling**: Resize entire solar system
- **Height adjustment**: Control hover distance above marker
- **Lighting control**: Adjust intensity and warmth
- **Orbital axis selection**: Change orbit planes (X/Y/Z)

## 🛠️ Technical Highlights

### **Orbital Mechanics**
- **Angle accumulators** prevent rotation drift errors
- **dt-based updates** for frame-rate independent motion
- **Clean matrix reconstruction** each frame
- **Hierarchical transformations** for parent-child relationships

### **Rendering Pipeline**
- **Multi-shader system**: Separate lit/unlit shaders
- **Depth-correct rendering**: Earth/Moon first, Sun last
- **Alpha blending** for smooth transitions
- **Background quad** with proper UV mapping

### **AR Integration**
- **Camera calibration** and pose estimation
- **Coordinate system conversion** (OpenCV ↔ OpenGL)
- **Real-time marker tracking** at 30+ FPS
- **Robust frame validation** and error handling

## 📋 Requirements

- **macOS** (tested on macOS 14.3+)
- **OpenGL 4.1+**
- **Camera** for ArUco marker detection
- **ArUco marker** (ID: 0, DICT_6X6_250)

### Dependencies
- **OpenCV 4** (ArUco detection)
- **GLFW 3** (window management)
- **GLM** (mathematics)
- **ImGui** (user interface)
- **glad** (OpenGL loading)
- **stb_image** (texture loading)

## 🚀 Quick Start

### 1. Clone and Build
```bash
git clone https://github.com/YOUR_USERNAME/locked-in-cg.git
cd locked-in-cg
make
```

### 2. Print ArUco Marker
Download and print: [ArUco Marker ID 0](https://chev.me/arucogen/) (DICT_6X6_250)

### 3. Run
```bash
./solar
```

### 4. Point Camera
Point your camera at the ArUco marker and watch the solar system appear!

## 🎮 Controls

### **Keyboard**
- `Tab` - Toggle UI panel
- `Escape` - Exit application

### **UI Panel Controls**
| Section | Control | Range | Description |
|---------|---------|--------|-------------|
| **System** | Height above tablet | 0.02f - 0.20f | Distance from marker surface |
| | System Scale | 0.1× - 1.0× | Overall system size |
| **Lighting** | Sun intensity | 0.2× - 2.0× | Light brightness |
| | Light warmth | 0.5 - 1.0 | Yellow ↔ White color |
| **Sun** | Spin speed | 0° - 60°/s | Rotation speed |
| **Earth** | Spin speed | 0° - 180°/s | Day/night cycle |
| | Orbit speed | 0° - 60°/s | Revolution around Sun |
| | Orbit radius | 0.05 - 4.0 | Distance from Sun |
| | Orbit axis | X/Y/Z | Orbital plane |
| **Moon** | Spin speed | 0° - 120°/s | Rotation speed |
| | Orbit speed | 0° - 150°/s | Revolution around Earth |
| | Orbit radius | 0.05 - 0.5 | Distance from Earth |
| | Orbit axis | X/Y/Z | Orbital plane |

## 📸 Screenshots

![System Overview](assets/system-overview.jpg)
*Complete solar system with lighting*

![UI Controls](assets/ui-controls.jpg)
*Interactive ImGui control panel*

![Marker Detection](assets/marker-detection.jpg)
*ArUco marker tracking in action*

## 🏗️ Project Structure

```
locked-in-cg/
├── src/                    # Source code
│   ├── main.cpp           # Main application loop
│   ├── ar_tracker.*       # ArUco detection & pose estimation
│   ├── object.*           # 3D object with orbital mechanics
│   ├── scene.*            # Scene graph management
│   ├── shader.*           # OpenGL shader management
│   ├── mesh.*             # 3D mesh loading/rendering
│   ├── texture.*          # Texture loading
│   ├── ui_panel.hpp       # ImGui control interface
│   ├── imgui_layer.*      # ImGui integration
│   └── logger.hpp         # Logging system
├── assets/                # Textures and resources
│   ├── sun.jpg           # Sun texture
│   ├── earth.jpg         # Earth texture
│   └── moon.jpg          # Moon texture
├── external/             # Third-party libraries
│   ├── imgui/           # Dear ImGui
│   ├── glad/            # OpenGL loader
│   └── stb/             # STB image library
└── cook/                # Build utilities
```

## 🏷️ Development Tags

The project is organized with progressive development tags:

| Tag | Description |
|-----|-------------|
| `v0-setup` | Initial project setup |
| `v1-opengl-window` | Basic OpenGL window |
| `v2-sun-texture` | Textured Sun sphere |
| `v3-orbits` | Earth + Moon orbital mechanics |
| `v4-ar-marker` | ArUco marker detection |
| `v5-solar-system` | Camera background integration |
| `v6-imgui` | Interactive UI controls |
| `v7-lighting` | **Full lighting system** ⭐ |

## 🔧 Build System

### Compiler Flags
- **C++17** standard
- **Optimized** release builds (-O3)
- **macOS compatibility** flags
- **Dependency linking**: OpenCV, GLFW, OpenGL

## 🐛 Troubleshooting

### Camera Issues
- **Permission denied**: Allow camera access in System Preferences
- **No camera found**: Check camera connection and availability
- **Poor detection**: Ensure marker is well-lit and unobstructed

### Performance Issues
- **Low FPS**: Reduce system scale or close other applications
- **Lag**: Ensure adequate lighting for marker detection
- **Crashes**: Check OpenGL 4.1+ support

### Build Issues
- **OpenCV not found**: `brew install opencv`
- **GLFW missing**: `brew install glfw`
- **Compiler errors**: Ensure Xcode command line tools installed

## 🎯 Default Settings

| Object | Scale | Spin Speed | Orbit Speed | Orbit Radius |
|--------|-------|------------|-------------|--------------|
| **Sun** | 0.18f (18mm) | 15°/s | - | - |
| **Earth** | 0.08f (8mm) | 90°/s | 24°/s | 0.4f |
| **Moon** | 0.02f (2mm) | 60°/s | 75°/s | 0.12f |

- **System Scale**: 0.3× (compact for demos)
- **Hover Height**: 0.06f (6mm above marker)
- **Light Intensity**: 0.8× (realistic)
- **Light Warmth**: 0.95 (warm sunlight)

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **OpenCV** for robust ArUco marker detection
- **Dear ImGui** for immediate mode GUI
- **GLFW** for cross-platform window management
- **GLM** for graphics mathematics
- **NASA** for planetary texture inspiration

---

**Built with ❤️ for FISI Conference**

[![Demo Video](https://img.shields.io/badge/▶-Demo%20Video-red?style=for-the-badge)](https://youtu.be)
[![Download Latest](https://img.shields.io/badge/⬇-Download%20Latest-green?style=for-the-badge)](https://github.com/Railly/solar-system-ar/releases/latest) 