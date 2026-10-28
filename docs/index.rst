PrioriSea
=========

PrioriSea is a priority manager for Flux. It integrates at the
end of the Flux priority assignment pipeline and allows administrators
to define custom priority policies for jobs.

At the Flux plugin level, the PrioriSea plugin computes a priority to new
scheduled jobs using a custom formula. This formula can be configured by
administrators and can include defined keywords to control the policy, such
as the previous priority assigned in the pipeline (used to integrate with Fairsharing
mechanisms), the user who submitted the job, ...

PrioriSea integrates with SeanerBUS inside formulas to enable communication with external
SEANERGYS components to retrieve meaningful information about jobs.

.. toctree::
   :maxdepth: 2
   :caption: Contents

   architecture
   getstarted
   daemon
   cli
   database
   plugin
   formula
   job
   user
   timewin
