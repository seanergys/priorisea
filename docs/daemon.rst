Daemon
======

The PrioriSea daemon is the main component of the PrioriSea software. It fulfills three roles:

- **Express the business logic.** Formulas (:doc:`formula`), users score
  (:doc:`user`) and time windows (:doc:`timewin`) are all defined through
  the Daemon.
- **Store and read state.** Every change is persisted in the database (that should first be created)
  and every value can be read back for inspection.
- **Synchronize the plugin.** The daemon can send data to the priority plugin when a certain
  message is sent via the data-plane. This help keeping the plugin and the database consistent
  (see :doc:`architecture`).

The PrioriSea daemon and CLI have a 1-to-1 endpoint mapping (i.e. for every possible RPC call to the
daemon there exists a corresponding CLI command). The daemon commands are documented in the
:doc:`CLI reference <cli-reference>`.
