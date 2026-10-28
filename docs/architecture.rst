Architecture
============

PrioriSea is split between five components:
- a Flux priority plugin
- an SQLite database
- a daemon connected to the Flux data-plane
- the CLI (command-line interface) tool.

The daemon is the main component of PrioriSea: it is connected to the database and receive RPC commands from
the CLI to both update the database and sync the data with the priority plugin.

When any command is typed in the CLI, a corresponding message is sent to the daemon via the Flux data-plane,
where it executes the operation. That operation may include communicating with the priority plugin. After the
operation has been executed, the daemon sends a response to the CLI.

All operations are persisted in the SQLite database administrated by the daemon. Since the priority plugin is a
separated component from the daemon, any change that should affect the priority plugin need to be transmitted using
the ``priorisea plugin sync`` command.
