Deployment
==========

PrioriSea requires a running Flux instance, as it uses the Flux data-plane to
handle communication between the daemon and the CLI.

1. Create the database:

   .. code-block:: bash

       priorisea database create /path/to/database.db

2. Start the daemon, pointing at the same database file:

   .. code-block:: bash

       priorisea daemon start /path/to/database.db --background

3. Load the job-manager plugin (under ``{INSTALL_PATH}/lib/flux/job-manager/plugins``):

   .. code-block:: bash

       flux jobtap load /path/to/sea_priority.so

4. Synchronise the plugin with the database:

   .. code-block:: bash

       priorisea plugin sync

5. Verify:

   .. code-block:: bash

       priorisea formula list
       priorisea plugin debug

Re-run ``priorisea plugin sync`` after every change to the formulas, the scores
or the time windows.

What next
---------

- Follow the :doc:`tutorial` for a guided, end-to-end walkthrough of configuring
  your first policy.
- Try the :doc:`howto-carbon-aware` guide.
- Look up a command in the :doc:`cli-reference`.
