# ILTD Media Player

A simple media player built with:
- **libVLC** (video/audio playback)
- **SDL2** (window & context)
- **ImGui** (UI)
- **OpenGL** (texture rendering)

---

## ✅ Prerequisites

1. **VLC SDK**
   - Download from: https://code.videolan.org/videolan/vlc/-/releases
   - Extract and copy the following files into your project:
     - `libvlc.dll`
     - `libvlccore.dll`
     - `plugins/` folder
   - Place them in:  
     ```
     ./dlls/
     ./dlls/plugins/
     ```

2. **SDL2**
   - Download from: https://github.com/libsdl-org/SDL/releases
   - Place `SDL2.dll` into the `./dlls/` directory

3. **logo.bmp**
   - Place a 24-bit BMP image as `logo.bmp` in the project root to show before playback.

---

## ✅ How to Build

1. Open the project in **Visual Studio**
2. Make sure `CMakeLists.txt` includes:
   ```cmake
   target_sources(media PRIVATE appicon.rc)
