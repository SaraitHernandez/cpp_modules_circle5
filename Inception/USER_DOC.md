# User documentation

For anyone who has to *run* this stack rather than modify it.

## 1. What the stack provides

Starting it gives you a working WordPress website served over HTTPS at
`https://<login>.42.fr` (here: `https://sarherna.42.fr`).

Three containers cooperate behind that single address:

| Container   | Role                                                                 |
|-------------|----------------------------------------------------------------------|
| `nginx`     | The web server and the only way in. Terminates TLS on port 443, serves static files, forwards `.php` to WordPress. |
| `wordpress` | WordPress running on php-fpm. Generates every page. Not reachable from outside. |
| `mariadb`   | The database holding all posts, pages, users and settings. Reachable only by WordPress. |

Two named volumes keep the data alive when containers are recreated:

| Volume           | Contains                    | On the host                        |
|------------------|-----------------------------|------------------------------------|
| `mariadb_data`   | the database                | `/home/sarherna/data/mariadb`      |
| `wordpress_data` | the WordPress site files    | `/home/sarherna/data/wordpress`    |

## 2. Starting and stopping

All commands are run from the root of the repository.

```bash
make          # build if needed, then start everything (this is the usual one)
make down     # stop and remove the containers — the data is kept
make stop     # pause the containers without removing them
make start    # resume them
make restart  # restart the three containers
```

The first `make` takes a few minutes: it builds three images and downloads
WordPress. Later runs take seconds.

**Erasing everything.** `make fclean` removes the containers, the images, the
volumes *and* the site data under `/home/sarherna/data`. Your posts are gone
for good. Use it when you want a demonstrably clean install, not as a way of
restarting the stack.

## 3. Reaching the website

The domain must resolve to the machine running the containers. Once:

```bash
make hosts        # adds "127.0.0.1 sarherna.42.fr" to /etc/hosts, asks for sudo
```

Then:

- Website: <https://sarherna.42.fr>
- Administration panel: <https://sarherna.42.fr/wp-admin>

The certificate is **self-signed** — 42 gives us no certificate authority — so
the browser warns you the first time. That is expected: click *Advanced* →
*Accept the risk and continue*. `http://` is not served at all; only port 443 is
open.

## 4. Accounts and passwords

There are two WordPress accounts, as required:

| Account       | Role          | Where the password lives           |
|---------------|---------------|------------------------------------|
| `sarait`      | administrator | `secrets/wp_admin_password.txt`    |
| `redactora`   | author        | `secrets/wp_user_password.txt`     |

And two database accounts:

| Account    | Role                       | Where the password lives            |
|------------|----------------------------|-------------------------------------|
| `wp_user`  | used by WordPress          | `secrets/db_password.txt`           |
| `root`     | database administrator     | `secrets/db_root_password.txt`      |

The user names live in `srcs/.env`; the passwords live only in `secrets/`, one
per file, readable by your user alone (`chmod 600`). They are generated randomly
by `make setup` and are excluded from git by `.gitignore` — they must never be
committed.

```bash
cat secrets/wp_admin_password.txt        # read the administrator password
```

**Changing a password.** Change it in WordPress (*Users* → *Profile*) or in the
database, and update the corresponding file so the two stay in sync. Changing
only the file has no effect on an already-installed site: the files are read at
*first* installation, and by WordPress at every request for the database
password.

## 5. Checking that everything is running

```bash
make ps
```

The three containers must be `Up` and `(healthy)`:

```
NAME        STATUS
nginx       Up 2 minutes (healthy)
wordpress   Up 2 minutes (healthy)
mariadb     Up 2 minutes (healthy)
```

`healthy` is stronger than `Up`: each container tests itself — MariaDB answers a
ping on its socket, php-fpm accepts a connection on port 9000, NGINX completes a
TLS handshake.

A quick end-to-end test:

```bash
curl -kI https://sarherna.42.fr        # expect: HTTP/2 200
```

To watch what the services are saying:

```bash
make logs                              # all three, live; Ctrl-C to stop
docker logs mariadb                    # just one of them
```

### If something is wrong

| Symptom                                   | Where to look                                              |
|-------------------------------------------|------------------------------------------------------------|
| Browser cannot find the site              | `make hosts` was not run, or the containers are down        |
| `502 Bad Gateway`                         | `wordpress` is down or still starting — `make ps`, `docker logs wordpress` |
| `wordpress` keeps restarting              | It cannot reach the database — `docker logs mariadb`        |
| `mariadb` unhealthy on first start        | `/home/sarherna/data/mariadb` is not writable by your user  |
| Certificate warning                       | Normal, the certificate is self-signed                      |

A container that crashes is restarted automatically (`restart: always`), so a
transient failure heals on its own; a container that keeps restarting has a real
problem and its logs will say which.
