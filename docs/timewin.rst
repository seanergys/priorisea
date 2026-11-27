Time Window
===========

Time windows are recurring time frames that can influence a job score. Administrators can manage these
time windows using the ``priorisea timewin`` subcommand. Time windows are defined by:

- a unique name
- a cron expression
- a duration in seconds
- a score value [#]_

As such, each time the cron expression is triggered, the timeframe last ``duration`` seconds. If a job
is submitted to run during this timeframe, the job gets influenced by the time window score.
The sum of the time window scores can then be used to influence the job score. See :doc:`formula`.

For example, if an administrator knows that jobs ran between 12am. and 1pm. produce less
carbon footprint, they can define a time window to grant 100 points to jobs submitted during
that time:
``priorisea timewin create carbon -c "0 0 12 * * * *" -d 3600 -s 100.0``.

At its core, PrioriSea uses `croncpp <https://github.com/mariusbancila/croncpp>`__ to parse the cron expression.
As such, a six or seven fields expression is expected.

.. [#] The score of time windows can be either a positive or negative value.
