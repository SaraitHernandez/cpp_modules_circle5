# -----------------------------------------------------------------------------
#  MariaDB — built from Alpine, penultimate stable branch (no `latest` tag).
# -----------------------------------------------------------------------------
ARG ALPINE_VERSION=3.23
FROM alpine:${ALPINE_VERSION}

LABEL maintainer="sarherna@student.42.fr" \
      description="Inception - MariaDB server"

RUN apk add --no-cache \
        mariadb \
        mariadb-client \
        mariadb-common \
        mariadb-server-utils \
 && mkdir -p /run/mysqld /var/lib/mysql \
 && chown -R mysql:mysql /run/mysqld /var/lib/mysql

# Loaded last (alphabetical order inside /etc/my.cnf.d) so it wins over the
# defaults shipped by the Alpine package.
COPY conf/zz-inception.cnf /etc/my.cnf.d/zz-inception.cnf

COPY tools/entrypoint.sh   /usr/local/bin/entrypoint.sh
COPY tools/healthcheck.sh  /usr/local/bin/healthcheck.sh
RUN chmod +x /usr/local/bin/entrypoint.sh /usr/local/bin/healthcheck.sh

EXPOSE 3306

# The entrypoint bootstraps the database once, then `exec`s the command below,
# so mariadbd becomes PID 1 and receives SIGTERM directly. No tail -f, no sleep,
# no `while true`.
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["mariadbd", "--user=mysql", "--console"]
