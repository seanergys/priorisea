# PrioriSea

PrioriSea is a priority manager for [Flux](https://flux-framework.org/). It
integrates at the end of Flux's priority-assignment pipeline and lets
administrators define custom priority policies for jobs through configurable
formulas, recurring time windows and job/user scoring. Formulas can also
integrate with [SeanerBUS](https://seanerbus.org) to pull external information
about jobs into the policy.

## Documentation

Concepts, quick start, build and install instructions,... — lives in the [documentation](docs/):

- [Architecture](docs/architecture.rst) — how the components fit together
- [Deployment](docs/deployment.rst) — get running and first steps
- [Tutorial](docs/tutorial.rst) — a guided first-policy walkthrough
- [Formula](docs/formula.rst) — formula syntax, variables and SeanerBUS
- [CLI reference](docs/cli-reference.rst) — all commands, grouped by subcommand
- [Build](docs/build.rst) — build and install from source
- [Glossary](docs/glossary.rst) — key domain terms
- and more: [daemon](docs/daemon.rst), [database](docs/database.rst),
  [plugin](docs/plugin.rst), [job score](docs/job.rst),
  [user score](docs/user.rst), [time window](docs/timewin.rst) and
  [the carbon-aware how-to](docs/howto-carbon-aware.rst)
