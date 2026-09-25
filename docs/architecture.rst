.. SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
..
.. SPDX-License-Identifier: MIT

Architecture
============

PrioriSea is split between three components: a Flux plugin,
a database and the CLI (command-line interface) tool.

When loaded using ``flux jobtap load sea_priority.so``, the Flux plugin reads the database
to load in-memory the job score formula as well as the user scores.

When updating or adding any value (formula, job score, user score, time window, decay expression)
using the CLI, the change is persisted to the database and also sent to the Flux plugin via the
Flux dataplane.

The CLI can be used to reset formulas and decay expression to their default values.
