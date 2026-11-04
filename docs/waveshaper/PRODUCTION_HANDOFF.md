> Authoring handoff retained for provenance. Commands and paths below refer to
> the private candidate package; its scripts are not shipped here. Build this
> repository with the root CMake project and run CTest for integrated checks.

# Repository-ready WaveShaper adapter

This package adapts the independently approved Type5 candidate to `mfrk::waveshaper`, matching current Harmo-style private tables/state and an unconditional test-only friend. The approved baseline is unchanged in the parent evidence directory; its review and full handoff are retained under provenance/. No tracked repository, build workflow, vendor instance, GUI or REA session was changed.

## Production files and wiring

- `include/mfrk/waveshaper.hpp`: private `Tables` and `Engine`, explicit zero-arena factory, typed input/status API.
- `src/waveshaper.cpp`: mathematical generation and all recovered implementation bodies. The global bit helper is implementation-private; native CVTT conversion is a private static helper.
- `PATH_MAP.json`: exact proposed source-to-repository paths for production, tests, fixtures and docs.
- `CMAKE_INTEGRATION_SNIPPET.cmake.txt`: proposed `mfrk_core`/CTest wiring only; root owns applying it.

All 25 moved table, numeric, lifecycle and transaction bodies remain byte-identical to the approved header bodies. `scripts/check_bodies.py` verifies them against the retained body hashes. Only namespace, declarations, helper visibility and storage access changed. Tables no longer expose mutable arrays, and no raw runtime state is public. The same class definition is used in production and test translation units.

Read [ADMISSION.md](ADMISSION.md) before integration. `Engine` has no accessible default constructor. The explicit zero-arena factory requires a subsequent `initialize()` call; reset and phase remain native no-ops. Release-active `try_*` status/rollback behavior is preserved. Never add an unproved numeric clamp or claim a failed native trajectory produces equivalent silence.

## Immutable reference data

`fixtures/waveshaper_vectors.bin` packs the unchanged 1,353,730-byte WSHP0001 reference, canonical SHA256 `8badb67258aa2dd9b16790d33f4bce40315c7e4aab2f2906099fbdde5674cfcb`, into 988,971 bytes. `fixtures/waveshaper_lifecycle_vectors.bin` packs the unchanged 144,396-byte WSHPL001 reference, canonical SHA256 `d7051ec06e8c742ce69a26fe1ba1f8e2553406dbb4ed0910216c629ff1158745`, into 82,715 bytes. Both canonical originals remain in this package for audit.

The WSPK0001 transport stores decoded size/CRC32 followed by bounded zero/literal packets of at most 128 bytes, reusing the repository's existing packet algorithm. All original table, constructor, state and sample bytes survive a byte-identical roundtrip. No expected output or coverage was altered. Readers support packed and canonical inputs, enforce bounds/framing, and check canonical length/CRC before engine operations. Verification also checks SHA256. These are trusted reference-fixture readers, not host or arbitrary-state parsers.

The matched processor SHA256 is `bd036dfa6a0afcfef733c29c73b1b3df969f2c86ca860b0fa535d4492fa8fb74`. Expectations execute original instructions with original static PE tables; mathematical table outputs are compared independently. Provenance includes constructor preservation, pinned MIT mathematical sources/license, slot-clamp evidence and original undefined-read cases. No opaque waveform snapshots are embedded in production.

## Build and verification

From the repo, after dot-sourcing `scripts/Enter-ProjectEnvironment.ps1`:

```powershell
../local/tools/reference-python/python.exe ../local/evidence/waveshaper-engine-agent/production-candidate/scripts/check_bodies.py
../local/tools/reference-python/python.exe ../local/evidence/waveshaper-engine-agent/production-candidate/scripts/verify.py
```

A single strict Release host check can be built directly:

```text
g++ -std=c++20 -O2 -DNDEBUG -fno-fast-math -ffp-contract=off -Wall -Wextra -Wpedantic -Wconversion -Werror -I include tests/waveshaper_test.cpp tests/waveshaper_allocation_probe.cpp src/waveshaper.cpp -o build/gold.exe
build/gold.exe fixtures/waveshaper_vectors.bin
```

Run that command from this package with the D: environment. Root should compile the mapped files on Linux and both Android ABIs before merge; startup libm equivalence is deliberately checked rather than assumed. Preserve `/fp:strict` or `-fno-fast-math -ffp-contract=off` on the library and tests.

The suite covers all 1,612 blocks, 51,584 raw and 51,584 scaled outputs, 25,792 slope and 25,792 folded samples, 348,192 prepared and 348,192 final state bytes, all 2,313 table values, constructor/Init profiles, and 320 lifecycle/interface/helper commands. Test-only allocation hooks observe ordinary C++ allocations around render/prepare/process; production uses fixed arrays and no allocation API. The independent reviewer's 39 additional release-guard checks are included, along with the original failure/rollback and 512-block defined stress tests.

Eight semantic mutants, checked/debug execution and malformed canonical/packed data test regression sensitivity and bounds. `verification-report.json` and `delivery-manifest.json` retain actual fresh results and hashes. Repository/CI wiring and target-platform publication remain root-owned.
