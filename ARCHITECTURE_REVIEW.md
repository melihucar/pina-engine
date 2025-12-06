# Pina Engine – Architecture Review Notes

## Subsystem Ownership & Lifecycle
- **Problem**: `Application::createSubsystems()` is a hand‑crafted list that mixes construction, dependency wiring, and platform specifics. Inserting a new subsystem or reusing a subset (e.g. headless tools, dedicated server) requires editing this method and remembering the correct order.
- **Risk**: Hidden dependencies (UI → Graphics, Input → Window, EventDispatcher → everyone) aren’t formalized, so future refactors can break initialization or shutdown ordering without compiler errors.
- **Suggestion**: Move to a registration‑based system where each subsystem declares its dependencies and lifecycle hooks. Let `Context` build/teardown the dependency graph and support phase‑based execution (Init → Update → Render → Shutdown). Make platform bindings (CocoaWindow/Input) plug in via factories rather than code paths inside `Application`.

## Rendering Stack
- **Problem**: `SceneRenderer` walks the scene graph and immediately issues OpenGL calls via `GraphicsDevice`. There is no abstraction for render queues, state sorting, multi-pass rendering, or culling.
- **Risk**: Adding features like deferred shading, shadow passes, or post-processing will require rewriting `SceneRenderer` logic. Multi-threaded scene traversal is impossible while rendering is tightly coupled to traversal.
- **Suggestion**: Separate scene traversal (build a list of renderables with world transforms/material info) from backend submission. Introduce explicit render queues or command buffers in `GraphicsDevice`, and encapsulate state objects (blend, depth, rasterizer) rather than ad-hoc setters. This will also ease future Vulkan/Metal backends.

## Resource & Scene Ownership
- **Problem**: Models own vectors of `unique_ptr<StaticMesh>` and raw Texture pointers, but there is no `ResourceCache`. Loading the same asset twice duplicates GPU resources, and there’s no lifecycle tracking for shared textures/materials.
- **Risk**: Memory bloat and dangling pointers when models outlive their textures. Editor/runtime cannot hot-reload resources safely.
- **Suggestion**: Implement the planned `Resource` subsystem now. Centralize texture/mesh creation with reference counting and path-based lookup so multiple models share GPU resources. Tie resource lifetimes to the context or a cache to ensure deterministic unloading.

- **Problem**: `Node::setParent` manipulates ownership by releasing/re-wrapping `unique_ptr` instances pointing to `this`.
- **Risk**: Exceptions or future changes can leak nodes. Reparenting relies on vector reallocation, so raw pointers to nodes become invalid.
- **Suggestion**: Manage nodes through IDs/handles or store them in a pool managed by `Scene`. Children vectors hold lightweight handles, and reparenting only updates references without reassigning ownership of `this`.

## Platform & UI Integration
- **Problem**: CocoaWindow polls NSEvents and forwards them to CocoaInput, while ImGui’s backend installs its own NSEvent monitor. Two independent consumers race on the same event stream, causing inconsistencies (modifiers, mouse coordinates).
- **Risk**: Any change to event handling requires touching platform code and UI backend simultaneously, and swapping out ImGui would repeat this work.
- **Suggestion**: Route events through a single dispatcher owned by the Window subsystem. Input, UI, and other consumers subscribe to the dispatcher, so platform code only knows about one interface. Keep ImGui-specific logic inside the UI subsystem instead of the platform layer.

- **Problem**: UI wrappers aim to abstract ImGui but still expose ImGui flags/concepts and duplicate boilerplate (e.g., RAII wrappers). This indirection complicates usage without offering a true cross-UI abstraction.
- **Suggestion**: Either embrace raw ImGui usage in higher layers (samples, editor) or architect a genuine UI abstraction with backend-agnostic concepts and adapters per UI toolkit.

## Testing & Validation
- **Problem**: Critical subsystems (Context factory overrides, LightManager, Scene traversal, platform input) lack automated tests.
- **Risk**: Bugs like light slots being skipped or modifier keys never toggling recur after refactors without early detection.
- **Suggestion**: Add lightweight tests/executables that spin up the Context with mock subsystems, assert lifecycle order, and verify LightManager behavior. For platform code, add integration tests (or at least scripted samples) that assert expected modifier/mouse states to catch regressions quickly.
