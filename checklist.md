# PoulpeEngine Portfolio Upgrade Checklist

This checklist is organized into **13** PR-sized steps to raise professional quality, performance, and modern C++ standards.
Each item was validated against the actual codebase (audit date: Feb 2026).

---

## PR 1 — Correctness hotfixes (highest priority)

Goal: remove obvious reviewer-blocking bugs before any refactor.

Files:
- src/Engine/Renderer/Vulkan/VulkanAPI.cpp
- src/Engine/Component/Mesh.cppm

### Audit findings

**Bug 1 — `checkValidationLayerSupport` always returns `false` (line 284).**
The function correctly returns `false` early when a layer is missing, but the success path at the end of the function also returns `false` instead of `true`.

**Bug 2 — Inverted caller condition (line 186).**
The condition `!isValidationLayersEnabled() && !checkValidationLayerSupport()` only checks support when validation layers are *disabled*. Likely should be `isValidationLayersEnabled() && !checkValidationLayerSupport()`.

**Bug 3 — Pointer comparison instead of string comparison (line 274).**
`layer_name.c_str() == layer_props.layerName` compares pointer addresses, not string contents. Must use `strcmp` or compare via `std::string`.

**Typo — `_has_push_contants` (Mesh.cppm lines 40, 56, 95).**
Member variable, setter, and getter all use `contants` instead of `constants`.

Checklist:
- [x] Fix `checkValidationLayerSupport` to `return true` on the success path (line 284).
- [x] Fix the inverted condition at line 186: `isValidationLayersEnabled() && !checkValidationLayerSupport()`.
- [x] Replace pointer comparison at line 274 with `strcmp` or `std::string` comparison.
- [x] Rename `_has_push_contants` → `_has_push_constants` in member, setter, and getter (Mesh.cppm lines 40, 56, 95).
- [x] Run build preset and verify zero new warnings in touched files.

Done when:
- Validation layer function has a correct passing path.
- Validation layers are actually checked when enabled.
- String layer names are compared by content.
- No inconsistent push-constant naming remains in public API.
- Build validates after fixes (`cmake --build --preset windows-release`).

---

## PR 2 — Lifetime safety cleanup

Goal: eliminate risky dangling-reference patterns.

Files:
- src/Engine/Core/PlpTypedef.cppm

### Audit findings

`EntityOptions` (lines 57-58) stores two `glm::vec3 const&` members:
```cpp
glm::vec3 const& pos{};
glm::vec3 const& scale{};
```
If constructed from temporaries or stack locals that go out of scope, these become dangling references — classic undefined behavior.

Checklist:
- [x] Replace `glm::vec3 const&` members with `glm::vec3` value members in `EntityOptions`.
- [x] Update constructors/setters accordingly.
- [x] Grep callers and adjust copies/moves explicitly.
- [x] Add small assertions or invariants where ownership assumptions remain.

Done when:
- Entity/options types no longer store references to external temporaries.

---

## PR 3 — CMake dependency order and target hygiene

Goal: make configuration deterministic across machines.

Files:
- CMakeLists.txt (lines 29-31)
- cmake/modules/fetchDeps.cmake
- cmake/modules/subdirs.cmake

### Audit findings

In `CMakeLists.txt`, `include(subdirs)` (line 30) runs **before** `include(fetchDeps)` (line 31). This means `add_subdirectory()` calls for engine modules execute before external targets (assimp, glfw, glm, fmt, etc.) are fetched and available. Currently works by luck due to deferred CMake evaluation, but is fragile and non-portable.

Checklist:
- [x] Swap lines 30-31: move `include(fetchDeps)` before `include(subdirs)`.
- [x] Verify imported/alias targets exist before use in each subdirectory.
- [x] Keep subdirectory order aligned with dependency graph.
- [x] Regenerate from a clean build folder and validate no configure warnings.

Done when:
- Fresh configure succeeds without order-sensitive failures.

---

## PR 4 — Debug quality gates (sanitizers + presets)

Goal: catch UB/memory issues early.

Files:
- CMakePresets.json
- CMakeHiddenPresets.json

### Audit findings

No working sanitizer presets exist. There is an orphaned hidden preset `ASAN_LIBRARY_PATH` that is dead config.

Checklist:
- [x] Remove or integrate the orphaned `ASAN_LIBRARY_PATH` preset.
- [x] Add a working debug preset with `-fsanitize=address,undefined` in both compile and link flags.
- [x] Ensure runtime flags (`ASAN_OPTIONS`, `UBSAN_OPTIONS`) are documented.
- [x] Test that the sanitizer preset configures and builds on at least one supported compiler (Clang recommended).

Done when:
- Sanitizer preset configures, builds, and runs successfully.
- Sanitizer presets added and validated locally (`windows-debug-asan` / `linux-debug-asan`).

---

## PR 5 — API cost reductions on hot paths

Goal: reduce copies and improve frame-time consistency.

Files:
- src/Engine/Renderer/Vulkan/VulkanAPI.cppm
- src/Engine/Manager/EntityManager.cppm
- various managers

### Audit findings

Many functions return `std::vector` by value (copies):
- `VulkanAPI::getValidationLayers()`
- `EntityManager::getEntities()`
- `AudioManager::getAmbientSound()`

Checklist:
- [x] Change return types to `const&` for getters that expose internally-owned containers.
- [x] **[NEW]** Use `std::span<T>` (C++20) where callers only need a read-only view and data is contiguous, avoiding container coupling entirely.
- [x] Keep return-by-value only where callers genuinely need ownership transfer.

Done when:
- Hot-path APIs avoid unnecessary allocations/copies.
- `std::vector` is not exposed where `std::span` suffices.
- Build validates after API changes (`cmake --build --preset windows-release`).

---

## PR 6 — Minimal test harness

Goal: demonstrate engineering discipline; reviewers notice the complete absence of tests.

Files:
- tests/ (create)

### Rationale

No tests exist in the repository.

Checklist:
- [ ] Add a `tests/` directory with a lightweight test framework (Catch2 or GoogleTest via FetchContent).
- [ ] Write at least 2-3 unit tests targeting non-GPU code (e.g. `EntityOptions` construction).
- [ ] Integrate `ctest`.
- [ ] **[NEW]** Add `Google Benchmark` to `tests/` for performance tracking (supports PR 12).

Done when:
- `ctest` runs and passes from a clean build.

---

## PR 7 — Coupling reduction pass (architecture polish)

Goal: improve maintainability signal for portfolio reviewers.

Files:
- src/Engine

### Scope warning
Keep scope tight — pick **one concrete decoupling**.

Checklist:
- [ ] Pick one manager-to-manager concrete dependency and replace it with a narrow interface/callback.
- [ ] Keep wiring centralized (single composition root).

Done when:
- At least one cross-manager dependency is broken into an abstraction.

---

## PR 8 — CI improvements

Goal: show professional delivery process.

Files:
- .github/workflows/CI.yml

### Gaps
No Windows build job, no sanitizer job.

Checklist:
- [ ] Add a Windows build matrix entry.
- [ ] Add a sanitizer job.
- [ ] Add a `ctest` step after build.

Done when:
- Both Linux and Windows get automatic pass/fail quality feedback.

---

## PR 9 — Architecture documentation for evaluators

Goal: make your engine understandable in 3-5 minutes.

Files:
- README.md
- docs/

Checklist:
- [ ] Add a high-level module map diagram.
- [ ] Add ownership/lifetime rules.
- [ ] Add build matrix and presets table.
- [ ] Add known limitations/roadmap section.

Done when:
- A reviewer can quickly grasp design decisions.

---

## PR 10 — Final portfolio hardening sweep

Goal: ship a clean, credible snapshot.

Checklist:
- [ ] Remove dead code and stale config duplicates.
- [ ] Ensure naming consistency.
- [ ] Capture 2-3 screenshots or GIFs.
- [ ] Final grep for typos.

---

## PR 11 — Modern C++23 Integration

Goal: Leverage latest C++ features for cleaner, dependency-free code.

Files:
- src/Engine/Core/Logger.cppm
- src/*

Checklist:
- [ ] Replace `fmt::print` with `std::print` (C++23) in `Logger.cppm` (reduces external usage dependencies).
- [ ] Use `std::to_underlying` (C++23) for enum casts (replaces `static_cast<int>(enum)`).
- [ ] Verify `import std;` usage is maximized reducing individual header includes.
- [ ] Use `if consteval` for compile-time logic optimizations where applicable.

Done when:
- Codebase reflects bleeding-edge C++ standards declared in user profile.

---

## PR 12 — Functional + DOD C++26 Core Pass

Goal: combine functional-style C++26 refactoring with data-oriented design improvements to improve maintainability and runtime performance together.

Files:
- src/Engine/Manager/EntityManager.cppm
- src/Engine/Manager/ComponentManager.cppm
- src/Engine/Manager/RenderManager.cpp
- src/Engine/Core/*

### Top 3 starting spots (priority order)
1. `RenderManager::renderScene` orchestration split (functional frame pipeline + frame-local buffers/preallocation).
2. `ComponentManager::get<T>` lookup redesign (replace linear scan with direct/indexed lookup path).
3. Culling + draw preparation path (`isClipped` / `renderEntity` / `drawEntity`) as pure transform stages over compact ID/data views.

### Phase roadmap (for spot 1)
- Phase 0: baseline and safe extraction (`FrameContext` builder + no behavior change in side-effect order).
- Phase 1: split `renderScene` into explicit stage functions (gather -> cull -> update -> draw/submit).
- Phase 2: functionalize stage boundaries (explicit input/output structs, reduced hidden shared state).
- Phase 3: DOD pass inside frame loop (reusable frame-local buffers, reduced transient allocations/lookups).
- Phase 4: targeted parallelism cleanup (batch tasks by stage, reduce tiny async task overhead).
- Phase 5: benchmark and stabilize (before/after checks, regressions guard, code cleanup/documentation).

### Focus Areas
- Prefer pure transformation functions and immutable-by-default local flow in update/render preparation logic.
- Replace ad-hoc loops with `std::ranges` pipelines where readability and safety improve.
- Use stronger type-safe APIs (`std::span`, `std::expected`, `std::optional`, `std::variant`) instead of implicit sentinel/error flows.
- Improve memory layout and iteration locality in hot paths (reduce pointer chasing, prefer contiguous storage/views where safe).

Checklist:
- [ ] Identify 2-3 hot-path functions and refactor them into side-effect-light pipeline stages (input -> transform -> output).
- [ ] Replace mutable temporary containers in those paths with views/ranges composition when possible.
- [ ] Introduce explicit error/value flow (prefer `std::expected` for recoverable operations).
- [ ] Apply one focused DOD refactor in a hot loop (contiguous data access, reduced indirection, or handle/index-based access where appropriate).
- [ ] Use C++23/26 utilities already available in your toolchain (`std::to_underlying`, `std::print`, constrained templates/concepts, improved constexpr usage).
- [ ] Keep performance guardrails: benchmark before/after using the PR6 benchmark harness.

Done when:
- Core update/render preparation paths are visibly more functional and compositional.
- At least one hot path shows improved locality/data access pattern from DOD changes.
- C++26-oriented APIs are used consistently in touched modules.
- Benchmarks show no regression (or measurable improvement).

---

## PR 13 — Vulkan Modernization (Bindless Textures)

Goal: reduce texture binding overhead and simplify material/texture management by moving to bindless texture access.

Files:
- src/Engine/Renderer/Vulkan/VulkanAPI.cpp
- src/Engine/Renderer/Vulkan/Renderer.cppm
- src/Engine/Manager/TextureManager.cppm
- src/Engine/Manager/ShaderManager.cppm

### Audit Findings
Dynamic Rendering is already in place. The higher-value next step is bindless textures via descriptor indexing to avoid per-draw texture descriptor rebinding.

Checklist:
- [ ] Enable required Vulkan features/extensions for descriptor indexing (bindless) during device creation.
- [ ] Introduce a global texture descriptor array (`VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT`, `UPDATE_AFTER_BIND`, variable descriptor count as needed).
- [ ] Update texture/material data flow so draw calls pass texture indices instead of per-material descriptor sets.
- [ ] Update shaders to sample from bindless arrays using texture indices.
- [ ] Keep a fallback path or guard for devices without bindless support.
- [ ] Rebuild and validate rendering path correctness with multiple materials/textures.

Done when:
- Renderer uses bindless texture indexing in the main path.
- Per-draw descriptor set churn for textures is removed or significantly reduced.
