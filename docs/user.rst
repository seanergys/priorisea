User Score
==========

To reflect the historical performance of a user, a user score can be computed from the
history of their jobs. The resulting user score can then be used as a variable in the
job priority formula. See :doc:`../formula`.

The user scores can be updated using ``priorisea user update`` and retrieved using
``priorisea user list`` or ``priorisea user get USERNAME``. The computation only
considers jobs that are more recent than the user period (the maximum age of jobs
considered, configured with ``priorisea daemon update-user-period``) and weights each
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
