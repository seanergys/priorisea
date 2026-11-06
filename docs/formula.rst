Formula
=======

PrioriSea supports custom formulas when computing a priority score or a job score. At its core,
it uses `tinyexpr <https://github.com/codeplea/tinyexpr>`__ to parse and evaluate the formulas.

Mandatory Formulas
------------------

PrioriSea ships with three mandatory formulas. Each can be reset to its default
value using ``priorisea formula reset``.

- ``job_priority`` is the priority assigned to a new job. Its default expression
  is ``<var:prev_priority> * 0.5 + <var:user_score> * 0.5``.
- ``job_score`` is the score of a job once it has finished running. Its default
  expression is ``<var:duration_ratio> + <var:timewin_score>``. See
  :doc:`../job`.
- ``job_weight`` weights a job by its age when aggregating the score of its user.
  Its default expression is ``max(0, 1 - <var:age_days> / 90)``. See
  :doc:`../user`.

Administrators can interact with the formulas using the ``priorisea formula`` subcommand. Formulas
expect the result to return a double value. Additionally, formulas allow keywords to be used as
variables in the formula. These variables are automatically resolved at evaluation. The available variables
are:

+--------------+--------------------------+--------------------------------------------------------------------------+
| Formula      | Variable                 | Usage                                                                    |
+==============+==========================+==========================================================================+
| job_priority | var:prev_priority        | Previous priority assigned to this job. Null if the PrioriSea Flux       |
|              |                          | plugin is the first one in the pipeline.                                 |
+--------------+--------------------------+--------------------------------------------------------------------------+
| job_priority | var:user_score           | Aggregate of the scores of the jobs previously ran by the user. Null if  |
|              |                          | no job of this user has been assigned a score. See :doc:`../user`.       |
+--------------+--------------------------+--------------------------------------------------------------------------+
| job_score    | var:duration_ratio       | Ratio of the job announced duration to the final job                     |
|              |                          | duration. Null if the job ran too fast                                   |
+--------------+--------------------------+--------------------------------------------------------------------------+
| job_score    | var:timewin_score        | Sum of the time windows scores of this job. 0 if the job was not ran     |
|              |                          | during any time window. See :doc:`../timewin`.                           |
+--------------+--------------------------+--------------------------------------------------------------------------+
| job_weight   | var:age_days             | Age of the job, in days.                                                 |
+--------------+--------------------------+--------------------------------------------------------------------------+
| job_weight   | var:age_seconds          | Age of the job, in seconds.                                              |
+--------------+--------------------------+--------------------------------------------------------------------------+
| job_weight   | var:duration_seconds     | The duration of the job, in seconds.                                     |
+--------------+--------------------------+--------------------------------------------------------------------------+
| job_weight   | var:duration_hours       | The duration of the job, in hours.                                       |
+--------------+--------------------------+--------------------------------------------------------------------------+
| job_weight   | var:max_duration_seconds | The user's longest job duration in the window, in seconds.               |
+--------------+--------------------------+--------------------------------------------------------------------------+
| job_weight   | var:max_duration_hours   | The user's longest job duration in the window, in hours.                 |
+--------------+--------------------------+--------------------------------------------------------------------------+
| job_score    | rpc:{{SB_UUID}}          | Sends an HelloWorldReqV1 containing the job id to {{SB_UUID}} through    |
|              |                          | the SeanerBUS. The remote SeanerBUS client is expected to return an      |
|              |                          | HelloWorldResV1 containing a double. Null if any error arise (SeanerBUS  |
|              |                          | connection failed, {{SB_UUID}} invalid, {{SB_UUID}} does not exist or    |
|              |                          | answer a wrong type, ...).                                               |
+--------------+--------------------------+--------------------------------------------------------------------------+
| job_score    | rpc_ns:{{SB_NAME}}       | Sends an HelloWorldReqV1 containing the job id to {{SB_NAME}} [#]_ after |
|              |                          | resolving {{SB_NAME}} through the SeanerBUS naming service. The remote   |
|              |                          | SeanerBUS client is expected to return an HelloWorldResV1 containing a   |
|              |                          | double. Null if any error arise (SeanerBUS connection failed,            |
|              |                          | {{SB_NAME}} is invalid, {{SB_NAME}} does not answer or answer a wrong    |
|              |                          | type, ...).                                                              |
+--------------+--------------------------+--------------------------------------------------------------------------+

When writing a formula, variables should be enclosed in angled brackets. For example, a valid
job score formula can be: ``<var:timewin_score> + <rpc_ns:{"service": "nrg_oracle"}>``.

When a variable can become null and administrators want to handle it gracefully, they can use
the ``--default`` keyword when defining formulas. This default value will be used when the
variable is not defined. For example, a job score formula can be:
``priorisea formula update job_score "<var:timewin_score>" --default var:timewin_score 0``.

Commands
--------

Administrators interact with the formulas through the ``priorisea formula``
subcommand. A formula must evaluate to a ``double``.

.. code-block:: bash

    priorisea formula get NAME [--json]
    priorisea formula list [NAME...] [--json]
    priorisea formula update NAME -e EXPR [-d NAME VALUE]...
    priorisea formula reset NAME...

``get`` NAME
    Print the formula named ``NAME``. ``--json`` outputs the formula as a JSON
    object with the keys ``name``, ``expr`` and ``defaults``.

    .. code-block:: bash

        priorisea formula get job_priority --json

    .. code-block:: json

        {
          "name": "job_priority",
          "expr": "<var:prev_priority> * 0.5 + <var:user_score> * 0.5",
          "defaults": {}
        }

    Without ``--json`` the formula is printed as a table:

    .. code-block:: text

        Name          Expression                                          Defaults
        job_priority  <var:prev_priority> * 0.5 + <var:user_score> * 0.5

``list`` [NAME...]
    List the configured formulas. When one or more names are given, only those
    are printed. ``--json`` outputs a JSON array (one object per formula, with
    the same keys as ``get``).

    .. code-block:: bash

        priorisea formula list

    .. code-block:: text

        Name          Expression                                          Defaults
        job_priority  <var:prev_priority> * 0.5 + <var:user_score> * 0.5
        job_score     <var:duration_ratio> + <var:timewin_score>
        job_weight    max(0, 1 - <var:age_days> / 90)

``update`` NAME
    Update the formula named ``NAME``. ``-e, --expr`` sets the expression to
    evaluate and ``-d, --default`` (given as repeated ``NAME VALUE`` pairs)
    defines the value used when a variable becomes null.

    .. code-block:: bash

        priorisea formula update job_priority "<var:prev_priority> * 2" --default var:prev_priority 5

    The command produces no output on success. Synchronise the plugin afterwards
    (see :doc:`../plugin`) for the change to take effect.

``reset`` NAME...
    Reset the named formula(s) to their default values. This command produces no
    output. Synchronise the plugin afterwards (see :doc:`../plugin`) for the
    change to take effect.

.. [#] {{SB_NAME}} must be a valid `SeanerBUS name <https://software.pages.seanergys.fz-juelich.de/seanerbus/addressing.html>`__ such as ``{"my-key-1": "my-value-1", "my-key-2": "my-value-2"}``.
