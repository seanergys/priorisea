Flux Plugin
===========

PrioriSea integrates with Flux as a job manager tap (jobtap) priority plugin. When
a new job is scheduled, the plugin is queried for the priority to assign to that job.
The priority is computed by evaluating the ``job_priority`` formula. See :doc:`formula`.

The priority plugin is a separate component from the daemon: it runs inside the Flux
instance while the daemon runs as a standalone process managing the database. Because
of this separation, any change that should affect the priority plugin (a formula
update, a user score update, a time window change, ...) must be transmitted to the
plugin using the ``priorisea plugin sync`` command.

The plugin can be managed using the ``priorisea plugin`` subcommand:

.. code-block:: bash

    priorisea plugin sync
    priorisea plugin debug

Commands
--------

The running Flux plugin is managed through the ``priorisea plugin`` subcommand.

.. code-block:: bash

    priorisea plugin sync
    priorisea plugin debug [--json]

``sync``
    Synchronise the plugin with the database. It propagates the current database
    state (formulas, time windows, scores) to the running plugin and should be
    run after every change to the formulas or the scores. Users with no score are
    not synced with the plugin. This command produces no output.

    .. code-block:: bash

        priorisea plugin sync

``debug``
    Print the plugin's current formula and user scores, which is useful to verify
    that the plugin is in sync with the database. Without ``--json`` the formula
    is printed (as a ``Name``/``Expression``/``Defaults`` table) followed by the
    scores as a ``UID``/``Score`` table; with ``--json`` they are printed as a
    single JSON object with the keys ``formula`` and ``scores`` (a mapping of
    user id to score).

    .. code-block:: bash

        priorisea plugin debug

    .. code-block:: text

        Name          Expression                                          Defaults
        job_priority  <var:prev_priority> * 0.5 + <var:user_score> * 0.5

        UID   Score
        1000  0.63
        1001  0.11

    With ``--json``:

    .. code-block:: json

        {
          "formula": {
            "name": "job_priority",
            "expr": "<var:prev_priority> * 0.5 + <var:user_score> * 0.5",
            "defaults": {}
          },
          "scores": {
            "1000": 0.63,
            "1001": 0.11
          }
        }

The plugin is loaded into Flux using:

.. code-block:: bash

    flux jobtap load sea_priority.so
