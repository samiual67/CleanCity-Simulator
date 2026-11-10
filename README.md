# CleanCity Simulator - Smart City Waste Management Simulation (C++ / OpenGL / FreeGLUT)

100% software simulation: no hardware, sensors, GPS or real data. (~70% complete build: all core features work.)

## Build & run
**Windows - Code::Blocks (MinGW):** install FreeGLUT (copy `include/GL/*` to MinGW `include/GL`, `lib/libfreeglut.a` to MinGW `lib`, `freeglut.dll` next to the exe).
Create an empty project, add all `src/*.cpp`, set *Search directories -> Compiler* to `include`, add compiler flag `-std=c++11`,
and link libraries: `freeglut opengl32 glu32`. Or just run `build_windows.bat`.
**Windows - Visual Studio:** new empty C++ project, add `src/*.cpp`, add `include` to Additional Include Directories, link `freeglut.lib opengl32.lib glu32.lib`.
**Linux:** `sudo apt install freeglut3-dev` then `make && ./CleanCitySimulator`.  **macOS:** use `-framework GLUT -framework OpenGL`.

## How to use
1. *Start Simulation* -> City Map opens. Click a dustbin (details popup + info card).
2. *Find Shortest Route* draws the Dijkstra route; *Dispatch Garbage Truck* sends a truck (stops, collects, returns, unloads to recycling).
3. *Toggle Traffic*: congested roads (red) cost 2.5x, routes adapt. *Toggle Day/Night*, *Auto Dispatch*, *Add Waste*, *Empty Selected Bin*, *Reset*.
4. Tabs: Dashboard, City Map, Recycling, Statistics (+ Generate Report -> `report_summary.txt`).

Keys: Space pause/resume, 1-5 screens, N night, T traffic, A add waste, E empty, D dispatch, R route, S speed x1/2/4, U auto, +/- zoom, arrows pan, 0 reset view, Esc back/exit.
Mouse: wheel zoom, drag pan, click select.

## Structure
`src/` + `include/`: Application (screens/input), Simulation (state + commands), CityLayout/CityMap (city & render), Dustbin, WasteManager, GarbageTruck,
RoutePlanner (Dijkstra), RecyclingCenter, Dashboard (+recycling screen), Statistics, TransformLab, Transformation (Mat3), Gfx (matrix-stack drawing),
Objects (truck/tree/building/bin), Animation, UIComponents. Logic (Simulation, WasteManager, GarbageTruck, RoutePlanner) is separate from rendering.

## Not finished yet (the remaining ~30%)
Custom font/asset loading, sound, more polish on transitions, truck traffic-light stopping, screenshots/video, final report figures.
