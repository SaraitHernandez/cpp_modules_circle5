#!/bin/sh
# Healthy = the TLS listener completes a handshake and answers HTTP.
# `-k` because the certificate is self-signed; we are talking to ourselves.
set -eu
exec curl -sk --max-time 3 -o /dev/null https://127.0.0.1:443/
