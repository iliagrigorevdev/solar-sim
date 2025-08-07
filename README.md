# Solar System Simulation

[![Run Simulation](https://img.shields.io/badge/Run-Simulation-brightgreen)](https://iliagrigorevdev.github.io/solar-sim/)

This project is a real-time, interactive 2D simulation of a planetary system, built with C++ and rendered in the browser using WebGL. The simulation is compiled to WebAssembly, allowing for near-native performance in a web environment.

This project was created with the help of gemini-cli and the Gemini 2.5 Pro model, running on an Android phone in the Termux command-line environment.

## Features

- **Real-time N-body simulation:** The core of the project is a physics-based simulation that calculates the gravitational interactions between all celestial bodies.
- **WebGL Rendering:** The simulation is visualized using WebGL for efficient, hardware-accelerated graphics.
- **Interactive Controls:**
  - **Zoom:** Use a two-finger pinch gesture on touch devices to zoom in and out.
- **Customizable Parameters:** A settings panel allows you to adjust various simulation parameters in real-time:
  - Gravitational Constant (G)
  - Density of celestial bodies
  - Number of bodies
  - Initialization radius
  - Time step (DT)
  - Softening factor
  - Maximum and minimum mass of generated bodies
  - Mass of the central body
  - Theta for the Barnes-Hut approximation
  - Simulation speed
  - Color gradient for the bodies based on their mass

## How it Works

The simulation logic is written in C++ and handles the physics calculations for the movement of celestial bodies. This includes:

- **Gravitational Force:** Calculating the force of gravity between all pairs of bodies.
- **Collision Detection:** Detecting when celestial bodies collide.
- **Collision Resolution:** Merging bodies that collide, conserving momentum and mass.

The C++ code is compiled to WebAssembly using Emscripten, which allows it to run in the browser. The rendering is done using WebGL, with GLSL shaders for the visual effects. The simulation is optimized using a quadtree data structure to reduce the complexity of collision detection from O(n^2) to O(n log n).

## Building and Running the Project

### Prerequisites

- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html): Required to compile the C++ code to WebAssembly. For Termux, use `pkg install emscripten`.
- [Node.js](https://nodejs.org/): Used for managing dependencies and running scripts.
- [Python 3](https://www.python.org/downloads/): Used to run a simple HTTP server for local development.

### Build Steps

1.  **Clone the repository:**

    ```bash
    git clone https://github.com/iliagrigorevdev/solar-sim.git
    cd solar-sim
    ```

2.  **Install dependencies:**

    ```bash
    npm install
    ```

3.  **Activate the Emscripten environment:**

    ```bash
    source /path/to/emsdk/emsdk_env.sh
    ```

4.  **Run the build script:**
    The `build.sh` script compiles the C++ code (`simulation.cpp`, `renderer.cpp`, `quadtree.cpp`) into a WebAssembly module (`simulation.wasm`) and the necessary JavaScript bindings (`simulation.js`). It also preloads the GLSL shader files.

    ```bash
    ./build.sh
    ```

5.  **Start the local server:**

    ```bash
    ./serve.sh
    ```

6.  **View the simulation:**
    Open your web browser and navigate to `http://localhost:8000`.

## Code Formatting

The project uses `prettier` for formatting HTML, CSS, and JavaScript files, and `clang-format` for C++ files. To format the code, run:

```bash
npm run format
```

This will format all the necessary files in the project.

## File Structure

- `build.sh`: The build script for compiling the project.
- `renderer.cpp` / `renderer.h`: Handles the WebGL rendering of the simulation.
- `simulation.cpp` / `simulation.h`: Contains the core logic for the N-body simulation.
- `quadtree.cpp` / `quadtree.h`: Implements the quadtree data structure for optimizing collision detection.
- `shader.frag` / `shader.vert`: GLSL shaders for rendering the celestial bodies.
- `public/`: Contains the web-related files.
  - `index.html`: The main HTML file for the web interface.
  - `style.css`: CSS for styling the web page.
  - `ui.js`: Handles the user interface and interaction.
  - `simulation.js` / `simulation.wasm`: The compiled WebAssembly module and JavaScript bindings.

## Acknowledgments

- Thanks to the [Emscripten](https://emscripten.org/) project for making it possible to run C++ code in the browser.
- Thanks to the [Termux](https://termux.com/) project for providing a powerful terminal emulator for Android.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
