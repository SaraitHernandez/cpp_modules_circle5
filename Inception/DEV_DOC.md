# Developer documentation

How the project is built, how to set it up from nothing, and where the state
lives.

## 1. Repository layout

```
.
├── Makefile                  entry point for every operation
├── README.md                 project description and design rationale
├── USER_DOC.md               how to operate the stack
├── DEV_DOC.md                this file
├── SETUP_42_VM.md            step by step for the 42 virtual machine (Spanish)
├── TEORIA.md                 the theory behind every design decision (Spanish)
├── .gitignore                keeps secrets and .env out of git
├── secrets/                  one password per file — never committed
│   ├── db_root_password.txt
│   ├── db_password.txt
│   ├── wp_admin_password.txt
│   └── wp_user_password.txt
└── srcs/
    ├── .env                  local configuration — never committed
    ├── .env.example          committed template
    ├── docker-compose.yml
    └── requirements/
        ├── mariadb/
        │   ├── Dockerfile
        │   ├── .dockerignore
        │   ├── conf/zz-inception.cnf
        │   └── tools/{entrypoint.sh,healthcheck.sh}
        ├── nginx/
        │   ├── Dockerfile
        │   ├── .dockerignore
        │   ├── conf/nginx.conf.template
        │   └── tools/{entrypoint.sh,healthcheck.sh}
        └── wordpress/
            ├── Dockerfile
            ├── .dockerignore
            ├── conf/{www.conf,php-inception.ini}
            └── tools/{entrypoint.sh,healthcheck.sh}
```

## 2. Prerequisites

- Linux (this is meant to run in the 42 virtual machine)
- `docker` and the `docker compose` plugin, with your user in the `docker` group
- `make`, `git`, `openssl`
- Port 443 free on the host

Check:

```bash
docker --version && docker compose version && make --version
```

## 3. Setting up from scratch

```bash
git clone <repo> inception && cd inception
make setup
```

`make setup` performs three things, each idempotent:

1. **`srcs/.env`** — copied from `srcs/.env.example` if absent. Review it:
   `DOMAIN_NAME` must be `<your-login>.42.fr`, and `DATA_PATH` must be an
   absolute path your user can write to (`/home/<login>/data` on the 42 VM).
   It holds no passwords.
2. **`secrets/*.txt`** — four random 24-character alphanumeric passwords,
   generated with `openssl rand`, `chmod 600`, only if the file does not exist.
   Regenerating one means deleting the file and running `make setup` again —
   which only matters *before* the first `make`, because the credentials are
   burned into the database and into WordPress at first installation.
3. **`$DATA_PATH/mariadb` and `$DATA_PATH/wordpress`** — the `local` volume
   driver binds to these directories, and a bind target must already exist.

Then, once:

```bash
make hosts    # appends "127.0.0.1 <DOMAIN_NAME>" to /etc/hosts (sudo)
```

The Makefile `-include`s `srcs/.env`, so `DATA_PATH` and `DOMAIN_NAME` are
available to its own recipes. If the file is missing, GNU make creates it via
the rule and re-executes itself — a fresh clone therefore works in one command.

## 4. Building and running

```bash
make            # docker compose up --build --detach
make build      # build the images only
make up         # start without rebuilding
make down       # stop and remove containers + network
make re         # fclean, then a full rebuild
```

Everything funnels through one compose invocation:

```
docker compose --project-name inception \
               --file srcs/docker-compose.yml \
               --env-file srcs/.env
```

`--env-file` matters: `docker-compose.yml` is not in the repository root, and
without it compose would look for `.env` next to the compose file only for
interpolation, while the Makefile would read a different file.

### Startup order

`mariadb` → (healthy) → `wordpress` → `nginx`.

`wordpress` uses `depends_on: {mariadb: {condition: service_healthy}}`, so
compose blocks it until MariaDB's healthcheck passes. This is what replaces the
wait loop most implementations put in the entrypoint — the subject forbids
`while true` / `sleep` patterns, and this is both compliant and more correct.

### What each entrypoint does

**mariadb** — creates the system tables on an empty volume with
`mariadb-install-db`, writes a one-shot SQL file, and `exec`s
`mariadbd --user=mysql --console --init-file=/run/mysqld/init.sql`. The server
runs that SQL itself, once, at startup, with the grant system enabled.

> Why not `mariadbd --bootstrap`? Because bootstrap mode implies
> `--skip-grant-tables`, and `CREATE USER` / `GRANT` / `ALTER USER` are then
> rejected with `ERROR 1290`. This is the single most common way to get a
> silently broken database in this project: the daemon starts fine, and
> WordPress fails later with `Host '172.x.x.x' is not allowed to connect`.

On subsequent boots the volume is not empty, no init file is written, and the
server starts unchanged. `root@localhost` authenticates
`VIA unix_socket OR mysql_native_password`, which is what lets the healthcheck
ping the server without a password on the command line.

**wordpress** — reads the secrets, downloads WordPress core with `wp-cli` if
absent, generates `wp-config.php`, installs the site, creates the second
(non-administrator) user, then `exec`s `php-fpm83 -F`. Each step is guarded, so
restarting the container never reinstalls anything. It also refuses to start if
`WP_ADMIN_USER` contains `admin`, which the subject forbids.

`wp-config.php` never stores the database password. After generating it, the
entrypoint runs `wp config set DB_PASSWORD "trim( file_get_contents( ... ) )"
--raw`, so the file in the volume references the secret instead of containing
it.

**nginx** — renders `nginx.conf` from its template (substituting
`__DOMAIN_NAME__`), generates a self-signed certificate on first boot, runs
`nginx -t` to fail fast on a bad config, then `exec`s
`nginx -g "daemon off;"`. The certificate is generated at runtime, not at build
time, so the private key never becomes an image layer.

All three end in `exec "$@"`: the daemon replaces the shell and becomes PID 1,
receiving signals directly.

## 5. Managing containers and volumes

```bash
make ps                       # status + health of the three containers
make logs                     # follow all logs
docker logs -f wordpress      # one service

make shell-nginx              # a shell inside nginx (also -wordpress, -mariadb)
docker exec -it mariadb mariadb            # a SQL prompt (root via unix_socket)
docker exec wordpress su-exec www-data wp user list --path=/var/www/html

docker volume ls                           # mariadb_data, wordpress_data
docker volume inspect mariadb_data         # shows the host directory behind it
docker network inspect inception           # who is on the network
docker image ls                            # mariadb/wordpress/nginx :inception
```

A useful sanity pass before pushing or before a defense:

```bash
make check
```

It validates the compose file, asserts that nothing under `secrets/` and no
`srcs/.env` is tracked by git, greps the entrypoints for `tail -f` /
`sleep infinity` / `while true`, and greps the Dockerfiles for `:latest`.

### Testing the restart policy

`docker kill wordpress` will **not** demonstrate it: the daemon treats a kill
through the Docker API as a manual stop and suspends the restart policy. Kill
the process from outside the container's PID namespace instead — and note that
`kill -9 1` *inside* the container does nothing either, since the kernel
protects a PID namespace's init from signals sent within it:

```bash
sudo kill -9 $(docker inspect -f '{{.State.Pid}}' wordpress)
docker inspect -f '{{.RestartCount}} {{.State.Status}}' wordpress   # 1 running
```

## 6. Where the data lives, and how it survives

Two named volumes, both backed by a directory under `$DATA_PATH`:

```yaml
volumes:
  mariadb_data:
    driver: local
    driver_opts: { type: none, o: bind, device: ${DATA_PATH}/mariadb }
```

| Volume           | Mounted at        | On the host                     | Holds                        |
|------------------|-------------------|---------------------------------|------------------------------|
| `mariadb_data`   | `/var/lib/mysql`  | `$DATA_PATH/mariadb`            | InnoDB tablespaces, grants   |
| `wordpress_data` | `/var/www/html`   | `$DATA_PATH/wordpress`          | core, themes, plugins, uploads, `wp-config.php` |

`nginx` mounts `wordpress_data` at `/var/www/html:ro` — it serves the files, it
never writes them.

What this means in practice:

- `make down` / `make up`, a reboot, or a crash: **data survives**. The
  entrypoints detect the existing state and skip installation.
- `make clean`: containers and images go, **data survives**.
- `make fclean`: volumes *and* `$DATA_PATH/*` are deleted. **Data is gone.**
- Anything written *outside* those two paths — a file created in `/tmp`, a
  package installed with `apk add` inside a running container — lives in the
  container's writable layer and disappears on the next `make down`.

Backing up is therefore a plain copy of two directories, with the stack stopped:

```bash
make down
sudo tar czf inception-backup.tgz -C /home/sarherna data
```

## 7. Extending it

To add a service (the bonus part asks for Redis, an FTP server, Adminer, a
static site):

1. `srcs/requirements/<service>/` with a `Dockerfile`, `conf/` and `tools/`.
2. A service block in `docker-compose.yml`: `build`, `image: <name>:inception`,
   `container_name`, `restart: always`, `networks: [inception]`, plus a volume
   or a `healthcheck` when relevant.
3. Its non-secret configuration in `srcs/.env` and `srcs/.env.example`; any
   password as a new file in `secrets/` plus an entry in the top-level
   `secrets:` block.
4. Publish a port only if the service genuinely has to be reachable from
   outside. Keep NGINX the only entry point for anything HTTP.
