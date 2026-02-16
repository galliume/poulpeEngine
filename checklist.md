# PoulpeEngine Portfolio Upgrade Checklist

This checklist is organized into **14** PR-sized steps to raise professional quality, performance, and modern C++ standards.
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
- [ ] Run build preset and verify zero new warnings in touched files.

Done when:
- Validation layer function has a correct passing path.
- Validation layers are actually checked when enabled.
- String layer names are compared by content.
- No inconsistent push-constant naming remains in public API.

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

## PR 4 — Warning policy professionalization

Goal: keep strictness without making contributor experience brittle.

Files:
- cmake/modules/compiler_flags.cmake

### Audit findings (status: partially already done)

Warning flags are **already applied per-target** via `set_poulpe_compiler_flags(target)` using `target_compile_options`. The actionable gap: `-Werror` is enabled in **all** configurations, making local development brittle. Also, `-Weverything` is enabled globally (line 36) which is aggressive.

Checklist:
- [x] Scope warning flags per target — already done via `set_poulpe_compiler_flags()`.
- [x] Gate `-Werror` behind a CMake option (e.g. `PLP_WERROR`, default OFF) so CI can enable it and local builds remain unbroken.
- [x] Consider replacing `-Weverything` with `-Wall -Wextra -Wpedantic` + specific `-W` flags.
- [x] Normalize compiler-specific flags (MSVC/Clang/GCC) with clear branches.

Done when:
- [x] Local dev build is robust; CI can still enforce strict mode via `-DPLP_WERROR=ON`.

---

## PR 5 — Debug quality gates (sanitizers + presets)

Goal: catch UB/memory issues early.

Files:
- CMakePresets.json
- CMakeHiddenPresets.json

### Audit findings

No working sanitizer presets exist. There is an orphaned hidden preset `ASAN_LIBRARY_PATH` that is dead config.

Checklist:
- [ ] Remove or integrate the orphaned `ASAN_LIBRARY_PATH` preset.
- [ ] Add a working debug preset with `-fsanitize=address,undefined` in both compile and link flags.
- [ ] Ensure runtime flags (`ASAN_OPTIONS`, `UBSAN_OPTIONS`) are documented.
- [ ] Test that the sanitizer preset configures and builds on at least one supported compiler (Clang recommended).

Done when:
- Sanitizer preset configures, builds, and runs successfully.

---

## PR 6 — API cost reductions on hot paths

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
- [ ] Change return types to `const&` for getters that expose internally-owned containers.
- [ ] **[NEW]** Use `std::span<T>` (C++20) where callers only need a read-only view and data is contiguous, avoiding container coupling entirely.
- [ ] Keep return-by-value only where callers genuinely need ownership transfer.

Done when:
- Hot-path APIs avoid unnecessary allocations/copies.
- `std::vector` is not exposed where `std::span` suffices.

---

## PR 7 — Minimal test harness

Goal: demonstrate engineering discipline; reviewers notice the complete absence of tests.

Files:
- tests/ (create)

### Rationale

No tests exist in the repository.

Checklist:
- [ ] Add a `tests/` directory with a lightweight test framework (Catch2 or GoogleTest via FetchContent).
- [ ] Write at least 2-3 unit tests targeting non-GPU code (e.g. `EntityOptions` construction).
- [ ] Integrate `ctest`.
- [ ] **[NEW]** Add `Google Benchmark` to `tests/` for performance tracking (supports item 13).

Done when:
- `ctest` runs and passes from a clean build.

---

## PR 8 — Coupling reduction pass (architecture polish)

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

## PR 9 — CI improvements

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

## PR 10 — Architecture documentation for evaluators

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

## PR 11 — Final portfolio hardening sweep

Goal: ship a clean, credible snapshot.

Checklist:
- [ ] Remove dead code and stale config duplicates.
- [ ] Ensure naming consistency.
- [ ] Capture 2-3 screenshots or GIFs.
- [ ] Final grep for typos.

---

## PR 12 — Modern C++23 Integration [NEW]

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

## PR 13 — Data-Oriented Design (DOD) Pass [NEW]

Goal: Improve cache locality and CPU performance.

Files:
- src/Engine/Manager/EntityManager.cppm
- src/Engine/Manager/ComponentManager.cppm

### Audit Findings
`EntityManager` stores `std::vector<std::shared_ptr<Entity>>`. Iterating this involves double indirection and cache misses.

Checklist:
- [ ] Benchmark current iteration performance using the new `Google Benchmark` harness (from PR 7).
- [ ] Refactor `EntityManager` to store entities/components more contiguously (e.g., `std::vector<Entity>` if possible, or flat Component arrays).
- [ ] Remove `std::shared_ptr` usage in hot loops; use raw pointers or handles/IDs.

Done when:
- Entity iteration is cache-friendly.
- Benchmarks show measurable improvement.

---

## PR 14 — Vulkan Modernization (Dynamic Rendering) [NEW]

Goal: Simplify renderer and reduce VRAM overhead.

Files:
- src/Engine/Renderer/Vulkan/VulkanAPI.cpp
- src/Engine/Renderer/Vulkan/Extension/ShadowMap.cpp

### Audit Findings
`VK_KHR_dynamic_rendering` is enabled (line 501 of VulkanAPI.cpp), but `vkCreateRenderPass` is still used (line 1340).

Checklist:
- [ ] Replace `VkRenderPass` and `VkFramebuffer` logic with `VkRenderingInfo` in command recording.
- [ ] Remove `vkCreateRenderPass` and `vkCreateFramebuffer` calls.
- [ ] Simplify pipeline creation (remove `renderPass` member from `VkGraphicsPipelineCreateInfo`).

Done when:
- The engine uses pure Dynamic Rendering (modern Vulkan 1.3 standard).
- Boilerplate code for RenderPasses is deleted.
