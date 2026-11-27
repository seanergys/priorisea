User Score
==========

To reflect the historical performance of a user, a user score can be computed from the
history of their jobs. The resulting user score can then be used as a variable in the
job priority formula. See :doc:`formula`.

The user scores can be updated using ``priorisea user update`` and retrieved using
``priorisea user list`` or ``priorisea user get USERNAME``. The computation only
considers jobs that are more recent than the user period (the maximum age of jobs
considered, configured on the :ref:`daemon <daemon-update-user-period>` with ``priorisea daemon update-user-period``) and weights each
job by its age using the ``job_weight`` formula.

Aggregation
-----------

The user score is a weighted average of the scores of the jobs the user ran. Each job
score is weighted by the ``job_weight`` formula, evaluated with the age of the job
and its duration relative to the user's longest job in the window.
With the default ``job_weight`` formula, a job keeps its full weight when recent and loses
weight linearly as it ages, contributing nothing once it is 90 days old. See :doc:`formula`
for the value.

Formally, for a given user, the score is:

    sum(job_score * weight) / sum(weight)

taken over all of the user's jobs within the user period, where ``weight`` is the value
of the ``job_weight`` formula for that job.

Limitations
-----------

Since the ``job_weight`` value of a job can vary depending on the time when the user score is updated,
all jobs in the user period are re-evaluated when the user score is updated. If the user period is large with many
jobs ran during this period, this can be computationally expensive.

Additionally, changing the ``job_weight`` formula does not affect the user scores that have
already been computed. Their score will be recomputed the next time the
``priorisea user update`` command is run.
