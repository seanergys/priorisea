Glossary
========

.. glossary::

   PrioriSea
      A priority manager for Flux. It integrates at the end of the Flux priority
      assignment pipeline and lets administrators define custom priority policies
      for jobs through formulas, time windows and job/user scoring.

   jobtap
      The Flux job-manager tap mechanism. PrioriSea ships a :term:`plugin`
      implemented as a jobtap priority plugin (``sea_priority.so``) that is
      queried for the priority to assign to each newly scheduled job.

   plugin
      The PrioriSea Flux jobtap priority plugin (``sea_priority.so``). It runs
      inside the Flux instance and is kept consistent with the database using
      :term:`sync`. See :doc:`plugin`.

   daemon
      The core PrioriSea component. It expresses the business logic (formulas,
      user scores, time windows), persists and reads state in the database, and
      synchronises the plugin. See :doc:`daemon`.

   CLI
      The ``priorisea`` command-line interface. See :doc:`cli-reference`.

   formula
      A custom expression (parsed and evaluated with
      `tinyexpr <https://github.com/codeplea/tinyexpr>`__) that computes a
      priority or score. See :doc:`formula`.

   job_priority
      The priority assigned to a new job.

   job_score
      The score of a job once it has finished running, computed post-run.

   job_weight
      Weights a job by its age when aggregating the score of its user.

   job score
      A job's performance score, computed post-run with ``priorisea job update``.

   user score
      A weighted average of the scores of the jobs a user ran, weighted by the
      :term:`job_weight` formula. It is used as an input to the
      :term:`job_priority` formula. See :doc:`user`.

   time window
      A recurring time frame, defined by a cron expression, a duration and a
      score, that influences the score of jobs submitted to run during it.
      See :doc:`timewin`.

   SeanerBUS
      The external communication bus PrioriSea integrates with inside formulas,
      to retrieve meaningful information about jobs from remote SEANERGYS
      components.

   RPC endpoint
      A mechanism for a formula to call a remote SeanerBUS service and use the
      returned value. Two formula variables invoke it: ``rpc:{{SB_UUID}}`` targets
      a service by UUID and ``rpc_ns:{{SB_NAME}}`` targets it by name (resolved
      through the SeanerBUS naming service). The remote client is expected to
      return a double; the variable is null on any error.

   t_inactive
      The inactivity threshold stored in the database info and reported by
      ``priorisea daemon debug``.

   user_period
      The maximum age (in seconds) of jobs considered when computing
      :term:`user score`. It is set when creating the database (default 90 days,
      i.e. ``7776000`` seconds) and changed with
      ``priorisea daemon update-user-period``.

   prev_priority
      The previous priority assigned to a job, available in formulas as
      ``var:prev_priority``. Equal to the `fair-share` priority if
      `flux-accounting <https://github.com/flux-framework/flux-accounting>`__)
      is installed and loaded before PrioriSea

   sync
      Short for ``priorisea plugin sync``. It propagates the current database
      state (formulas, time windows, scores) to the running plugin and should be
      run after every change to the formulas, the scores or the time windows.
