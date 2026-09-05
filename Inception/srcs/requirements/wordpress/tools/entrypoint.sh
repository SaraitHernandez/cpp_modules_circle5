#!/bin/sh
# -----------------------------------------------------------------------------
#  WordPress entrypoint.
#
#  Installs WordPress the first time the volume is used, then `exec`s php-fpm,
#  which becomes PID 1. It never waits in a loop for MariaDB: docker-compose
#  holds this container back until the `mariadb` healthcheck reports healthy.
# -----------------------------------------------------------------------------
set -eu

WP_PATH="/var/www/html"

read_secret() {
	_file="/run/secrets/$1"
	if [ ! -r "$_file" ]; then
		echo "wordpress: missing secret '$1' (expected at $_file)" >&2
		exit 1
	fi
	cat "$_file"
}

DB_PASSWORD="$(read_secret db_password)"
WP_ADMIN_PASSWORD="$(read_secret wp_admin_password)"
WP_USER_PASSWORD="$(read_secret wp_user_password)"

: "${MYSQL_DATABASE:?wordpress: MYSQL_DATABASE is not set}"
: "${MYSQL_USER:?wordpress: MYSQL_USER is not set}"
: "${MYSQL_HOST:?wordpress: MYSQL_HOST is not set}"
: "${WP_URL:?wordpress: WP_URL is not set}"
: "${WP_TITLE:?wordpress: WP_TITLE is not set}"
: "${WP_ADMIN_USER:?wordpress: WP_ADMIN_USER is not set}"

# The subject forbids an administrator named admin/administrator.
case "$(echo "$WP_ADMIN_USER" | tr '[:upper:]' '[:lower:]')" in
	*admin*)
		echo "wordpress: WP_ADMIN_USER ('$WP_ADMIN_USER') must not contain 'admin'." >&2
		exit 1
		;;
esac

mkdir -p "$WP_PATH"
chown -R www-data:www-data "$WP_PATH"
cd "$WP_PATH"

# --- 1. WordPress core files --------------------------------------------------
if [ ! -f "$WP_PATH/index.php" ]; then
	echo "wordpress: downloading core..."
	su-exec www-data wp core download --path="$WP_PATH"
else
	echo "wordpress: core already present."
fi

# --- 2. wp-config.php ---------------------------------------------------------
if [ ! -f "$WP_PATH/wp-config.php" ]; then
	echo "wordpress: generating wp-config.php..."

	su-exec www-data wp config create \
		--path="$WP_PATH" \
		--dbname="$MYSQL_DATABASE" \
		--dbuser="$MYSQL_USER" \
		--dbpass="$DB_PASSWORD" \
		--dbhost="$MYSQL_HOST" \
		--dbcharset="utf8mb4" \
		--dbcollate="utf8mb4_unicode_ci" \
		--extra-php <<'PHP'
define( 'FS_METHOD', 'direct' );
define( 'DISALLOW_FILE_EDIT', true );

/* NGINX terminates TLS in front of us; without this WordPress would keep
   redirecting to http:// and loop. */
if ( isset( $_SERVER['HTTP_X_FORWARDED_PROTO'] ) && 'https' === $_SERVER['HTTP_X_FORWARDED_PROTO'] ) {
	$_SERVER['HTTPS'] = 'on';
}
PHP

	# wp-config.php lives in a *persistent volume*. Rather than writing the
	# database password into it, we make it read the Docker secret at request
	# time, so the password never lands on the volume at all.
	su-exec www-data wp config set DB_PASSWORD \
		"trim( file_get_contents( '/run/secrets/db_password' ) )" \
		--raw --path="$WP_PATH"
else
	echo "wordpress: wp-config.php already present."
fi

# --- 3. Site installation -----------------------------------------------------
if ! su-exec www-data wp core is-installed --path="$WP_PATH" 2>/dev/null; then
	echo "wordpress: installing site at $WP_URL ..."
	su-exec www-data wp core install \
		--path="$WP_PATH" \
		--url="$WP_URL" \
		--title="$WP_TITLE" \
		--admin_user="$WP_ADMIN_USER" \
		--admin_password="$WP_ADMIN_PASSWORD" \
		--admin_email="$WP_ADMIN_EMAIL" \
		--skip-email
else
	echo "wordpress: site already installed."
fi

# --- 4. Second, non-administrator user ----------------------------------------
if ! su-exec www-data wp user get "$WP_USER" --path="$WP_PATH" >/dev/null 2>&1; then
	echo "wordpress: creating user '$WP_USER' (${WP_USER_ROLE})..."
	su-exec www-data wp user create "$WP_USER" "$WP_USER_EMAIL" \
		--path="$WP_PATH" \
		--role="$WP_USER_ROLE" \
		--user_pass="$WP_USER_PASSWORD"
else
	echo "wordpress: user '$WP_USER' already exists."
fi

chown -R www-data:www-data "$WP_PATH"

# --- 5. Hand over PID 1 to php-fpm -------------------------------------------
echo "wordpress: starting $*"
exec "$@"
