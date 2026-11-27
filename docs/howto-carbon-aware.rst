How-to: Configure a carbon-aware priority policy
================================================

This guide shows how to reward jobs that run during a low-carbon time frame, and
optionally combine that with an external score returned by a SeanerBUS service.
It assumes the daemon is running and the plugin is loaded and synced (see
:doc:`deployment`).

1. Define the low-carbon time window
------------------------------------

Create a recurring time window for the window in which jobs produce less carbon.
For example, jobs running between 12:00 and 13:00:

.. code-block:: bash

    priorisea timewin create carbon -c "0 0 12 * * * *" -d 3600 -s 100.0

The cron expression is parsed by `croncpp
<https://github.com/mariusbancila/croncpp>`__, so a six or seven field expression
is expected. Here the window fires at noon every day and lasts 3600 seconds
(1 hour), granting 100 points to jobs submitted during it.

2. Add the time-window score to the job score formula
-----------------------------------------------------

The default ``job_score`` formula already includes ``<var:timewin_score>``:

.. code-block:: bash

    priorisea formula get job_score

If it does not, set it so jobs running in a time window gain its score:

.. code-block:: bash

    priorisea formula update job_score "<var:duration_ratio> + <var:timewin_score>"

3. (Optional) Pull an external score over SeanerBUS
---------------------------------------------------

Add an ``rpc_ns`` variable to the ``job_score`` formula to fold in a score
returned by a remote SeanerBUS service. For example, to query an oracle service:

.. code-block:: bash

    priorisea formula update job_score "<var:duration_ratio> + <var:timewin_score> + <rpc_ns:{\"service\": \"nrg_oracle\"}>" \
        --default "rpc_ns:{\"service\": \"nrg_oracle\"}" 0

The ``--default`` makes the term resolve to ``0`` when the RPC variable is null
(the service is unreachable, the name is invalid, it does not answer, or answers
a wrong type). See :doc:`formula` for the ``rpc:``/``rpc_ns:`` variables and the
``--default`` keyword.

4. Sync and verify
------------------

The formula change lives in the database until it reaches the plugin, so
synchronise and verify:

.. code-block:: bash

    priorisea plugin sync
    priorisea plugin debug

New jobs submitted during the window now score the time-window points (plus any
external score), and therefore receive a higher priority.

See also
--------

- :doc:`timewin` — time window reference.
- :doc:`formula` — formula syntax, variables and defaults.
- :doc:`job` — how job scores are computed and updated.
- :doc:`plugin` — synchronising the plugin.
