#!/usr/bin/env bash

set -euo pipefail

COMMAND=${1:-pwd}
JOBID=$(flux submit $COMMAND)

flux job wait-event $JOBID clean >/dev/null

echo $JOBID
