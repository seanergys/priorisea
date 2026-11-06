User Score
==========

To reflect the historical performance of a user, a user score can be computed from the
history of their jobs. The resulting user score can then be used as a variable in the
job priority formula. See :doc:`../formula`.

The user scores can be updated using ``priorisea user update`` and retrieved using
``priorisea user list`` or ``priorisea user get USERNAME``. The computation only
considers jobs that are more recent than the user period (the maximum age of jobs
considered, configured on the :ref:`daemon <daemon-update-user-period>` with ``priorisea daemon update-user-period``) and weights each
job by its age using the ``job_weight`` formula.

Aggregation
-----------

The user score is a weighted average of the scores of the jobs the user ran. Each job
score is weighted by the ``job_weight`` formula, evaluated with the age of the job
(``var:age_days`` and ``var:age_seconds``) and its duration relative to the user's
longest job in the window (``var:duration_seconds``/``var:duration_hours`` and
``var:max_duration``/``var:max_duration_hours``). With the default ``job_weight`` formula
``max(0, 1 - <var:age_days> / 90)``, a job keeps its full weight when recent and loses
weight linearly as it ages, contributing nothing once it is 90 days old.

Formally, for a given user, the score is:

    sum(job_score * weight) / sum(weight)

taken over all of the user's jobs within the user period, where ``weight`` is the value
of the ``job_weight`` formula for that job.

Limitations
-----------

Because the user score is a weighted average of job scores, it is bounded by the maximum
job score (assuming non-negative weights). As such, even if a single job has a very high
score, the resulting user score cannot exceed the maximum job score.

Additionally, changing the ``job_weight`` formula does not affect the user score of jobs
that have already been computed. Their user score will be recomputed the next time the
``priorisea user update`` command is run and the user ran a job that got updated.

Commands
--------

User scores are managed through the ``priorisea user`` subcommand. Usernames are
derived from the user id (``userid``); the JSON output always uses the numeric
``userid``.

.. code-block:: bash

    priorisea user get USERNAME [-d] [--json]
    priorisea user list [USERNAME...] [-o ORDER] [--json]
    priorisea user update [USERNAME...] [-a]

``get`` USERNAME
    Print the user ``USERNAME``. ``-d, --detail`` shows additional details: the
    ``Formula`` (the ``job_weight`` formula used to aggregate the user's jobs),
    its ``Formula defaults``, ``t_executed`` (timestamp of the last executed
    job) and ``t_inactive_min`` (the user's shortest inactivity time), followed
    by the table of the user's jobs. Without ``-d`` only the username, user id
    and score are shown. Users without a score are shown with a ``/`` in the
    score column. ``--json`` outputs the user (and, with ``-d``, its jobs) as a
    JSON object.

    .. code-block:: bash

        priorisea user get alice -d

    .. code-block:: text

        Username  User ID  Score  Formula                         Formula defaults  t_executed  t_inactive_min
        alice     1000     0.63   max(0, 1 - <var:age_days> / 90)                   1700000000  42.5

    With ``--json`` the user is printed as:

    .. code-block:: json

        {
          "user": {
            "userid": 1000,
            "score": 0.63,
            "w_expr": "max(0, 1 - <var:age_days> / 90)",
            "w_defaults": {},
            "t_executed": 1700000000.0,
            "t_inactive_min": 42.5
          },
          "jobs": [
            {
              "jobid": "72057594037936128",
              "userid": 1000,
              "t_inactive": 42.5,
              "score": 0.75,
              "duration": 120.0,
              "s_expr": "<var:duration_ratio> + <var:timewin_score>",
              "s_defaults": {},
              "s_values": {},
              "weight": 0.9,
              "w_values": {}
            }
          ]
        }

``list`` [USERNAME...]
    List users. When one or more usernames are given, only those are printed.
    ``-o, --order`` sets the ordering (``asc`` or ``desc``, default ``desc``).
    Users without a score are shown with a ``/`` in the score column. ``--json``
    outputs a JSON array (each entry uses the same keys as ``get``'s ``user``
    object).

    .. code-block:: bash

        priorisea user list

    .. code-block:: text

        Username  User ID  Score
        alice     1000     0.63
        bob       1001     /

``update`` [USERNAME...]
    Recompute user scores. Without arguments only the explicitly listed users
    are updated; with ``-a, --all`` every user is updated. This command produces
    no output. Synchronise the plugin afterwards (see :doc:`../plugin`) for the
    change to take effect.

    .. code-block:: bash

        priorisea user update --all
