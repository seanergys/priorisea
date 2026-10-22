# Sphinx configuration for PrioriSea.
# The C++ API is pulled in from Doxygen's XML output via Breathe

project = "PrioriSea"
author = "SEANERGYS Partners"
copyright = "2026 SEANERGYS Partners"

extensions = ["breathe"]

breathe_projects = {"priorisea": "doxygen/xml"}
breathe_default_project = "priorisea"
breathe_domain_by_extension = {"hpp": "cpp", "h": "cpp"}

# Hide the "#include <file>" line Breathe shows on each compound; for types
# declared in their own header it is redundant.
breathe_show_include = False

html_theme = "sphinx_rtd_theme"
