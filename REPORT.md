# CleanCity Simulator - Academic Project Report (draft)

## 1. Objective
Simulate smart-city waste management entirely in software to demonstrate Computer Graphics concepts.

## 2. Computer Graphics concepts used
- **2D transformations with 3x3 homogeneous matrices** (`Transformation.cpp`): translation, rotation, scaling, reflection (X, Y, origin, y=x), shearing.
  `Gfx` multiplies every vertex by a matrix stack, so truck placement/heading, camera zoom/pan, and the CG Lab all use `Mat3`.
  Camera: `M = T(viewport centre + pan) * S(zoom) * T(-world centre)`; picking uses `M^-1`.
- **Animation (time based, dt):** truck motion along way-points, wheel rotation, collection animation, traffic lights, clouds, water waves, day/night blend, animated dashboard counters.
- **Interaction:** mouse picking, zoom, pan, buttons, keyboard shortcuts.
- **Rendering:** GL_LINES, points, triangles, quads, polygons, triangle fans, quad strips, vertex colour interpolation (gradients), alpha blending, scissor clipping, 2.5D oblique building projection.

## 3. Algorithm - Dijkstra
Intersections = nodes, roads = weighted edges (length x congestion factor). `RoutePlanner::shortestPath` uses a priority queue; the drawn and driven route is exactly this result. Traffic ON multiplies weights of congested roads by 2.5, changing route cost.

## 4. Simulation model
Bins fill at individual rates; levels Low/Medium/High/Full (0-30/31-70/71-99/100 %). Trucks: Idle -> To bin -> Collecting -> Returning -> Unloading. Collected waste feeds statistics; delivered waste feeds the recycling centre which processes the recyclable share over time. Zone cleanliness = 100 x (1 - waste/capacity).

## 5. Demonstration plan (video, ~3 min)
Splash -> Start -> select a bin -> Find Route -> Dispatch -> zoom/pan -> Traffic ON -> Night -> Dashboard -> Recycling -> Statistics report -> CG Lab (each of the 5 transformations, show the matrix).

## 6. Sample data
See `data/sample_simulation_data.txt`.

## 7. TODO for final submission
Add screenshots of all screens and the transformation demonstrations, and the demo video.
