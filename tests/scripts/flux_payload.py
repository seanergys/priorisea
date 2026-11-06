#!/usr/bin/env python3

import argparse
import json
import sys

import flux
from _flux._core import ffi
from flux.constants import FLUX_ROLE_USER

parser = argparse.ArgumentParser(
    description="send a JSON payload to a Flux RPC endpoint"
)
parser.add_argument("endpoint", help="RPC endpoint string")
parser.add_argument("data", help="JSON data file to send as the payload")
parser.add_argument(
    "--userid",
    type=int,
    default=None,
    help="simulate this userid (owner/test instances only)",
)
args = parser.parse_args()

h = flux.Flux()

# Make the RPC look like it comes from another user
if args.userid is not None:
    uid = ffi.new("uint32_t[1]", [args.userid])
    role = ffi.new("uint32_t[1]", [FLUX_ROLE_USER])
    h.flux_opt_set("flux::testing_userid", uid, ffi.sizeof("uint32_t"))
    h.flux_opt_set("flux::testing_rolemask", role, ffi.sizeof("uint32_t"))

with open(args.data) as data_file:
    payload = json.load(data_file)

# Send the payload and print the response as JSON on stdout so the result can
# be compared against the expected value. An error response makes the RPC
# future raise: report the message on stderr and exit non-zero so the caller
# (e.g. test_must_fail) sees a failure.
try:
    resp = h.rpc(args.endpoint, json.dumps(payload)).get()
except Exception as err:
    sys.stderr.write(str(err) + "\n")
    sys.exit(1)

if resp is not None:
    sys.stdout.write(json.dumps(resp, sort_keys=True) + "\n")
