Tutorial: Configure your first policy
=====================================

This tutorial walks you through configuring a working priority policy from a
fresh install.

Prerequisites
-------------

- A running Flux instance.
- PrioriSea is installed (see :doc:`install`).

This walkthrough assumes the daemon is running and the plugin is loaded and
synced. If that is not yet the case, follow :doc:`deployment` first.

1. Inspect the default policy
-----------------------------

PrioriSea ships with three mandatory formulas. List them:

.. code-block:: bash

    priorisea formula list

The ``job_priority`` formula is the priority assigned to a new job. Inspect it:

.. code-block:: bash

    priorisea formula get job_priority

By default it is ``<var:prev_priority> * 0.5 + <var:user_score> * 0.5``: a new
job's priority is an even mix of its previous priority and the score of the user
who submitted it. Verify what the running plugin sees:

.. code-block:: bash

    priorisea plugin debug

2. Recompute scores from job history
------------------------------------

As jobs run in Flux and finish, their scores can be computed from their history.
Recompute the scores of every job that finished since the last ``--new`` update,
then aggregate the users' scores:

.. code-block:: bash

    priorisea job update --new
    priorisea user update --all

Inspect the results:

.. code-block:: bash

    priorisea job list -n 3
    priorisea user list

Users with no score yet are shown with a ``/`` in the score column.

3. Change the policy
--------------------

Give returning users more influence by reweighting the ``job_priority`` formula:

.. code-block:: bash

    priorisea formula update job_priority "<var:prev_priority> * 0.2 + <var:user_score> * 0.8"

The change lives in the database until it is transmitted to the plugin, so
synchronise again:

.. code-block:: bash

    priorisea plugin sync

Submit new jobs and re-check: users with a strong history now contribute more of
the new job's priority. The user score itself is a weighted average of the
user's past job scores (see :doc:`user`).

Where to go next
----------------

- :doc:`formula` — the formula syntax and every available variable.
- :doc:`job` and :doc:`user` — how job and user scores are computed.
- :doc:`timewin` — reward jobs running in a recurring time window.
- :doc:`howto-carbon-aware` — a task-oriented guide that ties time windows and
  an external SeanerBUS service into the score.
