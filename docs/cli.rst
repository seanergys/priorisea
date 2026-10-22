Command Line Interface
======================

The ``priorisea`` command-line interface (CLI) is the single administrative
entry point of PrioriSea. It fulfills three roles:

- **Express the business logic.** Formulas (:doc:`formula`), decay expressions
  (:doc:`userscore`) and time windows (:doc:`timewin`) are all defined through
  the CLI.
- **Store and read state.** Every change is persisted in the database (backed by
  the Flux KVS), and every value can be read back for inspection.
- **Synchronize the plugin.** Whenever a value is created or updated, the change
  is also pushed to the running Flux plugin through the Flux dataplane so the
  plugin and the database stay consistent (see :doc:`architecture`).

The following command tree is available:

- :ref:`init <cli-init>`
- :ref:`config <cli-config>`
- :ref:`formula <cli-formula>`
- :ref:`decay <cli-decay>`
- :ref:`score <cli-score>`
- :ref:`timewin <cli-timewin>`
- :ref:`plugin <cli-plugin>`

Global options
--------------

The following options are available on the top-level ``priorisea`` command,
before any subcommand:

``--config PATH``
    Path to the configuration file to use. By default the CLI searches
    ``$XDG_CONFIG_HOME/priorisea/config.json`` (see :doc:`configuration`).
``-h, --help``
    Show help for ``priorisea`` or for a given subcommand.

.. _cli-init:

init
----

.. code-block:: bash

    priorisea init

Initialize the database. This runs the initialization of the decay parameters,
the mandatory formulas and the score database in a single call. It is the
recommended first step after installing PrioriSea (see :doc:`getstarted`).

Each domain also exposes its own ``init`` subcommand (for example
``priorisea formula init``), which can be used to initialize a single part.

.. _cli-config:

config
------

Manage the configuration (see :doc:`configuration`).

.. code-block:: bash

    priorisea config get [--json]
    priorisea config export PATH

``get``
    Print the current configuration. Without ``--json``, only the SeanerBUS URL
    is displayed as a table; with ``--json`` the full configuration is printed.
``export`` *PATH*
    Write the current configuration as JSON to *PATH*.

.. _cli-formula:

formula
-------

Manage the scoring formulas (see :doc:`formula`).

.. code-block:: bash

    priorisea formula init
    priorisea formula list [--json]
    priorisea formula get FORMULA... [--json]
    priorisea formula set FORMULA EXPR [-d NAME VALUE]... [--json]
    priorisea formula reset FORMULA...

``init``
    Initialize the mandatory formulas to their default values.
``list``
    List every configured formula.
``get`` *FORMULA...*
    Get the formula(s) named on the command line. At least one name is required.
``set`` *FORMULA* *EXPR*
    Set a formula. *FORMULA* is the formula name and *EXPR* the expression to
    evaluate. Use ``-d, --default`` (given as repeated ``NAME VALUE`` pairs) to
    define the default value used when a variable becomes null.

    Alternatively, pass the whole formula as a single JSON object with
    ``--json`` (``{"name": ..., "expr": ..., "defaults": {...}}``).

    .. code-block:: bash

        priorisea formula set job_priority "<var:prev_priority> * 2" --default var:prev_priority 5
        priorisea formula set '{"name":"job_priority","expr":"<var:x> * 2","defaults":{"var:x":5}}' --json
``reset`` *FORMULA...*
    Reset the named formula(s) to their default values.

.. _cli-decay:

decay
-----

Manage the decay parameters used to aggregate user scores (see :doc:`userscore`).

.. code-block:: bash

    priorisea decay init
    priorisea decay get [--json]
    priorisea decay set DECAY [--json]
    priorisea decay reset

``init``
    Initialize the decay parameters to their default value.
``get``
    Print the current decay frames.
``set`` *DECAY*
    Set the decay expression. By default *DECAY* is given in the compact string
    form ``duration:factor,...`` (for example ``3600:1,7200:0.5``). With
    ``--json``, a JSON array of ``{"duration": ..., "weight": ...}`` objects is
    accepted instead.
``reset``
    Reset the decay expression to its default value.

.. _cli-score:

score
-----

Manage the job and user scores (see :doc:`jobscore` and :doc:`userscore`).

.. code-block:: bash

    priorisea score init
    priorisea score update [--new] [JOBID...]
    priorisea score job [--json] [--all] [--limit N] [--id-format FORMAT] [JOBID...]
    priorisea score user [--json] [--all] [USERID...]

``init``
    Initialize the score database.
``update``
    Recompute the job scores, then the user scores of the users who ran those
    jobs. With ``--new``, all jobs that became inactive since the last
    ``--new`` update are considered; otherwise only the explicitly listed
    *JOBID* values are updated.
``job``
    Read job scores. Without any option, the scores of the given *JOBID* values
    are printed. ``--all`` lists every stored job, while ``--limit N`` returns
    the ``N`` most recent jobs ranked by inactivity time. ``--id-format``
    controls how job ids are printed (``dec`` (default), ``hex``, ``kvs``,
    ``dothex``, ``words``, ``f58``, ``f58plain``). ``--json`` outputs a
    ``{jobid: score}`` mapping.
``user``
    Read user scores. Without any option, the scores of the given *USERID*
    values are printed; ``--all`` lists every stored user. ``--json`` outputs a
    ``{userid: score}`` mapping.

.. _cli-timewin:

timewin
-------

Manage the recurring time windows that influence the job score (see
:doc:`timewin`).

.. code-block:: bash

    priorisea timewin list [--json]
    priorisea timewin get NAMES... [-j|--json]
    priorisea timewin add NAME [-s SCORE] [-c CRON] [-d DURATION] [--json]
    priorisea timewin edit NAME [-s SCORE] [-c CRON] [-d DURATION] [--json]
    priorisea timewin delete NAMES...

``list``
    List every configured time window.
``get`` *NAMES...*
    Get the time window(s) named on the command line.
``add`` *NAME*
    Add a time window. ``-s, --score`` sets the score, ``-c, --cron`` the cron
    expression and ``-d, --duration`` the duration in seconds.

    Alternatively, pass the whole time window as a single JSON object with
    ``--json`` (``{"name": ..., "cron": ..., "duration": ..., "score": ...}``).

    .. code-block:: bash

        priorisea timewin add carbon -c "0 0 12 * * *" -d 3600 -s 100.0
        priorisea timewin add '{"name":"monday","cron":"0 0 * * 1","duration":86400,"score":10}' --json
``edit`` *NAME*
    Edit an existing time window. Any of ``-s, --score``, ``-c, --cron`` and
    ``-d, --duration`` that is provided overrides the stored value. The JSON
    form (``--json``) replaces the whole time window.
``delete`` *NAMES...*
    Delete the named time window(s).

.. _cli-plugin:

plugin
------

Manage the running Flux plugin.

.. code-block:: bash

    priorisea plugin seanerbus get [--json]
    priorisea plugin seanerbus set URL

``seanerbus get``
    Print the SeanerBUS connection info currently used by the plugin.
``seanerbus set`` *URL*
    Set the SeanerBUS endpoint used by the plugin and propagate it through the
    dataplane. See :doc:`configuration` for the alternative configuration
    methods.
