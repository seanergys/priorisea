Formula
=======

PrioriSea supports custom formulas when computing a priority score or a job score. At its core,
it uses `tinyexpr <https://github.com/codeplea/tinyexpr>`__ to parse and evaluate the formulas.

Administrators can interact with the formulas using the ``priorisea formula`` subcommand. Formulas
expect the result to return a double value. Additionally, formulas allow keywords to be used as
variables in the formula. These variables are automatically resolved at evaluation. The available variables
are:

+--------------+--------------------+--------------------------------------------------------------------------+
| Formula      | Variable           | Usage                                                                    |
+==============+====================+==========================================================================+
| job_priority | var:prev_priority  | Previous priority assigned to this job. Null if the PrioriSea Flux       |
|              |                    | plugin is the first one in the pipeline.                                 |
+--------------+--------------------+--------------------------------------------------------------------------+
| job_priority | var:user_score     | Aggregate of the scores of the jobs previously ran by the user. Null if  |
|              |                    | no job of this user has been assigned a score. See :doc:`../userscore`.  |
+--------------+--------------------+--------------------------------------------------------------------------+
| job_score    | var:duration_ratio | Ratio of the job announced duration to the final job                     |
|              |                    | duration. Null if the job ran too fast                                   |
+--------------+--------------------+--------------------------------------------------------------------------+
| job_score    | var:timewin_score  | Sum of the time windows scores of this job. 0 if the job was not ran     |
|              |                    | during any time window. See :doc:`../timewin`.                           |
+--------------+--------------------+--------------------------------------------------------------------------+
| job_score    | rpc:{{SB_UUID}}    | Sends an HelloWorldReqV1 containing the job id to {{SB_UUID}} through    |
|              |                    | the SeanerBUS. The remote SeanerBUS client is expected to return an      |
|              |                    | HelloWorldResV1 containing a double. Null if any error arise (SeanerBUS  |
|              |                    | connection failed, {{SB_UUID}} invalid, {{SB_UUID}} does not exist or    |
|              |                    | answer a wrong type, ...).                                               |
+--------------+--------------------+--------------------------------------------------------------------------+
| job_score    | rpc_ns:{{SB_NAME}} | Sends an HelloWorldReqV1 containing the job id to {{SB_NAME}} [#]_ after |
|              |                    | resolving {{SB_NAME}} through the SeanerBUS naming service. The remote   |
|              |                    | SeanerBUS client is expected to return an HelloWorldResV1 containing a   |
|              |                    | double. Null if any error arise (SeanerBUS connection failed,            |
|              |                    | {{SB_NAME}} is invalid, {{SB_NAME}} does not answer or answer a wrong    |
|              |                    | type, ...).                                                              |
+--------------+--------------------+--------------------------------------------------------------------------+

When writing a formula, variables should be enclosed in angled brackets. For example, a valid
job score formula can be: ``<var:timewin_score> + <rpc_ns:{"service": "nrg_oracle"}>``.

When a variable can become null and administrators want to handle it gracefully, they can use
the ``--default`` keyword when defining formulas. This default value will be used when the
variable is not defined. For example, a job score formula can be:
``priorisea formula set job_score "<var:timewin_score>" --default var:timewin_score 0``.

.. [#] {{SB_NAME}} must be a valid `SeanerBUS name <https://software.pages.seanergys.fz-juelich.de/seanerbus/addressing.html>`__ such as ``{"my-key-1": "my-value-1", "my-key-2": "my-value-2"}``.
