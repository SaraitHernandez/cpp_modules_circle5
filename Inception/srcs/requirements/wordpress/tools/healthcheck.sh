#!/bin/sh
# Healthy = WordPress is configured *and* php-fpm accepts FastCGI connections.
set -eu
[ -f /var/www/html/wp-config.php ] || exit 1
exec php83 -r '$s = @fsockopen("127.0.0.1", 9000, $e, $m, 2); if (!$s) { exit(1); } fclose($s); exit(0);'
