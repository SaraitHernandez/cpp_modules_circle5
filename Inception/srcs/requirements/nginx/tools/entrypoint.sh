#!/bin/sh
# -----------------------------------------------------------------------------
#  NGINX entrypoint: renders the config for $DOMAIN_NAME, makes sure a TLS
#  certificate exists, then `exec`s nginx as PID 1.
# -----------------------------------------------------------------------------
set -eu

: "${DOMAIN_NAME:?nginx: DOMAIN_NAME is not set}"

SSL_DIR="/etc/nginx/ssl"
CRT="$SSL_DIR/inception.crt"
KEY="$SSL_DIR/inception.key"

# --- Config ------------------------------------------------------------------
sed "s|__DOMAIN_NAME__|${DOMAIN_NAME}|g" \
	/etc/nginx/nginx.conf.template > /etc/nginx/nginx.conf

# --- Self-signed certificate --------------------------------------------------
# 42 gives us no CA, so the certificate is self-signed: browsers will warn once
# and that is expected. It is generated here, not in the Dockerfile, so the
# private key never becomes part of a shareable image layer.
if [ ! -f "$CRT" ] || [ ! -f "$KEY" ]; then
	echo "nginx: generating a self-signed certificate for ${DOMAIN_NAME}..."
	mkdir -p "$SSL_DIR"
	openssl req -x509 -nodes \
		-newkey rsa:2048 \
		-days 365 \
		-keyout "$KEY" \
		-out "$CRT" \
		-subj "/C=ES/ST=Madrid/L=Madrid/O=42/OU=42-Inception/CN=${DOMAIN_NAME}" \
		-addext "subjectAltName=DNS:${DOMAIN_NAME},DNS:www.${DOMAIN_NAME}" \
		2>/dev/null
	chmod 600 "$KEY"
	chmod 644 "$CRT"
else
	echo "nginx: reusing the existing certificate."
fi

# Fail fast and loudly instead of restart-looping on a typo.
nginx -t

echo "nginx: starting $*"
exec "$@"
