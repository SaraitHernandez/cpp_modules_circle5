#!/bin/sh
# -----------------------------------------------------------------------------
#  MariaDB entrypoint.
#
#  On the very first boot (empty volume) it creates the system tables and writes
#  a one-shot SQL file, which is handed to the server through `--init-file`.
#  MariaDB runs that file itself, once, right after startup and with the grant
#  system enabled — so no temporary server, no polling loop, no `sleep`.
#
#  (`mariadbd --bootstrap` cannot be used for this: it implies
#   --skip-grant-tables, and CREATE USER / GRANT are rejected with error 1290.)
#
#  The server is then `exec`ed, so it becomes PID 1 and handles SIGTERM itself.
# -----------------------------------------------------------------------------
set -eu

DATADIR="/var/lib/mysql"
INIT_SQL="/run/mysqld/init.sql"

read_secret() {
	_file="/run/secrets/$1"
	if [ ! -r "$_file" ]; then
		echo "mariadb: missing secret '$1' (expected at $_file)" >&2
		exit 1
	fi
	cat "$_file"
}

DB_PASSWORD="$(read_secret db_password)"
DB_ROOT_PASSWORD="$(read_secret db_root_password)"

: "${MYSQL_DATABASE:?mariadb: MYSQL_DATABASE is not set}"
: "${MYSQL_USER:?mariadb: MYSQL_USER is not set}"

# The bind-backed volume comes up owned by root the first time.
mkdir -p /run/mysqld
chown -R mysql:mysql /run/mysqld "$DATADIR"

# Never leave a previous run's credentials lying around in the container.
rm -f "$INIT_SQL"

if [ ! -d "$DATADIR/mysql" ]; then
	echo "mariadb: empty data directory, initialising..."

	mariadb-install-db \
		--user=mysql \
		--datadir="$DATADIR" \
		--skip-test-db \
		--auth-root-authentication-method=socket >/dev/null

	echo "mariadb: scheduling first-boot SQL..."

	cat > "$INIT_SQL" <<-EOSQL
		DELETE FROM mysql.global_priv WHERE User='';
		DROP USER IF EXISTS 'root'@'%';
		ALTER USER 'root'@'localhost' IDENTIFIED VIA unix_socket OR mysql_native_password USING PASSWORD('${DB_ROOT_PASSWORD}');
		CREATE DATABASE IF NOT EXISTS \`${MYSQL_DATABASE}\` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
		CREATE USER IF NOT EXISTS '${MYSQL_USER}'@'%' IDENTIFIED BY '${DB_PASSWORD}';
		GRANT ALL PRIVILEGES ON \`${MYSQL_DATABASE}\`.* TO '${MYSQL_USER}'@'%';
		FLUSH PRIVILEGES;
	EOSQL

	chown mysql:mysql "$INIT_SQL"
	chmod 600 "$INIT_SQL"

	# Appended to the CMD coming from the Dockerfile.
	set -- "$@" --init-file="$INIT_SQL"
else
	echo "mariadb: existing data directory found, skipping initialisation."
fi

echo "mariadb: starting $*"
exec "$@"
