# Benchmark experiments and offline reports

The pipeline has two independent stages: **collect CSV** with the C worker,
then **render a portable site** using saved CSV. Python 3.9+ standard library
only; no display server, packages, CDN, JavaScript or network required.
The historical `./bin/theartoffun --benchmark` remains unchanged.

The full catalog contains 91 individual algorithms/functions (including two libm
references), not just group comparisons. It covers all public algorithm APIs in
`algos.h`, `fib_algos.h`, `pow_algos.h`, `compressing.h`, `mortoncode.h` and
`levenshtein.h`, including generator initialization and algorithms exposed by CLI
operations. `print_hex` is explicitly excluded as console I/O; CLI parsing, help
rendering, `get_seed`, benchmark orchestration, private helpers and historical
unsafe/interactive standalone programs are not algorithm workloads. `benchmarks/coverage.json`
records exclusions and legacy benchmark-name aliases. A coverage test fails when
a new public algorithm has neither a benchmark nor a justified exclusion.

`benchmarks/examples.json` additionally accounts for **every `examples/*.c` file**:
17 sources map to measured functions; nine display-only/unsafe/interactive sources
have explicit reasons for exclusion. `benchmarks/cli.json` maps every CLI option
to benchmark functions (except help, exponent configuration and suite invocation).
Tests detect new unaccounted example files/functions or CLI options. Reports include
these mappings in metadata and a separate `coverage.html` page, including exclusions.

Example adapters in `example_algorithms.c` compile the actual example sources with
renamed demo entry points, not rewritten algorithm copies. GJF64 and Tyche include
both initialization and generation benchmarks. KMP/Boyer–Moore, preprocessing,
Karatsuba/helpers, Floyd cycle detection, Floyd–Warshall, Bresenham rasterization,
knight attacks, XOR fold, XXTEA encrypt/decrypt, fuzzy comparison and all four
inverse-square-root functions are individually registered. Demo mains, formatting
and canvas helpers are not separate algorithm entries.

Example workloads currently support **only `default`**: do not interpret them as
the shared five-scenario inputs. Variable-size adapters use deterministic bounded
data: search text repeats four letters, pattern `abca`; cycle input is a valid ring;
XXTEA uses uint32 arrays and restores each operation (copy cost included). Fixed
geometry canvases/4-vertex Floyd–Warshall use size 1 in the catalog and are not size
scaling experiments. KMP and Floyd–Warshall normally print results; the adapter
redirects formatted values to an in-memory checksum with **formatting cost included**,
not terminal I/O. Their absolute timing is not directly comparable to pure-return
functions. Example adapters disable inlining to retain real function calls; no
accuracy references are registered for these example-only functions yet.

## Quick start

```sh
make benchmark-list
make benchmark
# Copy the exact run directory printed by the collector:
make benchmark-plots BENCHMARK_INPUT=results/benchmarks/RUN PLOT_OUTPUT=results/plots/RUN
# Open results/plots/RUN/index.html
```

Every report includes `site.zip`: extract it anywhere and open `index.html`.
It contains the report, SVGs, raw CSV, metadata and statistical summaries,
but not executables. The archive deliberately excludes itself; the download
link inside an extracted copy refers to an archive you may regenerate or copy
alongside it. All other report links work offline.

The main page is a lightweight text catalog: algorithm names, scenarios, measured
conditions and links to graphs/CSV. No images, scripts or external resources are
loaded. Algorithm pages list SVG links and statistics; environment/method,
comparisons and accuracy summaries have separate pages. All pages use the system
serif font and a small inline stylesheet with no cards or rounded corners.

To delete **all saved runs, sites and archives in the repository's `results/`**:

```sh
make clean-results
```

This is an explicit destructive command, separate from `make clean`, which still
preserves experiment results. Custom output paths outside `results/` are not deleted.

```sh
# Focused experiment, all five data scenarios:
make benchmark BENCH_ARGS="--algorithms binary_pow libm_pow fast_pow fastest_pow --sizes 64 256 1024 --samples 7 --processes 3 --min-time-ms 20"

# Quick smoke run, not publication-quality measurements:
make benchmark BENCH_ARGS="--algorithms isqrt div3 --scenarios random --sizes 64 --samples 2 --processes 1 --min-time-ms 1 --iterations 100 --warmups 1"

# A single PRNG workload (size automatically becomes 1):
make benchmark BENCH_ARGS="--algorithms xorshift64 pcg32"
```

Defaults: all algorithms and compatible scenarios, sizes 64/256/1024/4096,
seven samples per process, three independent sequential processes per workload,
two warmup batches, minimum batch duration 20 ms, initial 10,000 iterations,
seed 42, timeout 120 seconds per worker. A full run can take several minutes.
`--iterations` is the **initial calibration count**, not a fixed batch size.
`--scenarios` filters the catalog; selecting no compatible scenario for an
algorithm is an error. PRNGs support only `default`; others support five scenarios.

Limits: unique sizes 1–65,536; samples/warmups 1–1,000; processes 1–100;
initial iterations 1–1e9; seed 1–2³²−1; target duration 1–10,000 ms.
Duplicate names and unknown scenarios/algorithms are rejected.

`BENCHMARK_OUTPUT` changes the CSV parent directory. `PLOT_OUTPUT` chooses
the site directory, which must be new/empty and disjoint from input. Collection
and site generation stage their outputs before publication. Failed workers,
timeouts, invalid data or rendering errors do not publish partial results.
Existing runs are never overwritten. `make clean` preserves Git-ignored `results/`.

Direct invocation (no Make required for offline reporting):

```sh
python3 benchmarks/run.py --runner bin/benchmark --output results/benchmarks --help
python3 benchmarks/plot.py --input results/benchmarks/RUN --output results/plots/RUN
```

## Structure

```text
benchmarks/
  runner.c      Algorithm catalog, input scenarios, timed batches, accuracy probes
  run.py        Calibration invocation, shuffled process schedule, environment capture
  data.py       CSV helpers and schema constants
  plot.py       Timing validation, reusable SVG charts, atomic site publication
  report.py     Accuracy validation, summaries, HTML report, downloads and ZIP

results/benchmarks/RUN/
  metadata.json
  GROUP/ALGORITHM/samples.csv
  GROUP/ALGORITHM/accuracy.csv        # only with a registered reference

results/plots/RUN/
  index.html                         # lightweight algorithm catalog, graph/CSV links
  experiment.html                    # environment, method, parameters, warnings
  comparisons.html                   # links to group comparison SVGs
  accuracy.html                      # accuracy summary table
  report.json                        # machine-readable report
  summary.csv                        # timing count, median, min/max, mean, stdev, CV
  accuracy-summary.csv               # per workload max/mean/median/p95 errors
  data/metadata.json
  data/GROUP/ALGORITHM/*.csv          # downloadable original data
  GROUP/SCENARIO/comparison.svg
  GROUP/SCENARIO/SIZE-tradeoff.svg
  GROUP/ALGORITHM/SCENARIO/timing.svg
  GROUP/ALGORITHM/SCENARIO/SIZE-{absolute,relative}.svg
  GROUP/ALGORITHM/SCENARIO/SIZE-{absolute,relative}-cdf.svg
  site.zip
```

## Scenarios and references

Scenarios are shared, deterministic input builders, not duplicated per algorithm:

Additional workload contracts:

- Fibonacci bounds N to `input % 94`, avoiding uint64 overflow beyond F(93).
- Leibniz pi uses the raw input as its internal series length; the batch count
  means complete pi evaluations, not terms. No numerical accuracy reference is
  currently registered for this workload.
- Morton encode maps raw input to uint16 coordinates `(x, x XOR 0xaaaa)`;
  decode takes the raw input as its Morton code.
- Levenshtein compares generated text with a copy mutated at every seventh
  character. Its quadratic workload is capped at 256 characters: requested sizes
  are clamped/deduplicated by the collector, and the worker rejects larger sizes.
  The actual size is always recorded; the catalog exposes this limit.
- Shuffle restores a uint32 input array each operation; copying is included.
  Fletcher's word API uses `size` uint16 elements, not bytes.
- XOR list insertion resets a valid two-node neighborhood each operation;
  traversal measures a three-node link. Checksums consume logical outcomes,
  not nondeterministic pointer addresses.
- PRNG initialization functions are separate default-scenario workloads;
  global historical PRNGs are reset by `legacy_prng_seed` before each batch.
  The reset is deterministic, not a promise of statistical seed quality and not
  thread-safe. Its cost contributes to each batch. Default legacy CLI behavior
  is unchanged unless this API is explicitly called.

| Scenario | Scalar inputs | Byte inputs |
|---|---|---|
| random | seeded uniform values in 1..size | seeded lowercase text |
| sorted | ascending 1..size | ascending lowercase letters |
| reverse | descending size..1 | descending lowercase letters |
| repeated | fixed midpoint | one long run of `a` |
| boundary | alternating 1 and size | alternating `a` and `z` |
| default | PRNG state reset from seed | not applicable |

Sizes mean bytes for hash/sort/RLE and input range for scalars. Powers map raw
input to base `1 + input/size` with exponent 7. Inverse square roots use positive
raw input; conversions use raw miles; dates map into bounded valid dates.
PRNG size is always 1, preventing misleading size-scaling plots.

Counting sort includes restoring unsorted input each operation. Hash algorithms
share text; Jenkins also receives the operation counter as its hash seed.
RLE inputs contain no digits. Decode data is encoded before timing; decode
includes allocation and free. Generator state initialization is included in
each PRNG batch. Input generation itself is outside timing; scalar indexing,
checksum accumulation, copies and algorithm setup costs may be inside timing.

Accuracy probes are **separate worker invocations**, never part of timed loops.
They evaluate every generated scalar input (including duplicates) for that size
and scenario. Power uses double `pow`, rsqrt uses double `1/sqrt`, conversion
uses double multiplication by 1.609344. These are practical double-precision
references, **not arbitrary-precision or exact mathematical oracles**.

- Absolute error: `abs(actual-reference)`.
- Relative error: `absolute_error/abs(reference)`; blank/undefined for zero reference.
- Maximum: observed maximum over the sampled input set, **not a global bound**.
- P95: nearest-rank 95th percentile.
- Distribution: empirical cumulative distribution (ECDF), including zeros and repeats.
- Tradeoff chart: median ns/op versus maximum relative error at matching
  group/scenario/size. Lower-left is better. No cross-size pooling.

Charts use actual linear numeric axes, medians and min–max whiskers (not confidence
intervals). Timing samples from independent processes are pooled. Error-at-input
charts aggregate duplicate x values. Accuracy metrics are intentionally absent
for algorithms without numerical references: hashes, PRNGs, sorting, dates, bits.

## Measurement method and provenance

The worker measures CLOCK_MONOTONIC wall time. Before each independent process's
measurements it calibrates count towards `--min-time-ms`, bounded by 12 attempts
and 1e9 operations. Calibration and warmup are excluded from CSV. The target is
best-effort, not a guarantee under OS scheduling noise. Actual counts/durations
are saved per sample. All process/workload jobs run sequentially in seeded
shuffled order, even under `make -j`. No baseline subtraction or outlier removal.

Avoid concurrent workloads. Prefer release builds, stable CPU frequency/power
policy and CPU affinity. The report records observed affinity but does not change
it. CV >10% and batches below 1 ms produce warnings, not correctness failures.
Absence of warnings does not prove statistical significance or lack of bias.
Loop overhead can dominate very fast algorithms. Do not use debug/sanitizer runs
for performance comparisons. Approximate methods need not give equivalent answers.

Metadata includes CPU model, logical cores, reported physical memory, affinity,
OS, compiler `--version`, Make flags, parameters, catalog, Git revision/dirty
status and runner SHA-256. CPU/memory collection uses Linux `/proc`; unsupported
fields are explicitly unknown. This reports host physical memory, not necessarily
a container's effective limit. Compiler details are invocation metadata; direct
Python collection needs `BENCH_CC`, `BENCH_CFLAGS`, `BENCH_CPPFLAGS` to identify
the build. A dirty Git revision does not reconstruct local sources. Inspect
metadata before publishing; it contains machine and build details.

Keep algorithms in separate translation units and do not enable LTO for benchmark
library objects. The worker disables LTO and libm reference builtins. No hard speed
thresholds are used in tests or CI.

## CSV compatibility

Version 1 timing CSV is still supported for offline reports (scenario `legacy`).
Version 2 timing columns are:

```csv
schema_version,group,algorithm,axis,size,sample,iterations,seed,elapsed_ns,ns_per_op,checksum,scenario,process,order
```

`process` is one-based repetition of a workload; `order` is its position in the
shuffled schedule. `sample` is one-based within the process. `ns_per_op` equals
elapsed_ns/iterations. Checksum consumes algorithm results; it is not an accuracy
oracle. Accuracy CSV is:

```csv
schema_version,group,algorithm,scenario,size,point,input,actual,reference,absolute_error,relative_error
```

The renderer rejects malformed headers/rows, identity/path mismatches, duplicate
sample/point identities, nonfinite values, negative errors, inconsistent normalized
time or error arithmetic, incompatible within-process configurations, and accuracy
workloads without matching timing. It does not require a C executable to validate.

## Extend with minimal changes

1. Add a batch in `runner.c` (use `SCALAR`, `BUFFER`, or `PRNG` for standard shapes).
2. Add one `CASE(group, name, axis)` catalog entry. Catalog axis determines supported
   shared scenarios automatically. Use stable identifier names.
3. For numerical accuracy, use `SCALAR` plus `ACC(reference_family, name)` instead
   of `CASE`; register a new `reference_FAMILY` only for a new operation family.

The collector, directory layout, summaries, charts, downloads, ZIP and all-worker
tests discover catalog entries automatically. No Python algorithm-name lists are
needed. Document new workload semantics/reference contracts here. Complex custom
algorithms still need a batch implementation and meaningful input design.

```sh
make test-benchmarks
make check
```

Tests cover workers, deterministic checksums, scenarios, calibrated collection,
multiple processes, accuracy arithmetic, zero references, old CSV compatibility,
malformed input, valid SVG XML, metadata and portable ZIP contents.
