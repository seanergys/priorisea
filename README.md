# PrioriSea

PrioriSea is a priority manager for [Flux](https://flux-framework.org/). It
integrates at the end of Flux's priority-assignment pipeline and lets
administrators define custom priority policies for jobs through configurable
formulas, recurring time windows and job/user scoring. Formulas can also
integrate with [SeanerBUS](https://seanerbus.org) to pull external information
about jobs into the policy.

## Components

PrioriSea is split into four components:

- a Flux job-manager plugin (`sea_priority.so`) that assigns a priority to
  newly scheduled jobs,
- an SQLite database that stores the configuration and state used to compute
  priorities (formulas, time windows, jobs and users with their scores),
- a daemon, which is the core of PrioriSea: it owns the database and executes
  the business logic,
- the `priorisea` CLI, which reads and writes that state.

The CLI and the daemon communicate over the Flux data-plane with a 1-to-1
endpoint mapping: every CLI command maps to an RPC handled by the daemon.
Because the plugin runs inside the Flux instance while the daemon runs as a
standalone process, changes that must reach the plugin (formula, score or time
window updates) are transmitted with `priorisea plugin sync`.

See [docs/architecture.rst](docs/architecture.rst) for details.

## Concepts

- **Formulas** — custom expressions (powered by
  [tinyexpr](https://github.com/codeplea/tinyexpr)) that compute a job's
  priority, a job's post-run score and the age-based weight used to aggregate
  user scores. PrioriSea ships with three mandatory formulas: `job_priority`,
  `job_score` and `job_weight`.
- **Job score** — a job's performance score, computed post-run with
  `priorisea job update`.
- **User score** — a weighted average of the scores of the jobs a user ran,
  used as an input to the `job_priority` formula.
- **Time windows** — recurring time frames (defined by a cron expression, a
  duration and a score) that influence the score of jobs running during them.

See [docs/index.rst](docs/index.rst) for the full concepts overview.

## Quick start

PrioriSea requires a running Flux instance, as it uses the Flux data-plane for
communication between the CLI and the daemon.

1. Create the database:
   ```sh
   priorisea database create /path/to/database.db
   ```
2. Start the daemon, pointing at the same database file:
   ```sh
   priorisea daemon start /path/to/database.db
   ```
3. Load the job-manager plugin:
   ```sh
   flux jobtap load sea_priority.so
   ```
4. Synchronise the plugin with the database:
   ```sh
   priorisea plugin sync
   ```
5. Verify:
   ```sh
   priorisea formula list
   priorisea plugin debug
   ```

Re-run `priorisea plugin sync` after every change to the formulas, the scores
or the time windows.

Build and install instructions are in
[docs/getstarted.rst](docs/getstarted.rst).

## Documentation

The full documentation lives in [docs/](docs/):

- [Architecture](docs/architecture.rst) — how the components fit together
- [Getting started](docs/getstarted.rst) — build, install and first steps
- [Daemon](docs/daemon.rst) — business logic, state, plugin synchronisation and commands
- [Database](docs/database.rst) — the SQLite schema and its contents
- [Flux plugin](docs/plugin.rst) — the `sea_priority` jobtap plugin
- [Formula](docs/formula.rst) — formula syntax, variables, SeanerBUS and commands
- [Job score](docs/job.rst), [User score](docs/user.rst) and
  [Time window](docs/timewin.rst) — the scoring concepts and their commands

## Tests

Unit tests live under `src/lib/test/` and are written with
[Catch2](https://github.com/catchorg/Catch2).

The user-facing behaviour tests live under `tests/` and are TAP tests using
[sharness](tests/sharness.sh).

To both build the software and run the tests, run:
```sh
git submodule update --init --recursive
cmake -S . -B build && cmake --build build -j && ctest --test-dir build --parallel 0
```
