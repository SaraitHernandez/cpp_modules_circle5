*This project has been created as part of the 42 curriculum by sarherna.*

# Inception

## Description

Inception is a system-administration project. The goal is to build, from
scratch, a small containerised infrastructure that serves a WordPress site over
HTTPS, and to run it inside a virtual machine.

Three services, three containers, one Docker network:

| Container   | Contents                        | Reachable from                |
|-------------|---------------------------------|-------------------------------|
| `nginx`     | NGINX, TLS 1.2/1.3, port 443    | the outside world (only entry)|
| `wordpress` | WordPress + php-fpm (port 9000) | `nginx` only                  |
| `mariadb`   | MariaDB (port 3306)             | `wordpress` only              |

Everything is built from a `Dockerfile` written for this project on top of
Alpine 3.23 — the penultimate stable branch. No ready-made application image is
pulled from Docker Hub.

```
                                    ┌───────── WWW
                                    │  443/tcp (TLS 1.2 / 1.3)
┌───────────────────────────────────┼───────────────────────────────────┐
│ host                              ▼                                   │
│   ┌─── docker network "inception" (bridge) ────────────────────────┐   │
│   │  ┌─────────┐  3306   ┌──────────────────┐  9000   ┌─────────┐  │   │
│   │  │ mariadb │◄───────►│ wordpress+php-fpm│◄───────►│  nginx  │  │   │
│   │  └────┬────┘         └────────┬─────────┘         └────┬────┘  │   │
│   └───────┼───────────────────────┼────────────────────────┼───────┘   │
│      mariadb_data            wordpress_data ───────────────┘ (read-only)│
│    /home/<login>/data/mariadb    /home/<login>/data/wordpress           │
└────────────────────────────────────────────────────────────────────────┘
```

## Instructions

Requirements: a Linux host (or VM) with `docker`, the `docker compose` plugin,
`make`, `git` and `openssl`.

```bash
git clone <this-repo> inception && cd inception

make setup          # creates srcs/.env, generates secrets, creates the volume dirs
$EDITOR srcs/.env   # check DOMAIN_NAME and DATA_PATH
make hosts          # maps <login>.42.fr to 127.0.0.1 (asks for sudo)
make                # builds the three images and starts the stack
```

Then open `https://<login>.42.fr`. The certificate is self-signed, so the
browser shows a warning once — accept it.

Useful targets:

| Command            | Effect                                                |
|--------------------|-------------------------------------------------------|
| `make`             | build the images and start everything                 |
| `make ps`          | container status and health                           |
| `make logs`        | follow the logs of the three containers               |
| `make shell-nginx` | open a shell inside a container                       |
| `make down`        | stop and remove the containers                        |
| `make clean`       | the above, plus the images built here                 |
| `make fclean`      | the above, plus the volumes and the data on the host  |
| `make re`          | `fclean` then a full rebuild                          |
| `make check`       | pre-defense sanity checks                             |

`make help` lists them all. Day-to-day usage is described in
[USER_DOC.md](USER_DOC.md); the internals are in [DEV_DOC.md](DEV_DOC.md).

## Project description

### How Docker is used here

Each service gets its own `Dockerfile` under
`srcs/requirements/<service>/`, built by `docker compose` and named after the
service (`mariadb:inception`, `wordpress:inception`, `nginx:inception`). The
`inception` tag is explicit on purpose: an untagged `image:` entry would produce
`nginx:latest`, and `latest` is forbidden by the subject — it would also be
indistinguishable, in `docker images`, from the official image pulled from
Docker Hub.

The only thing downloaded from a registry is `alpine:3.23`, which the subject
explicitly allows. WordPress itself is fetched by `wp-cli` at first boot, and
`wp-cli` is pinned to release 2.12.0 so builds stay reproducible.

Every container runs a real daemon as PID 1, `exec`ed by its entrypoint:
`mariadbd`, `php-fpm83 -F`, `nginx -g "daemon off;"`. There is no `tail -f`, no
`sleep infinity` and no `while true` anywhere — a container is not a virtual
machine, and keeping it alive artificially would hide the fact that the service
died.

### Main design choices

**Waiting for the database without polling.** WordPress must not start before
MariaDB accepts connections. Instead of looping in the entrypoint, `mariadb`
declares a `healthcheck` and `wordpress` declares
`depends_on: {mariadb: {condition: service_healthy}}`. Compose does the waiting.

**Initialising MariaDB with `--init-file`.** The obvious approach,
`mariadbd --bootstrap`, cannot work: bootstrap mode implies
`--skip-grant-tables`, so `CREATE USER` and `GRANT` fail with error 1290. The
entrypoint instead writes the SQL once and passes it to the real server via
`--init-file`; MariaDB executes it at startup, with the grant system enabled,
and keeps running. One process, no temporary server, no wait loop.

**The database password never lands in the volume.** `wp-config.php` lives in a
persistent volume, so instead of a literal password it contains
`define( 'DB_PASSWORD', trim( file_get_contents( '/run/secrets/db_password' ) ) );`.
The password is read from `/run/secrets/db_password` at request time and never
written to the volume.

**NGINX is the only door.** It is the only container publishing a port, and it
publishes 443 only — there is no `listen 80` anywhere, so the infrastructure
cannot be reached in clear text at all. Its copy of the WordPress volume is
mounted read-only: it serves static files, it never writes them.

### Virtual Machines vs Docker

A virtual machine emulates hardware and boots a complete guest kernel; a
container is a set of processes on the *host* kernel, isolated by namespaces
(PID, network, mount, user) and limited by cgroups.

The practical consequences: a VM starts in tens of seconds and costs hundreds of
megabytes of RAM for the guest OS alone, while these three containers start in a
couple of seconds and share the host kernel. In exchange, a VM isolates far more
strongly — a container escape is a kernel exploit away — and a VM can run a
different kernel or a different OS entirely, which a container cannot.

That is exactly why this project uses both: the VM is the security and
portability boundary given to us by 42, and the containers are the unit of
service inside it.

### Secrets vs Environment Variables

An environment variable is convenient but leaky: it shows up in
`docker inspect`, it is inherited by every child process, it is visible in
`/proc/<pid>/environ`, and it is easy to commit by accident.

A Docker secret is a file exposed read-only at `/run/secrets/<name>`. It is
never baked into the image, it is not inherited by child processes, and
`docker inspect` shows only the path — never the value.

Worth being precise about, because it is a classic defense question: outside of
Swarm, `docker compose` implements a secret as a **read-only bind mount** of the
host file. It is Swarm that stores secrets in its encrypted raft log and
delivers them on a tmpfs. So here the real protection comes from the file being
`chmod 600`, git-ignored, and mounted read-only — not from any encryption.

So this project splits them by sensitivity: `.env` carries configuration
(`DOMAIN_NAME`, `MYSQL_DATABASE`, `MYSQL_USER`, `DATA_PATH`, the admin's login),
`secrets/*.txt` carries the four passwords. Both are in `.gitignore`;
`srcs/.env.example` is the committed template.

### Docker Network vs Host Network

With `network_mode: host` a container shares the host's network namespace: no
isolation, no per-container ports, and MariaDB's 3306 would be listening on the
host itself.

The `inception` bridge network gives each container its own namespace and an
embedded DNS server, so `wordpress` reaches the database simply as `mariadb:3306`
and NGINX reaches php-fpm as `wordpress:9000`. Only what is explicitly published
(443, by NGINX) crosses to the host. `expose` — used for 3306 and 9000 — is pure
documentation: it opens nothing.

This is also why `links:` is not needed: it is the legacy mechanism that user
defined networks replaced, and the subject forbids it.

### Docker Volumes vs Bind Mounts

A bind mount maps an arbitrary host path into a container. It is great for
development — edit on the host, see it live — but the container inherits
whatever ownership and permissions the host path has, and nothing about it is
described in `docker volume ls`.

A named volume is a first-class Docker object with a name and a lifecycle,
managed by a driver and listed by `docker volume ls`. The subject demands named
volumes, and additionally demands that their data sit under `/home/<login>/data`.

Both are satisfied by declaring named volumes whose `local` driver is told which
host directory to back them with:

```yaml
volumes:
  mariadb_data:
    driver: local
    driver_opts: { type: none, o: bind, device: ${DATA_PATH}/mariadb }
```

`docker volume ls` shows real named volumes; the bytes live exactly where the
subject wants them.

## Resources

- [Docker documentation](https://docs.docker.com/) — Dockerfile reference, best practices, `ENTRYPOINT` vs `CMD`
- [Compose file specification](https://docs.docker.com/reference/compose-file/)
- [Docker Compose secrets](https://docs.docker.com/compose/how-tos/use-secrets/)
- [Alpine Linux packages](https://pkgs.alpinelinux.org/packages) — checking what a branch actually ships
- [MariaDB: `--init-file` and server startup](https://mariadb.com/kb/en/mysqld-options/)
- [MariaDB authentication with `unix_socket`](https://mariadb.com/kb/en/authentication-plugin-unix-socket/)
- [php-fpm configuration](https://www.php.net/manual/en/install.fpm.configuration.php)
- [NGINX `ngx_http_ssl_module`](https://nginx.org/en/docs/http/ngx_http_ssl_module.html)
- [WP-CLI commands](https://developer.wordpress.org/cli/commands/)
- [Hardening WordPress](https://developer.wordpress.org/advanced-administration/security/hardening/)

### Use of AI

AI (Claude Code) was used as an assistant on this project, mainly for:

- scaffolding the repository layout and a first draft of the `Dockerfile`s,
  `docker-compose.yml`, entrypoints and `Makefile`;
- checking which package names and versions the pinned Alpine branch really
  ships, instead of guessing;
- drafting this documentation.

It was **not** used as a black box. Every generated file was read, tested and
corrected — two of its first attempts were plainly wrong and were fixed after
running them: `mariadbd --bootstrap` cannot create users because bootstrap mode
implies `--skip-grant-tables`, and `log_error = /dev/stderr` makes MariaDB try
to open `/dev/stderr.err` and fail. The debugging, the design decisions and the
final code are mine, and I can explain and justify every line of them.
