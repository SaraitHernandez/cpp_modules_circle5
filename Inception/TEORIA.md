# Teoría de Inception

Todo lo que hay que entender para defender este proyecto, organizado por temas y
atado a los archivos reales del repositorio.

No es un documento para memorizar. En la defensa no se puntúa recitar la
definición de *namespace*: se puntúa que, cuando te pregunten por qué tu
entrypoint acaba en `exec "$@"`, sepas responder sin dudar. Cada sección termina
con **"en este proyecto"**, que es donde la teoría se vuelve concreta.

Índice:

1. [Virtualización y contenedores](#1-virtualización-y-contenedores)
2. [Arquitectura de Docker](#2-arquitectura-de-docker)
3. [Imágenes, capas y sistema de archivos](#3-imágenes-capas-y-sistema-de-archivos)
4. [El Dockerfile](#4-el-dockerfile)
5. [PID 1, señales y demonios](#5-pid-1-señales-y-demonios)
6. [Docker Compose](#6-docker-compose)
7. [Redes](#7-redes)
8. [Almacenamiento](#8-almacenamiento)
9. [Secretos y variables de entorno](#9-secretos-y-variables-de-entorno)
10. [TLS, HTTPS y certificados](#10-tls-https-y-certificados)
11. [NGINX](#11-nginx)
12. [CGI, FastCGI y php-fpm](#12-cgi-fastcgi-y-php-fpm)
13. [WordPress](#13-wordpress)
14. [MariaDB](#14-mariadb)
15. [Make](#15-make)
16. [Linux de fondo](#16-linux-de-fondo)
17. [Banco de preguntas de defensa](#17-banco-de-preguntas-de-defensa)

---

## 1. Virtualización y contenedores

### Máquina virtual

Un **hipervisor** emula hardware y sobre él arranca un sistema operativo
completo, con **su propio kernel**.

- **Tipo 1 (bare metal)**: corre directamente sobre el hardware. ESXi, Xen, KVM.
- **Tipo 2 (hosted)**: corre como aplicación sobre un SO anfitrión. VirtualBox,
  VMware Workstation, UTM.

Coste: cada VM arrastra un kernel, un `init`, servicios de sistema. Cientos de
megas de RAM y decenas de segundos de arranque antes de ejecutar nada útil.

Beneficio: aislamiento fuerte. La frontera es el hipervisor, y el kernel del
invitado es suyo — puedes correr Windows sobre Linux.

### Contenedor

Un contenedor **no** es una máquina pequeña: es un conjunto de procesos
normales del kernel anfitrión, a los que se les ha restringido lo que pueden
ver y consumir. Tres mecanismos del kernel de Linux lo hacen posible:

**Namespaces** — controlan *qué ve* un proceso. Cada uno aísla un recurso global:

| Namespace | Aísla                                                       |
|-----------|-------------------------------------------------------------|
| `pid`     | el árbol de procesos: dentro, tu proceso es el PID 1        |
| `mnt`     | los puntos de montaje: su propio sistema de archivos        |
| `net`     | interfaces, IPs, puertos, tablas de rutas e iptables        |
| `uts`     | hostname y nombre de dominio                                |
| `ipc`     | memoria compartida System V, colas de mensajes              |
| `user`    | mapeo de uid/gid: root dentro puede ser un usuario normal fuera |
| `cgroup`  | la vista del árbol de cgroups                               |
| `time`    | los relojes monotónicos y de arranque                       |

**Cgroups** — controlan *cuánto consume*: CPU, memoria, E/S, número de procesos.
Es lo que hace `docker run --memory 512m`.

**Capabilities y seccomp** — controlan *qué puede hacer*. Docker arranca los
contenedores con la mayoría de capacidades de root eliminadas y con un perfil
seccomp que bloquea decenas de llamadas al sistema peligrosas.

A esto se le añade un cambio de raíz del sistema de archivos (`pivot_root`, el
sucesor moderno de `chroot`).

### Comparación

|                        | Máquina virtual                 | Contenedor                          |
|------------------------|---------------------------------|-------------------------------------|
| Kernel                 | propio                          | el del anfitrión                    |
| Arranque               | decenas de segundos             | milisegundos a segundos             |
| Memoria en reposo      | cientos de MB                   | lo que consuma el proceso           |
| Tamaño en disco        | GB                              | MB (aquí: 26 MB nginx, 390 MB mariadb) |
| Aislamiento            | fuerte (frontera: hipervisor)   | menor (frontera: kernel compartido) |
| Puede cambiar de SO    | sí                              | no: siempre Linux sobre Linux       |
| Escapar del aislamiento| exploit del hipervisor          | exploit del kernel                  |

> **En este proyecto.** 42 pide *las dos cosas*, y no es redundante: la VM es la
> frontera de seguridad y portabilidad — puedes romperla entera y reinstalarla
> sin tocar la máquina del campus — y los contenedores son la unidad de
> servicio dentro de ella. Un contenedor que se corrompe se recrea con
> `make re` en segundos; ninguno de los tres puede ensuciar el sistema de la VM
> porque nada se instala fuera de las imágenes.

---

## 2. Arquitectura de Docker

Docker no es un único programa. Cuando escribes `docker run`:

```
docker (CLI)
   │  API REST sobre /var/run/docker.sock
   ▼
dockerd  (demonio: imágenes, redes, volúmenes, API)
   │
   ▼
containerd  (ciclo de vida de los contenedores, pull de imágenes)
   │
   ▼
containerd-shim  (uno por contenedor; sobrevive a reinicios de dockerd)
   │
   ▼
runc  (crea namespaces y cgroups, y hace exec del proceso)
```

**OCI** (Open Container Initiative) es el estándar que hace intercambiables las
piezas: define el *image spec* (formato de las imágenes) y el *runtime spec*
(cómo se arranca un contenedor). Por eso `podman` puede correr imágenes
construidas por Docker.

El **socket** `/var/run/docker.sock` es la razón de que tu usuario tenga que
estar en el grupo `docker`: hablar con ese socket equivale a ser root en la
máquina, porque puedes montar `/` dentro de un contenedor privilegiado.

> **En este proyecto.** `sudo usermod -aG docker $USER` en la VM. Y por eso
> mismo eso *no* se hace en una máquina compartida de producción.

---

## 3. Imágenes, capas y sistema de archivos

Una **imagen** es una plantilla de solo lectura: una lista ordenada de capas
más un JSON de configuración (comando por defecto, variables de entorno,
usuario, puertos expuestos). Cada capa es un tarball con las diferencias
respecto a la anterior, identificado por su `sha256` — el contenido *es* el
nombre, así que dos imágenes que comparten una capa la almacenan una sola vez.

Un **contenedor** es una imagen más una **capa escribible** encima.

**Copy-on-Write con OverlayFS**: el driver `overlay2` monta

- `lowerdir`: las capas de la imagen, solo lectura
- `upperdir`: la capa escribible del contenedor
- `merged`: la vista unificada que ve el proceso

Al leer un archivo se busca de arriba abajo. Al **escribir** uno que vive en
una capa inferior, se copia entero al `upperdir` primero (*copy-up*) y se
modifica la copia. Al borrarlo se crea un archivo especial *whiteout* que lo
oculta: **el contenido original sigue en la imagen**.

Consecuencias que importan:

1. Un `RUN rm secreto.txt` **no borra nada**: el archivo sigue en la capa
   anterior y `docker history` lo delata. Los secretos no se meten en una imagen
   ni para quitarlos después.
2. Escribir mucho en la capa del contenedor es lento (copy-up constante). Una
   base de datos escribe muchísimo → **por eso los datos van a un volumen**, que
   se salta el sistema de capas por completo.
3. Todo lo escrito fuera de un volumen desaparece al eliminar el contenedor.

> **En este proyecto.** `/var/lib/mysql` y `/var/www/html` son volúmenes; todo
> lo demás es efímero. Si entras con `make shell-wordpress` e instalas un
> paquete, desaparece en el siguiente `make down && make up`.

---

## 4. El Dockerfile

### Instrucciones

| Instrucción | Qué hace | Trampa |
|---|---|---|
| `FROM` | imagen base | Sin tag → `:latest`. Prohibido y no reproducible. |
| `RUN` | ejecuta en build, crea capa | Cada `RUN` es una capa; encadena con `&&`. |
| `COPY` | copia del contexto a la imagen | Preferible siempre. |
| `ADD` | como COPY + descomprime tar + descarga URLs | Magia implícita; evítala. |
| `ARG` | variable **de build** | Queda en `docker history`: **nunca un secreto**. |
| `ENV` | variable en la imagen y en el contenedor | Persiste, la ve todo proceso hijo. |
| `WORKDIR` | directorio de trabajo | Mejor que `RUN cd`, que no persiste. |
| `USER` | usuario a partir de ese punto | |
| `EXPOSE` | **documentación** de puertos | No abre nada. |
| `VOLUME` | crea un volumen anónimo | Difícil de anular después; evítalo. |
| `LABEL` | metadatos | |
| `HEALTHCHECK` | prueba de salud | Aquí lo declaramos en el compose. |
| `ENTRYPOINT` / `CMD` | qué se ejecuta | Ver abajo. |

`ARG` declarado **antes** de `FROM` solo es visible en el propio `FROM`; para
usarlo en el cuerpo hay que volver a declararlo después. Por eso en nuestros
Dockerfiles aparece `ARG ALPINE_VERSION=3.23` seguido de
`FROM alpine:${ALPINE_VERSION}`.

### ENTRYPOINT vs CMD

| Dockerfile | `docker run img` ejecuta | `docker run img otra cosa` ejecuta |
|---|---|---|
| solo `CMD ["a","b"]` | `a b` | `otra cosa` |
| solo `ENTRYPOINT ["a"]` | `a` | `a otra cosa` |
| ambos | `ENTRYPOINT + CMD` | `ENTRYPOINT + otra cosa` |

La regla mental: **ENTRYPOINT es el programa, CMD son sus argumentos por
defecto**.

**Forma exec vs forma shell** — esto sí cae en defensa:

```dockerfile
CMD ["nginx", "-g", "daemon off;"]    # exec  → PID 1 es nginx
CMD nginx -g "daemon off;"            # shell → PID 1 es /bin/sh, nginx es su hijo
```

La forma shell envuelve todo en `/bin/sh -c`. El resultado es que **PID 1 es la
shell**, que no reenvía señales: `docker stop` manda SIGTERM a la shell, nginx
no se entera, y a los 10 segundos llega SIGKILL. Usa siempre la forma exec.

### Caché de construcción

Docker cachea capa a capa. Una instrucción invalida la caché de todas las
siguientes si cambia su texto; `COPY` la invalida además si cambia el checksum
de los archivos copiados. De ahí la regla de oro: **lo que cambia poco, arriba**
(instalar paquetes) y **lo que cambia mucho, abajo** (copiar tu código).

`.dockerignore` reduce el contexto que se envía al demonio y, sobre todo, evita
copiar por accidente cosas que no deben estar en la imagen.

> **En este proyecto.** Tres Dockerfiles, uno por servicio, todos sobre
> `alpine:3.23` — la penúltima rama estable, como pide el subject (la estable
> actual es 3.24). Los tres acaban en `ENTRYPOINT` (script) + `CMD` (el demonio
> con sus flags), en forma exec, y el script termina en `exec "$@"`. Ninguno
> contiene una contraseña ni un `ARG` con datos sensibles.

---

## 5. PID 1, señales y demonios

### Qué es PID 1

En un sistema Linux, el PID 1 es `init` (hoy `systemd`) y tiene dos deberes:

1. **Adoptar huérfanos.** Cuando un proceso muere dejando hijos vivos, esos
   hijos se reasignan a PID 1.
2. **Recolectar zombis.** Un proceso que termina queda en estado *zombie* hasta
   que su padre llama a `wait()` para leer su código de salida. Si PID 1 no
   recolecta, la tabla de procesos se llena de zombis.

En un contenedor, PID 1 es **el proceso que arrancas tú**, dentro de su
namespace `pid`.

### El kernel trata a PID 1 de forma especial

Para un proceso normal, una señal sin manejador aplica su acción por defecto
(SIGTERM → terminar). Para **PID 1, el kernel no aplica acciones por defecto**:
solo se le entregan las señales para las que ha instalado un manejador
explícito.

Dos consecuencias muy concretas:

- Un PID 1 que no maneja SIGTERM **ignora `docker stop`**. Docker espera el
  plazo de gracia (10 s por defecto) y manda SIGKILL. Cada `make down` tarda
  diez segundos de más y el servicio muere de golpe, sin cerrar bien.
- **`kill -9 1` desde dentro del contenedor no hace nada.** El kernel protege al
  init de un namespace frente a señales enviadas desde dentro de ese namespace.
  Solo un proceso de un namespace *ancestro* puede matarlo. Lo comprobamos: hay
  que matar el proceso desde la VM, con el PID que da
  `docker inspect -f '{{.State.Pid}}'`.

### Por qué `exec "$@"`

`exec` **reemplaza** la imagen del proceso actual, conservando el PID. Sin él:

```
PID 1  /bin/sh entrypoint.sh
PID 7    └── nginx              ← el servicio real no es PID 1
```

La shell no reenvía señales a su hijo, así que vuelves al problema anterior. Con
`exec`, la shell desaparece y nginx *es* el PID 1.

### Por qué `tail -f` está prohibido

El truco consiste en lanzar el servicio en segundo plano y mantener vivo el
contenedor con algo que no termine nunca: `tail -f /dev/null`,
`sleep infinity`, `while true; do :; done`.

Es un antipatrón por una razón concreta, no por purismo: **si el servicio se
muere, el contenedor sigue "Up"**. Docker vigila al PID 1, y el PID 1 es `tail`,
que goza de perfecta salud. Pierdes el reinicio automático, pierdes la
detección de fallos, y `docker ps` te miente.

### Demonios y primer plano

Un demonio clásico hace `fork()`, `setsid()`, cierra sus descriptores y se va al
segundo plano. Dentro de un contenedor eso es exactamente lo que **no** quieres:
el proceso que arrancaste terminaría y el contenedor moriría con él.

Por eso todo servicio en contenedor se ejecuta en primer plano:

| Servicio | Flag | Efecto |
|---|---|---|
| NGINX | `-g "daemon off;"` | no hace fork al segundo plano |
| php-fpm | `-F` / `--nodaemonize` | idem |
| MariaDB | `--console` | primer plano y log de errores a stderr |

Y así los logs salen por stdout/stderr, que es de donde los recoge Docker
(`docker logs`, driver `json-file`).

> **En este proyecto.** `docker exec nginx ps -o pid,comm` da `1 nginx`; en
> wordpress da `1 php-fpm83`; en mariadb, `1 mariadbd`. Es la prueba directa de
> que no hay ningún envoltorio.

---

## 6. Docker Compose

Compose describe en un YAML lo que si no serían quince `docker run` con veinte
flags cada uno, y añade el orden de arranque, las dependencias y una gestión
unificada de redes, volúmenes y secretos.

**Compose v1 vs v2**: v1 era `docker-compose` (con guion), un binario en Python,
hoy descontinuado. v2 es `docker compose` (sin guion), un plugin del CLI escrito
en Go. Este proyecto usa v2 — de ahí la insistencia en no instalar `docker.io`
en Debian, que arrastra la v1.

### Piezas del archivo

- `services:` cada uno se convierte en un contenedor.
  - `build:` construir desde un Dockerfile (con `context` y `args`).
  - `image:` cómo se llamará la imagen construida (o cuál descargar).
  - `container_name:` nombre fijo, en vez de `proyecto-servicio-1`.
  - `restart:` política de reinicio.
  - `expose:` documentación; `ports:` publicación real al anfitrión.
  - `environment:` variables **dentro del contenedor**.
  - `depends_on:` orden de arranque.
  - `healthcheck:` cómo sabe Docker si el servicio está realmente listo.
- `volumes:`, `networks:`, `secrets:` en el nivel superior: objetos compartidos.

### depends_on y healthcheck

`depends_on` a secas solo garantiza el **orden de arranque**, no que el servicio
esté listo — un contenedor puede estar "arrancado" y su base de datos tardar
otros diez segundos en aceptar conexiones. Por eso existen las condiciones:

| Condición | Significa |
|---|---|
| `service_started` | el contenedor arrancó (por defecto) |
| `service_healthy` | su `healthcheck` pasó |
| `service_completed_successfully` | terminó con código 0 |

Un `healthcheck` tiene `test` (el comando), `interval`, `timeout`, `retries` y
`start_period`. Durante el `start_period` los fallos **no** cuentan para
`retries`: es el margen de arranque.

### Variables

Hay que distinguir tres cosas que se confunden siempre:

| Mecanismo | Para qué |
|---|---|
| `--env-file` (CLI) | valores para **interpolar `${...}` en el propio YAML** |
| `env_file:` (servicio) | variables **dentro del contenedor**, leídas de un archivo |
| `environment:` (servicio) | variables **dentro del contenedor**, escritas a mano |

> **En este proyecto.** `--env-file srcs/.env` rellena `${DOMAIN_NAME}`,
> `${DATA_PATH}`, `${ALPINE_VERSION}`… en el compose; `environment:` pasa a cada
> contenedor solo lo que ese contenedor necesita. Y `wordpress` espera a
> `mariadb` con `condition: service_healthy`, que es lo que sustituye al bucle
> de espera que el subject prohíbe.

### Políticas de reinicio

| Política | Cuándo reinicia |
|---|---|
| `no` | nunca (por defecto) |
| `on-failure[:n]` | si el código de salida ≠ 0 |
| `always` | siempre, y también al reiniciar el demonio |
| `unless-stopped` | como `always`, salvo si lo paraste tú |

Detalle que sorprende en la defensa: **una parada por la API de Docker
(`docker stop`, `docker kill`) suspende la política de reinicio**. El demonio
marca el contenedor como detenido manualmente. Para demostrar `restart: always`
hay que provocar un fallo real, matando el proceso desde la VM.

---

## 7. Redes

### Tipos de red

| Driver | Qué hace |
|---|---|
| `bridge` | red virtual privada con NAT hacia el exterior (por defecto) |
| `host` | el contenedor comparte el namespace de red del anfitrión: sin aislamiento |
| `none` | sin red |
| `overlay` | red entre varios hosts (Swarm) |
| `macvlan` | el contenedor recibe una MAC propia en la red física |

### bridge por defecto vs bridge propia

No son lo mismo, y es una pregunta habitual:

| | `docker0` (por defecto) | red definida por el usuario |
|---|---|---|
| Resolución por nombre | **no** | **sí**, DNS embebido en `127.0.0.11` |
| Aislamiento | todos los contenedores juntos | solo los de esa red |
| Conectar/desconectar en caliente | no | sí |

El DNS embebido es la razón de que `fastcgi_pass wordpress:9000;` funcione:
`wordpress` es el nombre del servicio, y Docker lo resuelve a la IP del
contenedor. Nada está codificado a fuego, y si el contenedor cambia de IP al
recrearse, sigue funcionando.

`links:` era el mecanismo antiguo para lo mismo (inyectaba entradas en
`/etc/hosts` y variables de entorno). Las redes propias lo sustituyeron y el
subject lo prohíbe explícitamente, junto con `network: host`.

### expose vs ports

- `expose: ["3306"]` — **metadatos**. No abre absolutamente nada. Documenta que
  el servicio escucha ahí para quien lea el compose.
- `ports: ["443:443"]` — publica de verdad: Docker añade una regla DNAT en
  `iptables` (más un `docker-proxy` en espacio de usuario como respaldo) que
  redirige el puerto del anfitrión al del contenedor.

El tráfico **entre contenedores de la misma red** no pasa por NAT: van por el
bridge directamente.

> **En este proyecto.** Una sola red, `inception`, driver `bridge`. Solo `nginx`
> tiene `ports: ["443:443"]`. MariaDB y php-fpm únicamente `expose`, así que
> desde la VM no hay forma de llegar a 3306 ni a 9000: no están publicados.
> Compruébalo con `docker ps --format '{{.Names}}: {{.Ports}}'`.

---

## 8. Almacenamiento

Cuatro formas de que un contenedor vea datos:

| Tipo | Dónde vive | Lo gestiona Docker | Sobrevive a `docker rm` |
|---|---|---|---|
| Capa escribible | `/var/lib/docker/overlay2/...` | sí | **no** |
| Volumen con nombre | `/var/lib/docker/volumes/<n>/_data` | sí | sí |
| Volumen anónimo | igual, con nombre aleatorio | sí | sí, pero no lo encuentras |
| Bind mount | ruta arbitraria del anfitrión | no | sí (es del anfitrión) |
| tmpfs | RAM | sí | no |

**Volumen con nombre vs bind mount** — la comparación que pide el subject:

Un *bind mount* mapea una ruta del anfitrión dentro del contenedor. Es cómodo
para desarrollar (editas fuera, se ve dentro), pero el contenedor hereda dueño y
permisos de esa ruta, la ruta debe existir, y el objeto no aparece en
`docker volume ls` ni tiene ciclo de vida propio.

Un *volumen con nombre* es un objeto de primera clase de Docker: tiene nombre,
driver, y se gestiona con `docker volume create/ls/inspect/rm`. Docker crea el
directorio y le pone los permisos correctos.

El subject exige **volúmenes con nombre** y además que los datos estén en
`/home/<login>/data`. Se cumplen las dos cosas diciéndole al driver `local` qué
directorio del anfitrión debe respaldar el volumen:

```yaml
mariadb_data:
  driver: local
  driver_opts:
    type: none        # no hay sistema de archivos que montar...
    o: bind           # ...es un bind
    device: ${DATA_PATH}/mariadb   # ...de este directorio
```

Sigue siendo un volumen con nombre para todos los efectos (`docker volume ls` lo
lista), y los bytes están donde el subject quiere. El directorio **tiene que
existir antes**: por eso `make setup` lo crea.

> **En este proyecto.** `mariadb_data` → `/var/lib/mysql`, y `wordpress_data` →
> `/var/www/html` en el contenedor de WordPress y `/var/www/html:ro` en el de
> NGINX. NGINX sirve los archivos, no los escribe: montarlo de solo lectura es
> gratis y elimina una clase entera de fallos.

---

## 9. Secretos y variables de entorno

### Por qué una variable de entorno no es un buen sitio para una contraseña

- Aparece entera en `docker inspect <contenedor>`.
- Aparece en `/proc/<pid>/environ`, legible por cualquiera con los permisos.
- **La hereda todo proceso hijo**, incluidos los que no tienen nada que ver.
- Suele acabar en logs, trazas de error y volcados de fallo.
- Se cuela en git con una facilidad pasmosa.

Y un `ARG` de build es peor todavía: queda registrado en `docker history` de la
imagen para siempre.

### Secretos de Docker

Un secreto se expone como un **archivo de solo lectura** en
`/run/secrets/<nombre>`. No forma parte de la imagen, no lo heredan los hijos, y
`docker inspect` muestra la ruta pero nunca el valor.

Aquí conviene ser exacta, porque es donde se pilla a quien ha copiado la
explicación de un blog:

- En **Swarm**, los secretos se guardan cifrados en el log raft del clúster y se
  entregan al contenedor en un **tmpfs** (RAM). Nunca tocan el disco del nodo.
- Fuera de Swarm, `docker compose` implementa el secreto como un **bind mount de
  solo lectura del archivo del anfitrión**. Se comprueba en un segundo:

```bash
docker inspect wordpress --format '{{range .Mounts}}{{.Type}} {{.Source}} -> {{.Destination}}{{println}}{{end}}'
# bind /ruta/secrets/db_password.txt -> /run/secrets/db_password  rw=false
```

Es decir: aquí la protección real viene de que el archivo sea `chmod 600`, esté
en `.gitignore` y se monte de solo lectura — **no** de ningún cifrado. Decirlo
así, sabiendo la diferencia, vale más que repetir "los secretos van cifrados".

> **En este proyecto.** El reparto es por sensibilidad: `srcs/.env` lleva
> configuración (dominio, nombre de la base de datos, nombre de usuario, rutas)
> y `secrets/*.txt` lleva las cuatro contraseñas, una por archivo. Ambos están
> en `.gitignore`; lo que se sube es `srcs/.env.example`.
>
> Y un detalle del que merece la pena hablar en la defensa: `wp-config.php` vive
> en un **volumen persistente**. En vez de escribir ahí la contraseña, el
> entrypoint deja `define('DB_PASSWORD', trim(file_get_contents('/run/secrets/db_password')));`
> — WordPress lee el secreto en cada petición y la contraseña nunca llega a
> tocar el volumen.

---

## 10. TLS, HTTPS y certificados

### El problema

Necesitas tres garantías sobre un canal público: **confidencialidad** (nadie
lee), **integridad** (nadie modifica) y **autenticidad** (hablas con quien
crees).

La criptografía **simétrica** (AES) es rápida pero exige una clave compartida.
La **asimétrica** (RSA, curvas elípticas) resuelve el reparto de claves pero es
lenta. TLS usa las dos: asimétrica para acordar una clave de sesión, simétrica
para el resto de la conversación.

### El handshake

**TLS 1.2** (dos vueltas antes de enviar datos):

```
Cliente → ClientHello    versiones, cifrados soportados, SNI
Servidor → ServerHello   cifrado elegido
         + Certificate   su certificado X.509
         + ServerKeyExchange, ServerHelloDone
Cliente → ClientKeyExchange, ChangeCipherSpec, Finished
Servidor → ChangeCipherSpec, Finished
```

**TLS 1.3** (una sola vuelta): el cliente **adivina** el grupo de intercambio de
claves y ya manda su `key_share` en el ClientHello. El servidor responde con el
suyo y a partir de ahí todo va cifrado, incluido su propio certificado.

TLS 1.3 además **elimina** todo lo que había envejecido mal: intercambio de
claves RSA (que no da *forward secrecy*), modos CBC, RC4, SHA-1, compresión y
renegociación. Todos sus cifrados son AEAD y **siempre** hay forward secrecy —
la propiedad de que robar la clave privada del servidor mañana no permite
descifrar el tráfico capturado hoy, porque las claves de sesión eran efímeras.

TLS 1.0 y 1.1 están retirados desde 2021 (RFC 8996). Por eso el subject pide
1.2 o 1.3 y nada más.

### Certificados X.509

Un certificado une una **identidad** (un nombre de dominio) a una **clave
pública**, y lo firma una **autoridad certificadora** (CA). El navegador confía
en un conjunto de CA raíz que trae de fábrica, y valida la cadena
certificado → intermedia → raíz.

Campos que importan:

- `Subject` con su `CN` (Common Name) — histórico.
- `Subject Alternative Name` (SAN) — **es el que miran los navegadores
  modernos**. Un certificado sin SAN se rechaza aunque el CN sea correcto.
- `notBefore` / `notAfter`: validez.
- `Issuer`: quién firma. En un **autofirmado**, `Issuer == Subject`.

Un certificado autofirmado cifra exactamente igual de bien: lo que no aporta es
**autenticidad**, porque nadie externo respalda que esa clave sea de ese
dominio. Por eso el navegador avisa. En 42 no hay CA, así que es lo correcto
aquí — y hay que saber decir que el aviso es por autenticidad, no por cifrado
débil.

**SNI** (Server Name Indication): el cliente manda el nombre del dominio en
claro en el ClientHello, para que un servidor con varios sitios en la misma IP
sepa qué certificado presentar. Es la razón de que `openssl s_client` necesite
`-servername`.

> **En este proyecto.** El entrypoint de NGINX genera el certificado con
> `openssl req -x509 -nodes -newkey rsa:2048 -days 365`, con `CN=sarherna.42.fr`
> y `-addext "subjectAltName=DNS:sarherna.42.fr,..."`. Se genera **en tiempo de
> ejecución**, no en el Dockerfile: así la clave privada nunca es una capa de
> imagen que pudieras publicar. En la config, `ssl_protocols TLSv1.2 TLSv1.3;` y
> ni una línea `listen 80`.

---

## 11. NGINX

### Arquitectura

Un proceso **master** (arranca como root, lee la configuración y abre el puerto
443 — solo root puede abrir puertos < 1024) y varios **workers** sin privilegios
que atienden las conexiones. Los workers son *event-driven*: un solo hilo
gestiona miles de conexiones con `epoll`, en vez de un proceso o hilo por
conexión como hacía Apache clásico.

### Estructura de la configuración

```nginx
user  nginx;              # contexto principal
events { ... }            # cómo se aceptan conexiones
http {
    server {              # un sitio virtual
        listen 443 ssl;
        server_name ...;
        location / { ... }   # qué hacer según la URI
    }
}
```

### Prioridad de `location`

No es "el primero que coincide". El orden real es:

1. `location = /ruta` — coincidencia exacta. Gana siempre.
2. `location ^~ /pre` — prefijo que, si gana, impide evaluar las regex.
3. `location ~ regex` / `~* regex` (insensible) — **en el orden del archivo**,
   gana la primera que coincida.
4. El prefijo más largo que coincida.

`try_files $uri $uri/ /index.php?$args;` significa: prueba el archivo tal cual,
luego como directorio, y si no existe ninguno, entrégaselo a `index.php`. Eso es
lo que hace funcionar los *permalinks* de WordPress.

`root` vs `alias`: con `root /var/www`, la URI `/a/b` busca `/var/www/a/b`
(concatena). Con `alias /var/www`, busca `/var/www/b` (sustituye el prefijo del
`location`).

### Reverse proxy vs pasarela FastCGI

- `proxy_pass http://backend;` — NGINX habla **HTTP** con el backend. Es un
  proxy inverso.
- `fastcgi_pass wordpress:9000;` — NGINX habla el **protocolo FastCGI**, que es
  binario y no es HTTP.

NGINX no puede ejecutar PHP por sí mismo: no lleva intérprete embebido (a
diferencia del `mod_php` de Apache). Traduce la petición HTTP a variables
FastCGI y se la pasa a php-fpm, que ejecuta y devuelve la respuesta.

> **En este proyecto.** Un único `server`, en 443, sin `listen 80` en ninguna
> parte. `location ~ \.php$` empieza por `try_files $uri =404;` — sin eso, con
> `cgi.fix_pathinfo` activo, una URL como `/imagen.jpg/x.php` podría hacer que
> php-fpm ejecutara `imagen.jpg` como código. Es una vulnerabilidad clásica y
> vale oro mencionarla.

---

## 12. CGI, FastCGI y php-fpm

**CGI** (1993): el servidor web lanza **un proceso nuevo por cada petición**, le
pasa la petición por variables de entorno y stdin, y lee la respuesta de stdout.
Simple y terriblemente lento: arrancar un intérprete de PHP por cada imagen de
la página no escala.

**FastCGI**: los procesos son **persistentes**. El servidor web mantiene una
conexión (socket unix o TCP) con un pool de trabajadores ya arrancados y les
envía las peticiones en un protocolo binario de registros. Se ahorra el arranque
del intérprete y se puede precompilar código en caché (OPcache).

**php-fpm** (*FastCGI Process Manager*) es la implementación oficial de PHP: un
master que gestiona **pools** de workers.

Modos de gestión de procesos (`pm`):

| Modo | Comportamiento |
|---|---|
| `static` | número fijo de workers |
| `dynamic` | entre `min_spare` y `max_spare`, hasta `max_children` |
| `ondemand` | se crean solo cuando hacen falta y mueren tras un tiempo |

Variables FastCGI clave: `SCRIPT_FILENAME` (qué archivo ejecutar — la que
importa de verdad), `DOCUMENT_ROOT`, `QUERY_STRING`, `REQUEST_METHOD`, `HTTPS`.

> **En este proyecto.** `srcs/requirements/wordpress/conf/www.conf` define el
> pool: usuario `www-data`, escucha en el **puerto TCP 9000** y `pm = dynamic`.
> Es TCP y no un socket unix por una razón concreta: un socket unix es un
> archivo, y NGINX vive en **otro contenedor** — compartirlo exigiría un volumen
> común solo para eso. Con la red de Docker, `wordpress:9000` es más limpio.
> `clear_env = no` deja que PHP vea las variables que pasa Compose.

---

## 13. WordPress

Un CMS en PHP sobre MySQL/MariaDB. Es la mitad "aplicación" de un stack **LEMP**
(Linux, Nginx, MySQL, PHP) — LAMP si el servidor fuera Apache.

Piezas que hay que saber situar:

- **`wp-config.php`** — credenciales de la base de datos, prefijo de tablas y
  las **salts**: ocho constantes aleatorias que se usan para firmar cookies y
  hashear sesiones. Cambiarlas cierra la sesión de todo el mundo.
- **Tablas** — `wp_posts`, `wp_users`, `wp_options`… `wp_options` guarda
  `siteurl` y `home`, que son las URL canónicas del sitio: si no coinciden con
  cómo entras, WordPress te redirige en bucle.
- **Roles** — `administrator`, `editor`, `author`, `contributor`, `subscriber`,
  de más a menos permisos.
- **wp-cli** — la herramienta oficial de línea de comandos: `wp core download`,
  `wp config create`, `wp core install`, `wp user create`. Permite instalar
  WordPress sin pasar por el asistente web, que es imprescindible para
  automatizar.

**HTTPS detrás de un proxy**: php-fpm recibe la conexión de NGINX en texto
plano, así que PHP no sabe por sí solo que el cliente venía por HTTPS. Si no se
le dice, WordPress genera URLs `http://` y redirige en bucle. Se resuelve
mirando la cabecera `X-Forwarded-Proto` y fijando `$_SERVER['HTTPS'] = 'on'`.

> **En este proyecto.** El contenedor lleva **WordPress + php-fpm y nada más** —
> sin servidor web dentro, como exige el subject. El entrypoint instala con
> wp-cli sólo si hace falta, crea el administrador (`sarait`, sin la cadena
> "admin" en el nombre, también por exigencia del subject) y un segundo usuario
> `redactora` con rol `author`. La corrección de `X-Forwarded-Proto` va en el
> `--extra-php` de `wp config create`.

---

## 14. MariaDB

Fork de MySQL creado en 2009 por sus desarrolladores originales tras la compra
de Sun (y por tanto de MySQL) por Oracle. Compatible a nivel de protocolo y de
SQL para casi todo.

### Conceptos

- **datadir** (`/var/lib/mysql`): donde viven los archivos. Aquí es un volumen.
- **InnoDB**: el motor de almacenamiento por defecto. Transaccional (ACID), con
  *buffer pool* en memoria y *redo log* para recuperarse de una caída.
- **Base de datos `mysql`**: las tablas del sistema. Desde MariaDB 10.4 los
  usuarios viven en **`mysql.global_priv`**; `mysql.user` sigue existiendo pero
  como vista de compatibilidad.
- **Las cuentas son `usuario@host`**. `'wp_user'@'%'` y `'wp_user'@'localhost'`
  son cuentas **distintas**, con contraseñas y permisos distintos. `%` es el
  comodín.

Dos errores que se confunden siempre y conviene distinguir:

| Error | Significa |
|---|---|
| **1130** `Host 'x' is not allowed to connect` | no existe ninguna cuenta que case con ese host |
| **1045** `Access denied` | la cuenta existe, la contraseña es incorrecta |

- **Plugins de autenticación**: `mysql_native_password` (contraseña),
  `unix_socket` (te identifica por tu usuario del sistema al conectar por el
  socket local, sin contraseña), `ed25519`. Se pueden combinar:
  `IDENTIFIED VIA unix_socket OR mysql_native_password USING PASSWORD('...')`.
- **`GRANT`** da permisos; **`FLUSH PRIVILEGES`** recarga las tablas de permisos
  cuando las has modificado con `INSERT`/`DELETE` directos.
- **Socket unix vs TCP**: en local, `/run/mysqld/mysqld.sock` es más rápido y
  permite `unix_socket`. Entre contenedores solo hay TCP.
- **`utf8mb4` vs `utf8`**: el `utf8` de MySQL usa 3 bytes y **no puede almacenar
  emojis**. `utf8mb4` es el UTF-8 de verdad. Usa siempre `utf8mb4`.

### Inicialización: el punto donde casi todo el mundo tropieza

- **`mariadb-install-db`** crea las tablas del sistema en un datadir vacío.
- **`mariadbd --bootstrap`** lee SQL de stdin, lo aplica y termina. **Pero
  implica `--skip-grant-tables`**, así que `CREATE USER`, `GRANT` y `ALTER USER`
  fallan con **error 1290**. Y falla en silencio para quien no mire los logs: el
  servidor arranca perfectamente y es *WordPress* quien se cae después con el
  error 1130.
- **`mariadbd --init-file=/ruta.sql`** es la solución: el servidor arranca
  normal, **con el sistema de permisos activo**, ejecuta ese SQL una vez y sigue
  sirviendo. Un solo proceso, sin servidor temporal y sin bucle de espera.

> **En este proyecto.** Exactamente eso: el entrypoint detecta el datadir vacío,
> llama a `mariadb-install-db`, escribe el SQL en `/run/mysqld/init.sql` y hace
> `exec mariadbd --user=mysql --console --init-file=...`. En arranques
> posteriores no escribe nada y arranca el servidor tal cual. `root@localhost`
> queda con `unix_socket OR mysql_native_password`, y por eso el healthcheck
> puede hacer `mariadb-admin ping` por el socket **sin contraseña en la línea de
> comandos** — que si no, saldría en `ps` dentro del contenedor.

---

## 15. Make

`make` decide qué reconstruir comparando fechas de modificación entre un
**objetivo** y sus **prerrequisitos**.

```make
objetivo: prerrequisito1 prerrequisito2
	receta          # ← TABULADOR, no espacios
```

- **`.PHONY`**: declara objetivos que no son archivos (`all`, `clean`, `re`).
  Sin esto, si existiera un archivo llamado `clean`, `make clean` no haría nada.
- **`:=` vs `=`**: `:=` se evalúa una vez, al leer el archivo. `=` se evalúa
  cada vez que se usa (expansión perezosa). Usa `:=` salvo que necesites lo
  contrario.
- **Automáticas**: `$@` el objetivo, `$<` el primer prerrequisito, `$^` todos.
- **Reglas de patrón**: `%.txt:` casa con cualquier archivo `.txt`.
- **`include`**: si el archivo incluido no existe pero hay una regla para
  crearlo, **make lo crea y se reejecuta a sí mismo** desde cero.

> **En este proyecto.** Ese último punto no es trivia: `-include srcs/.env` con
> una regla que lo crea desde `.env.example` es lo que hace que un clon recién
> hecho funcione con un solo `make`, sin que `DATA_PATH` salga vacío. La regla
> de patrón `$(SECRETS_DIR)/%.txt:` genera cada contraseña **solo si el archivo
> no existe** — que es justo lo que quieres: idempotente y sin machacar
> credenciales ya en uso.

---

## 16. Linux de fondo

### Usuarios, grupos y permisos

Todo proceso tiene un **uid** y un **gid**. Los nombres (`www-data`) son solo una
traducción de `/etc/passwd` y `/etc/group`: **el kernel solo ve números**.

Esto importa en Docker: si un archivo lo crea el uid 82 en un contenedor y otro
contenedor lo lee, lo que se compara es el **82**, no el nombre. Si en la segunda
imagen el uid 82 se llama de otra forma, da igual: funciona. Si `www-data` es el
uid 33 en una imagen y el 82 en otra, **no** funciona.

Permisos: `rwx` para dueño, grupo y otros. `644` = dueño lee y escribe, el resto
lee. `755` = lo mismo más ejecución (en un directorio, "ejecución" significa
poder entrar). `600` = solo el dueño, que es lo que llevan los archivos de
`secrets/`.

### Señales

| Señal | Número | Qué hace |
|---|---|---|
| `SIGHUP` | 1 | por convenio, recargar configuración |
| `SIGINT` | 2 | Ctrl-C |
| `SIGKILL` | 9 | mata sin remedio, **no se puede capturar** |
| `SIGTERM` | 15 | petición educada de terminar (la de `docker stop`) |

### Comandos útiles

```bash
ss -tlnp             # qué está escuchando y en qué puerto
ps -o pid,comm       # procesos (busybox, dentro de los contenedores)
id                   # tu uid, gid y grupos
getent group 82      # a qué grupo corresponde un gid
df -h / du -sh       # espacio en disco
journalctl -u docker # logs del demonio de Docker en la VM
```

---

## 17. Banco de preguntas de defensa

Respuestas cortas. Si alguna no la puedes contestar sin mirar, esa es tu
siguiente sección que releer.

**¿Diferencia entre una VM y un contenedor?**
La VM emula hardware y arranca su propio kernel; el contenedor son procesos del
kernel anfitrión aislados con namespaces y limitados con cgroups. La VM aísla
más y cuesta más; el contenedor arranca en segundos y pesa megas.

**¿Por qué el subject exige hacerlo en una VM si Docker ya aísla?**
Porque el aislamiento del contenedor es más débil (kernel compartido) y porque
así puedes romperlo y reinstalarlo todo sin tocar la máquina del campus.

**¿Qué es PID 1 y por qué me importa?**
El init del namespace de procesos. Adopta huérfanos, recolecta zombis, y el
kernel solo le entrega las señales para las que tiene manejador. Si tu PID 1 no
maneja SIGTERM, `docker stop` tarda diez segundos y acaba en SIGKILL.

**¿Por qué `exec "$@"` al final del entrypoint?**
Para que el demonio reemplace a la shell conservando el PID 1 y reciba las
señales directamente. Sin `exec`, la shell sigue siendo PID 1 y no las reenvía.

**¿Por qué está prohibido `tail -f`?**
Porque mantiene vivo el contenedor aunque el servicio haya muerto: Docker vigila
al PID 1, y el PID 1 sería `tail`. Pierdes el reinicio automático y `docker ps`
te miente.

**¿ENTRYPOINT o CMD?**
ENTRYPOINT es el programa, CMD sus argumentos por defecto. Lo que pasas en
`docker run` sustituye al CMD, no al ENTRYPOINT.

**¿Por qué no se puede usar el tag `latest`?**
Porque no identifica nada: la misma línea construye imágenes distintas en
momentos distintos. Adiós a la reproducibilidad.

**¿`expose` o `ports`?**
`expose` es documentación y no abre nada. `ports` publica de verdad con una
regla DNAT en iptables. Aquí solo NGINX usa `ports`, y solo el 443.

**¿Por qué una red propia y no la de por defecto?**
Por el DNS embebido: los contenedores se resuelven por nombre de servicio. En
`docker0` eso no existe y habría que usar IPs o `links`, que está obsoleto y
prohibido.

**¿Volumen con nombre o bind mount?**
El volumen es un objeto de Docker con nombre y ciclo de vida propio; el bind es
una ruta del anfitrión que Docker no gestiona. El subject exige volúmenes con
nombre; con `driver_opts` los respaldamos en `/home/<login>/data`, que también
lo exige.

**¿Por qué los datos de la base no van en la capa del contenedor?**
Porque es efímera y copy-on-write: se pierden al recrear el contenedor y escribir
ahí es lento.

**¿Secretos o variables de entorno?**
Las variables se ven en `docker inspect`, en `/proc/pid/environ` y las heredan
todos los hijos. El secreto es un archivo de solo lectura en `/run/secrets`.
Fuera de Swarm, Compose lo implementa como bind mount de solo lectura: la
protección viene del `chmod 600` y del `.gitignore`, no de un cifrado.

**¿Cómo esperas a que la base de datos esté lista?**
Con un `healthcheck` en `mariadb` y `depends_on: condition: service_healthy` en
`wordpress`. Compose hace la espera; no hay ningún bucle en el entrypoint.

**¿Por qué `--init-file` y no `--bootstrap`?**
Porque `--bootstrap` implica `--skip-grant-tables` y `CREATE USER` falla con
error 1290. `--init-file` lo ejecuta el servidor ya arrancado, con permisos
activos, en un solo proceso.

**¿Por qué solo TLS 1.2 y 1.3?**
1.0 y 1.1 están retirados (RFC 8996): cifrados rotos y sin forward secrecy
obligatorio. 1.3 además reduce el handshake a una vuelta y elimina el
intercambio de claves RSA.

**¿Por qué avisa el navegador del certificado?**
Porque es autofirmado: `Issuer == Subject` y ninguna CA de confianza lo
respalda. El cifrado es idéntico; lo que falta es la autenticidad.

**¿Por qué WordPress no lleva NGINX dentro?**
Porque el subject pide un servicio por contenedor, y porque separarlos permite
escalar o reemplazar cada pieza por su cuenta. Hablan por FastCGI en el 9000.

**¿Por qué php-fpm escucha en TCP y no en un socket unix?**
Porque un socket unix es un archivo y NGINX está en otro contenedor;
compartirlo exigiría un volumen extra. Con la red de Docker, `wordpress:9000`
es más simple.

**Demuéstrame que reinicia al caerse.**
`docker kill` no vale: Docker lo trata como parada manual y suspende la
política. `kill -9 1` desde dentro tampoco: el kernel protege al init de su
namespace. Hay que matar el proceso desde la VM con el PID de
`docker inspect -f '{{.State.Pid}}'`.
