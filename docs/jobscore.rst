
Job Score
=========

To reflect the performance of a job, a job score can be computed post-run. A custom formula
is used to compute the job score. See :doc:`../formula`.

The job scores can be computed/updated using ``priorisea score update`` [#]_. The ``--new``
flag can be passed to the update command. Internally, PrioriSea keeps the date since the last
``priorisea score update --new`` command. This date is used to update the score of jobs who finished
running since the last time the ``--new`` flag was used.

Limitations
-----------

Since computing a job score can be computationally expensive, the job score is not computed
immediately after a job finishes running. Instead, it should be updated periodically using the
``priorisea score update`` command. Moreover, updating the job score formula does not affect
the job score of jobs that have already been computed. To update them again, the list of job ids
to update can be passed directly to the ``priorisea score update`` command.

.. [#] Any user who ran a job will have their score updated. See :doc:`../userscore`
