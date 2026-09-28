.. SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
..
.. SPDX-License-Identifier: MIT

Configuration
=============

The CLI and the Flux plugin can be configured using several
methods. These methods are, by increasing order of precedence:

- Built-in default values
- Configuration file
- Environment variables
- Flux system configuration (Flux plugin only)
- RPC call (Flux plugin only)

The configuration file is a JSON file. The CLI and the Flux plugin
both search for the configuration file in ``$XDG_CONFIG_HOME/priorisea/config.json``
or ``$HOME/.config/priorisea/config.json`` as a fallback.

In the case of the Flux plugin, the Flux system config method has a higher precedence than
the other methods. The ``priorisea`` section is used in this case:

.. code-block:: toml

    [priorisea]
    ...

SeanerBUS
---------

The SeanerBUS endpoint that will be used by the CLI and the
Flux plugin can be configured. By default, the endpoint is
``demobus://localhost:5398``. To change the endpoint, set the
``SEANERBUS_URL`` environment variable (``export SEANERBUS_URL=demobus://localhost:1234``) or update the
configuration file:

.. code-block:: json

   {
     "seanerbus": "demobus://localhost:1234"
   }

For the Flux plugin, the configuration can also be set via the Flux system config:

.. code-block:: toml

    [priorisea]
    seanerbus = "demobus://localhost:1234"

or be overridden using the following CLI command: ``priorisea plugin seanerbus set "demobus://localhost:1234"``.
Changing the configuration this way is only stored in-memory and will be lost when the
Flux plugin is restarted.
