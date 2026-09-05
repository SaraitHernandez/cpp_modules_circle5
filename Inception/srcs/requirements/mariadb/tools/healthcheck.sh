#!/bin/sh
# Reports "healthy" only once the server answers on its socket.
# root@localhost also authenticates via unix_socket, so no password is needed
# here and none ends up in `docker inspect` or in the process list.
set -eu
exec mariadb-admin --protocol=socket --socket=/run/mysqld/mysqld.sock ping --silent
