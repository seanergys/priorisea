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

``plugin sync`` propagates the current database state (formulas, time windows, scores)
to the running plugin. It should be run after every change to the formulas or the
scores. ``plugin debug`` prints the formula and the user scores currently used by the
plugin, which is useful to verify that the plugin is in sync with the database.

The plugin is loaded into Flux using:

.. code-block:: bash

    flux jobtap load sea_priority.so
