# -----------------------------------------------------------------------------
#  NGINX — the single entrypoint of the infrastructure. TLS 1.2/1.3, port 443.
# -----------------------------------------------------------------------------
ARG ALPINE_VERSION=3.23
FROM alpine:${ALPINE_VERSION}

LABEL maintainer="sarherna@student.42.fr" \
      description="Inception - NGINX TLS reverse proxy for php-fpm"

RUN apk add --no-cache nginx openssl curl \
 && mkdir -p /run/nginx /etc/nginx/ssl /var/www/html \
 # The Alpine package ships a plain-HTTP vhost on :80. The subject allows 443
 # only, so it goes away and our own nginx.conf replaces the whole config.
 && rm -f /etc/nginx/http.d/default.conf \
 && chown -R nginx:www-data /run/nginx /etc/nginx/ssl

COPY conf/nginx.conf.template /etc/nginx/nginx.conf.template

COPY tools/entrypoint.sh  /usr/local/bin/entrypoint.sh
COPY tools/healthcheck.sh /usr/local/bin/healthcheck.sh
RUN chmod +x /usr/local/bin/entrypoint.sh /usr/local/bin/healthcheck.sh

EXPOSE 443

# `daemon off;` keeps nginx in the foreground as PID 1 — it is a real daemon
# option, not a `tail -f`-style workaround.
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["nginx", "-g", "daemon off;"]
