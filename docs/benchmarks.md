# Benchmark collection and offline plots

The structured benchmark pipeline is separate from the historical, human-readable
`./bin/theartoffun --benchmark`. It has two independent stages:

1. A C worker measures algorithms; a Python collector publishes raw CSV and run metadata.
2. An offline Python renderer reads a saved run and creates SVG charts and statistical summaries.

Requirements: the normal C build toolchain and Python **3.9+**, standard library only.
No matplotlib, display server, network access, or Python package installation is needed.

## Collect measurements

```sh
make benchmark-list
make benchmark

# Select algorithms, sizes, repetitions, and seed.
make benchmark BENCH_ARGS="--algorithms jenkins_hash fnv1a_hash --sizes 64 256 1024 4096 --iterations 20000 --samples 9 --warmups 2 --seed 42"

# Small end-to-end experiment.
make benchmark BENCH_ARGS="--algorithms isqrt div3 --sizes 64 256 --iterations 1000 --samples 3 --warmups 1"
```

The collector prints progress to stderr and the completed run directory to stdout.
Every invocation creates a unique UTC timestamp/UUID run. It never overwrites a previous
run. CSV is staged in a temporary directory and published only when **all** workers
succeed; errors and timeouts return a nonzero exit status. `--timeout` (default 120)
limits each algorithm/size worker, not the entire collection.

Defaults: all catalog algorithms, sizes `64 256 1024 4096`, 10,000 operations
per sample, seven samples, two warmup batches, seed 42. All warmups use the same
iteration count as measurements and are excluded from CSV. Limits are checked
before collection: sizes 1–65,536, samples/warmups 1–1,000, iterations 1–1e9,
seed 1–2³²−1. Duplicate sizes/algorithm names and unknown names are rejected.

```text
results/benchmarks/20261109T120000Z-a1b2c3d4/
├── metadata.json
├── integer/
│   ├── isqrt/samples.csv
│   └── div3/samples.csv
└── hash/
    ├── jenkins_hash/samples.csv
    └── fnv1a_hash/samples.csv
```

`BENCHMARK_OUTPUT` changes the parent directory. To use an existing executable,
run `python3 benchmarks/run.py --runner bin/benchmark --output results/benchmarks`.
Use `--help` for collector options. `make benchmark-build` builds without running.

## Render a saved run

Use the **exact directory printed by the collector**, not its parent:

```sh
make benchmark-plots \
  BENCHMARK_INPUT=results/benchmarks/20261109T120000Z-a1b2c3d4 \
  PLOT_OUTPUT=results/plots/20261109T120000Z-a1b2c3d4

# Equivalent, including on another machine with no C compiler:
python3 benchmarks/plot.py \
  --input results/benchmarks/20261109T120000Z-a1b2c3d4 \
  --output results/plots/20261109T120000Z-a1b2c3d4
```

Open `index.html` in the output directory. Output contains:

- `GROUP/ALGORITHM/timing.svg`: per-algorithm median ns/operation by size;
- `GROUP/comparison.svg`: overlay of algorithms within a group;
- `summary.csv`: count, median, minimum, maximum, mean and sample standard deviation;
- `index.html`: links to all charts.

Whiskers represent the observed minimum and maximum, **not confidence intervals**.
The x-axis places tested sizes at equal spacing (it is explicitly not a linear
numeric scale). Single-size runs are supported. No smoothing or outlier removal
is applied. Output must be new or empty to avoid stale charts from another run.
The renderer validates the entire input before writing: schema, finite positive
timings, directory identities, duplicate samples, per-algorithm configuration,
and consistency of total and normalized time.

`make benchmark-plots` has **no build or collection prerequisite**. It cannot
silently rerun measurements or replace source CSV. `make clean` removes binaries
and build products, but retains `results/`; generated results are Git-ignored.

## CSV protocol (version 1)

Each algorithm has one `samples.csv`, containing raw rows for all requested sizes.
The header is:

```csv
schema_version,group,algorithm,axis,size,sample,iterations,seed,elapsed_ns,ns_per_op,checksum
```

`sample` is one-based within a size. `elapsed_ns` is the monotonic wall-clock
duration of one batch; `ns_per_op = elapsed_ns / iterations`. `checksum` keeps
computed results observable; it is a reproducibility aid, not an accuracy oracle
or a cryptographic digest. Warmup rows are not stored.

`metadata.json` records UTC time, parameters, platform, Git revision/dirty status,
worker SHA-256, and compiler/flags passed by Make. When invoking Python directly,
build settings are unknown unless `BENCH_CC`, `BENCH_CFLAGS`, and
`BENCH_CPPFLAGS` are supplied. These fields describe invocation, not a complete
reproducible-build manifest; retain your toolchain version and environment when
publishing results. A dirty working tree is not reconstructed by its Git revision.

## Workloads and interpretation

The catalog covers integer/bit operations, powers (including libm), inverse square
root, distance conversions, dates, hashes/checksums, counting sort, RLE encode/decode,
and six explicitly seedable PRNGs. Historical global-state generators without a
reset API are intentionally not included. The legacy CLI suite remains available.

- `axis=bytes`: size is input byte length. Input is deterministic lowercase text
  generated from the seed. Hashes share the same data; Jenkins receives the loop
  counter as its hash seed. Fletcher includes its string-length work.
- Counting sort includes copying fresh, unsorted bytes before **every** operation;
  it does not repeatedly sort an already-sorted buffer.
- RLE uses lowercase text without digits, respecting the codec contract. This is
  a mostly incompressible workload, not a compression-ratio study. Decode input
  is pre-encoded outside timing; allocation and free are included in decode timing.
- `axis=input_range`: integers cycle over `1..size` with a seed offset. Powers
  use bases in `(1,2]` and exponent 7; inverse square roots use positive inputs.
  Date inputs are mapped into bounded valid dates. Input generation and checksum
  accumulation inside the batch contribute to reported time.
- `axis=batch_parameter`: PRNGs ignore size. For PRNG-only runs use `--sizes 1`;
  multiple sizes repeat the same workload, not a scaling experiment. Each batch
  resets generator state from the seed; reset cost is included in timing.

The C worker uses `CLOCK_MONOTONIC`, excludes file I/O from timing, and consumes
the checksum after timing. Algorithm calls are direct within batch loops and are
compiled in separate translation units; the worker disables LTO and compiler
builtins for the libm reference calls. Do not supply LTO flags for benchmark
library objects. No empty-loop subtraction or automatic iteration calibration
is used. Increase `--iterations` until batch durations are comfortably above
timer noise. Short smoke-test batches are **not** useful performance results.

Run release builds on an otherwise idle machine, preferably with a fixed power
policy/CPU affinity. Collection is sequential even under `make -j`; avoid other
heavy Make targets concurrently. Debug/sanitizer timings are not comparable to
release timings. Approximate power, conversion and inverse-square-root algorithms
do **not** guarantee the same accuracy as their references. A faster timing is
not evidence of equivalent answers, security, or superior randomness. Group
overlays organize data; not every pair in a group performs the same operation.

## Tests and extending the suite

```sh
make test-benchmarks
make check                 # includes benchmark pipeline tests
```

Tests use tiny workloads: every catalog worker, deterministic checksums, invalid
worker arguments, CSV collection, metadata, offline SVG parsing, summaries, and
rejection of malformed input. They never assert speed thresholds.

To add an algorithm, add a `work_NAME(size, iterations, seed)` batch in
`benchmarks/runner.c` and a `CASE(group, NAME, axis)` catalog entry. Use stable
identifier names, initialize state reproducibly, keep inputs within the algorithm
contract, and consume results. Document workload-specific costs here. The collector,
renderer, directory structure and all-worker tests discover the new entry without
hardcoded algorithm lists. Change the schema version if CSV semantics change.
