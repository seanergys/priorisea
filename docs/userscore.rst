
User Score
==========

To reflect the historical performance of a user, a user score can be computed from the
history of their jobs. PrioriSea uses decay frames to perform this aggregation. The resulting
user score can then be used in the job priority formula. See :doc:`../formula`.

The users scores can be updated using ``priorisea score update``. Note that this command
first updates the score of jobs, then updated the score of users who ran those jobs.

The user scores can be retrieved using ``priorisea score user``.

Decay
-----

The decay expression represents how a job score should influence the final user score depending
on the time since the user ran that job. A decay expression is made of a number of decay
frames. Each decay frame is composed of a duration in seconds and a weight. Each job ran
by a user has a score assigned, and that score is multiplied by the weight of the
corresponding decay frame. For example, if a job was ran 2 days ago, and the decay expression
has two decay frames:

- A decay frame with a duration of 1 day and a weight of 0.5
- A decay frame with a duration of 2 days and a weight of 0.25

Then, the decayed job score would be multiplied by 0.25 as it was run after the first frame ended (lasting
one day) and before the second frame ended (lasting two days, up to day three).

In each time frame, PrioriSea computes the average decayed job score for this period.
The final user score is then computed as the sum of all decayed job scores over all the decay periods.

Decay expression can be customised through the CLI. For the previous example, the command to set
the corresponding decay would be: ``priorisea decay set 86400:0.5,172800:0.25``

Limitations
-----------

The user score system does not ponder the job scores by their durations. As such, a 1-second
job has the same weight as a 10-minute job on the evaluation of the user score. Moreover, the
resulting value is difficult to interpret as it is a sum of averages. Thus, even if job scores
were to be limited to a maximum value, the resulting user score can still be greater than that value.

Additionally, changing the decay expression does not affect the user score of jobs that have already
been computed. Their user score will properly be recomputed when the ``priorisea score update``
command is run and if the user ran a job that got updated.
