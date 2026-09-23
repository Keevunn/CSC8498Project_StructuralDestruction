# Design and Evaluation of Graph-Based Structural Approximations for Real-Time Game Destruction

An Unreal Engine 5 C++ project exploring graph-based structural approximations for real-time destruction in games. Developed as my master's dissertation project.

## Demo
[Click here to watch the project demonstration]()

## Abstract
Many video games use destructible environments to create dynamic and more immersive gameplay. These destruction systems can be complex, as they are able to produce damaged geometry and determine the subsequent behaviour of the resulting structure. This paper investigates whether simplified graph-based structural models can produce game-plausible behaviour in a real-time simulation, relative to a physics-driven baseline. 

Three approaches are implemented and evaluated: 
- **PHYS**: physics-driven constraint-based baseline
- **CONN**: graph connectivity model
- **LOAD**: directed load propagation model

Benchmark scenarios compare their setup costs, runtime costs, and behavioural agreement across different structure layouts and sizes. The results show that graph-based models provide more predictable structural response, while load-based reasoning enables more expressive destructive behaviour at the cost of additional tuning and runtime complexity.

## Technical Highlights
- Unreal Engine 5 C++
- Chaos physics
- Runtime graph construction
- Connectivity/support analysis
- Breadth-first graph traversal
- Directed load propagation
- Structural piece roles and capacities
- Explosion damage and impulse handling
- Automated benchmark scenarios
- CSV performance/result collection

## Structural Piece Types
- Anchor
- Support
- Load
- Objective
- Protected

## Running the Project

### Packaged Build

Run `\Windows\ControlledDemolition.exe`

### Unreal Project
The source Unreal project is included.

Open `ControlledDemolition.uproject` with the appropriate Unreal Engine version and rebuild the C++ modules if required.

## Controls

### Player
- `WASD`: Move
- `Space`: Jump
- `R`: Restart level

### Debug
- `Right Alt`: Toggle debug drawing
- `Right Ctrl`: Toggle verbose log

## Benchmarking

Benchmark sweeps output their results to:

`Windows/ControlledDemolition/Saved/BenchmarkResults/DemolitionBenchmark.csv`

## Piece Colour Guide

| Piece | Colour |
| --- | --- |
| Anchor | Magenta |
| Support | Yellow |
| Load | White |
| Objective | Green |
| Protected | Cyan |
