# PrioriSea

Seanergys' priority plugin for [Flux](https://flux-framework.org/).

PrioriSea plugs into the end of Flux's priority-assignment pipeline and lets
administrators define custom priority policies for jobs through configurable
formulas, recurring time windows and a score-decay system. Formulas can also
integrate with [SeanerBUS](https://seanerbus.org) to pull external information
about jobs into the policy.

## Components

PrioriSea is split into three components:

- a Flux job-manager plugin (`sea_priority.so`) that assigns a priority to jobs
  at submission time,
- a database (backed by the Flux KVS) that stores formulas, scores, time windows
  and decay expressions,
- the `priorisea` CLI, which reads and writes that state and keeps the running
  plugin in sync through the Flux dataplane.

See [docs/architecture.rst](docs/architecture.rst) for details.

## Quick start

1. Load the plugin: `flux jobtap load sea_priority.so`
2. Initialize the database and plugin with the default values: `priorisea init`
3. Verify: `priorisea formula list` and `priorisea decay get`

Build and install instructions are in
[docs/getstarted.rst](docs/getstarted.rst).

## Documentation

The full documentation lives in [docs/](docs/):

- [Getting started](docs/getstarted.rst) — build, install and first steps
- [CLI reference](docs/cli.rst) — every `priorisea` command and subcommand
- [Configuration](docs/configuration.rst) — configuration file, environment
  variables and the SeanerBUS endpoint
- [Concepts](docs/index.rst) — formulas, job and user scores, decay and time
  windows
- [C++ API](docs/lib.rst) — the `src/lib` public API

## Tests

Unit tests live under `src/test/` and are written with [Catch2](https://github.com/catchorg/Catch2).

The user-facing behaviour tests live under `tests/` and are covered by TAP tests using
[sharness](tests/sharness.sh):

To run both the unit tests and the user-facing behaviour tests, run:
```sh
make check
```
