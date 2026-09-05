# -----------------------------------------------------------------------------
#  WordPress + php-fpm — no web server here, this container only speaks FastCGI.
# -----------------------------------------------------------------------------
ARG ALPINE_VERSION=3.23
FROM alpine:${ALPINE_VERSION}

LABEL maintainer="sarherna@student.42.fr" \
      description="Inception - WordPress running on php-fpm"

# Pinned on purpose: an unpinned download makes the build non-reproducible.
ARG WP_CLI_VERSION=2.12.0

RUN apk add --no-cache \
        curl \
        su-exec \
        php83 \
        php83-fpm \
        php83-mysqli \
        php83-curl \
        php83-dom \
        php83-exif \
        php83-fileinfo \
        php83-gd \
        php83-iconv \
        php83-intl \
        php83-mbstring \
        php83-opcache \
        php83-openssl \
        php83-phar \
        php83-session \
        php83-simplexml \
        php83-sodium \
        php83-tokenizer \
        php83-ctype \
        php83-xml \
        php83-xmlreader \
        php83-xmlwriter \
        php83-zip \
 # wp-cli's shebang is `#!/usr/bin/env php`; Alpine only ships `php83`.
 && ln -sf /usr/bin/php83 /usr/bin/php \
 # gid 82 (www-data) already exists in Alpine; only the user is missing.
 && adduser -u 82 -D -S -G www-data www-data \
 && mkdir -p /var/www/html /run/php /var/tmp/wp-cli \
 && chown -R www-data:www-data /var/www/html /run/php /var/tmp/wp-cli

# wp-cli: used by the entrypoint to install and configure WordPress.
RUN curl -fsSL -o /usr/local/bin/wp \
        "https://github.com/wp-cli/wp-cli/releases/download/v${WP_CLI_VERSION}/wp-cli-${WP_CLI_VERSION}.phar" \
 && chmod +x /usr/local/bin/wp \
 && wp --info --allow-root >/dev/null

COPY conf/www.conf        /etc/php83/php-fpm.d/www.conf
COPY conf/php-inception.ini /etc/php83/conf.d/99-inception.ini

COPY tools/entrypoint.sh   /usr/local/bin/entrypoint.sh
COPY tools/healthcheck.sh  /usr/local/bin/healthcheck.sh
RUN chmod +x /usr/local/bin/entrypoint.sh /usr/local/bin/healthcheck.sh

# wp-cli writes its cache here; www-data has no real home directory.
ENV WP_CLI_CACHE_DIR=/var/tmp/wp-cli

EXPOSE 9000

# `-F` keeps php-fpm in the foreground: it is exec'd by the entrypoint and
# becomes PID 1, so it reaps its own workers and reacts to SIGTERM.
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["php-fpm83", "-F"]
