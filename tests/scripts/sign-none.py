#!/usr/bin/env python3
#
# sign-none.py - sign-wrap a jobspec with the flux "none" mechanism
#
# (see src/common/libjob/sign_none.c).  The "none" mechanism carries no
# cryptographic signature - it merely stamps the userid - so it is only
# accepted for the instance owner (or, in tests, via the FLUX_HANDLE_USERID
# testing hook).
#
# Usage: sign-none.py USERID < jobspec

import base64
import sys


def sign_none(payload: bytes, userid: int) -> bytes:
    header = f"version:i1:userid:i{userid}:mechanism:snone:"
    # argz encoding: ':' field separators become NUL bytes (including the
    # trailing one), matching header_encode() in sign_none.c.
    header_argz = header.replace(":", "\0").encode("ascii")
    h = base64.b64encode(header_argz).decode("ascii")
    p = base64.b64encode(payload).decode("ascii")
    return f"{h}.{p}.none".encode("ascii")


def main() -> int:
    if len(sys.argv) != 2:
        sys.stderr.write(f"usage: {sys.argv[0]} USERID < jobspec\n")
        return 1
    try:
        userid = int(sys.argv[1])
    except ValueError:
        sys.stderr.write(f"sign-none: invalid userid {sys.argv[1]}\n")
        return 1
    payload = sys.stdin.buffer.read()
    sys.stdout.buffer.write(sign_none(payload, userid))
    return 0


if __name__ == "__main__":
    sys.exit(main())
