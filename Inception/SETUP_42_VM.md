# Guía paso a paso: montar Inception en la máquina de 42

Esta guía no es parte de lo que pide el subject — es tu manual de operaciones
para pasar de "no tengo nada" a "el proyecto corre en la VM y lo puedo
defender". El resto de documentos (`README.md`, `USER_DOC.md`, `DEV_DOC.md`)
están en inglés porque el subject lo exige; este está en castellano a propósito.

---

## 0. Lo que hay que tener claro antes de empezar

**El proyecto tiene que correr dentro de una máquina virtual.** No es negociable
ni es un detalle de estilo: el subject lo dice en la primera línea de las
*General guidelines* y en la primera del *Mandatory part*. Docker corriendo
directamente sobre la máquina de 42 no vale.

**El código sí se desarrolla fuera y se lleva dentro.** Todo el proyecto son
archivos de texto y las imágenes se construyen con `make` allí donde se ejecuta.
Lo que viaja es el repositorio, nunca imágenes ya construidas.

**Tu Mac es arm64 y la VM será x86_64.** Da igual: Alpine es multiarquitectura y
las imágenes se reconstruyen en destino. Lo único que rompería esto sería fijar
un `platform:` en el compose o fijar versiones de paquete al parche exacto — ni
una cosa ni la otra están en este proyecto.

**Dónde guardar la VM.** En 42 el `$HOME` suele tener una cuota pequeña y una VM
ocupa decenas de gigas. Guarda el disco virtual en `/goinfre/$USER` o
`/sgoinfre/students/$USER` según lo que tenga tu campus. Ojo: `/goinfre` se borra
al cerrar sesión en muchos campus — si es tu caso, usa `/sgoinfre`, que
persiste. Confírmalo antes de invertir dos horas.

---

## 1. Crear la máquina virtual

### 1.1 Descargar la ISO

Debian estable, imagen *netinst* para amd64, desde <https://www.debian.org/distrib/netinst>.
Guárdala también en `/sgoinfre/students/$USER`, no en el home.

> La regla de "penúltima versión estable" del subject aplica a la **imagen base
> de los contenedores** (aquí Alpine 3.23), no al sistema de la VM. Para la VM
> usa Debian estable a secas.

### 1.2 Crear la VM en VirtualBox

| Ajuste            | Valor                                                          |
|-------------------|----------------------------------------------------------------|
| Tipo              | Linux / Debian (64-bit)                                        |
| Memoria           | 4096 MB (2048 MB funciona, va justo)                           |
| CPUs              | 2                                                              |
| Disco             | 30 GB, VDI, reservado dinámicamente                            |
| **Carpeta**       | `/sgoinfre/students/<tu_login>/` — **no** el home               |
| Red               | NAT (suficiente: el navegador irá dentro de la VM)             |
| Aceleración       | VT-x/AMD-V activado                                            |

Si VirtualBox no está instalado en la máquina de 42 y no tienes `sudo` en el
host, usa GNOME Boxes o `virt-manager` (QEMU/KVM) — el resto de la guía es
idéntico.

### 1.3 Instalar Debian

Arranca desde la ISO y elige *Graphical install*. Puntos donde no improvisar:

- **Hostname**: `inception` (o lo que quieras).
- **Contraseña de root**: déjala **vacía**. Debian entonces añade tu usuario a
  `sudo` automáticamente, que es lo que quieres.
- **Usuario**: crea uno con tu login de 42 → `sarherna`. Esto importa: el
  subject exige que los volúmenes vivan en `/home/<login>/data`, y así ese
  directorio es tuyo sin pelearte con permisos.
- **Particionado**: *Guided – use entire disk*, todo en una partición.
- **Software selection**: marca **Xfce** y **standard system utilities**.
  Desmarca GNOME. Necesitas un escritorio con navegador dentro de la VM: en la
  máquina de 42 lo más probable es que no tengas `sudo` para tocar el
  `/etc/hosts` del host, así que la defensa se hace desde el navegador de la VM.

Reinicia y entra con tu usuario.

### 1.4 Ajustes posteriores

```bash
su -                          # si dejaste root sin contraseña, usa: sudo -i
apt update && apt upgrade -y
apt install -y sudo git make curl vim firefox-esr
usermod -aG sudo sarherna     # sólo si tu usuario aún no está en sudo
exit
```

Cierra sesión y vuelve a entrar para que el grupo `sudo` tome efecto.

Opcional pero cómodo: instala las *Guest Additions* para tener pantalla completa
y portapapeles compartido.

---

## 2. Instalar Docker en la VM

**No uses `apt install docker.io`.** Ese paquete trae `docker-compose` v1, y
este proyecto usa la sintaxis de compose v2 (`docker compose`, sin guion, con
`depends_on: condition: service_healthy`). Usa el repositorio oficial:

```bash
sudo apt-get update
sudo apt-get install -y ca-certificates curl gnupg

sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/debian/gpg \
     -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc

echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] \
https://download.docker.com/linux/debian $(. /etc/os-release && echo "$VERSION_CODENAME") stable" \
  | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

sudo apt-get update
sudo apt-get install -y docker-ce docker-ce-cli containerd.io \
                        docker-buildx-plugin docker-compose-plugin
```

Para no escribir `sudo` delante de cada comando:

```bash
sudo usermod -aG docker $USER
```

Cierra sesión y vuelve a entrar. Comprueba:

```bash
docker --version          # Docker version 2x.x
docker compose version    # Docker Compose version v2.x   <- v2, sin guion
docker run --rm hello-world
```

---

## 3. Llevar el código a la VM

El proyecto se entrega en su **propio repositorio de 42**, con el `README.md` en
la raíz. Ahora mismo esta carpeta vive dentro de `cpp_modules`, así que hay que
sacarla.

**Desde el Mac**, publica el contenido de esta carpeta en el repo de Inception
que te dé la intra:

```bash
cd ~/42/cpp_modules/Inception
git init
git add .
git commit -m "Inception: nginx + wordpress/php-fpm + mariadb"
git remote add origin <URL_del_repo_de_Inception_en_la_intra>
git push -u origin main
```

`.gitignore` ya excluye `secrets/*.txt` y `srcs/.env`, así que no se sube ni una
contraseña. Compruébalo antes de empujar:

```bash
git ls-files | grep -E 'secrets/|\.env$'    # no debe imprimir nada
```

**Dentro de la VM**, genera una clave SSH y añádela en la intra
(*Settings → SSH keys*), y clona:

```bash
ssh-keygen -t ed25519 -C "sarherna@student.42.fr"
cat ~/.ssh/id_ed25519.pub        # copia esto a la intra

git clone <URL_del_repo> ~/inception
cd ~/inception
```

Si el repo de la intra te da problemas, `scp` desde el Mac también sirve para
probar (`scp -r Inception/ usuario@ip_vm:~/inception`), pero la entrega final
tiene que estar en el repositorio de 42.

---

## 4. Configurar y arrancar

```bash
cd ~/inception
make setup
```

Esto crea `srcs/.env` a partir de `srcs/.env.example`, genera las cuatro
contraseñas aleatorias en `secrets/` y crea `/home/sarherna/data/{mariadb,wordpress}`.

**Revisa `srcs/.env`** — es el único archivo que depende de la máquina:

```bash
vim srcs/.env
```

| Variable        | Valor en la VM de 42        | Por qué                                    |
|-----------------|-----------------------------|--------------------------------------------|
| `LOGIN`         | `sarherna`                  | tu login                                    |
| `DOMAIN_NAME`   | `sarherna.42.fr`            | lo exige el subject                         |
| `DATA_PATH`     | `/home/sarherna/data`       | lo exige el subject                         |
| `WP_URL`        | `https://sarherna.42.fr`    | tiene que coincidir con `DOMAIN_NAME`       |
| `WP_ADMIN_USER` | `sarait`                    | **no puede contener `admin`**               |

> Si en el Mac lo cambiaste a `/Users/...` para probar en local, aquí tiene que
> volver a ser `/home/sarherna/data`. Como `.env` no está en git, el que se crea
> en la VM ya sale correcto desde `.env.example`.

Apunta el dominio a la propia VM y arranca:

```bash
make hosts     # añade "127.0.0.1 sarherna.42.fr" a /etc/hosts (pide sudo)
make           # construye las tres imágenes y levanta el stack
```

La primera vez tarda varios minutos. Al terminar:

```bash
make ps        # los tres en Up ... (healthy)
```

Abre Firefox **dentro de la VM** en <https://sarherna.42.fr>. El certificado es
autofirmado: acepta la advertencia. El panel está en `/wp-admin`, con el usuario
`sarait` y la contraseña de `secrets/wp_admin_password.txt`.

---

## 5. Comprobar que cumple el subject

Ejecuta esto en la VM antes de pedir la defensa. Cada bloque corresponde a un
requisito literal del enunciado.

```bash
# --- un servicio por contenedor, los tres arriba y sanos -------------------
make ps

# --- las imágenes son tuyas y ninguna usa el tag "latest" ------------------
docker image ls
grep -rn '^FROM' srcs/requirements/*/Dockerfile     # alpine:3.23, nunca :latest

# --- NGINX es la única entrada, y sólo por 443 ----------------------------
docker ps --format '{{.Names}}: {{.Ports}}'         # sólo nginx publica 443
curl -I http://sarherna.42.fr                       # tiene que fallar

# --- sólo TLSv1.2 y TLSv1.3 ----------------------------------------------
for v in -tls1 -tls1_1 -tls1_2 -tls1_3; do
  printf "%-8s " "$v"
  echo Q | openssl s_client -connect sarherna.42.fr:443 $v 2>&1 \
    | grep -qE "New, TLSv1\.[23]" && echo ACEPTADO || echo rechazado
done
# esperado: -tls1 rechazado, -tls1_1 rechazado, -tls1_2 y -tls1_3 aceptados

# --- volúmenes con nombre, apuntando a /home/<login>/data -----------------
docker volume ls
docker volume inspect mariadb_data | grep -E 'device|Name'
ls /home/sarherna/data/mariadb /home/sarherna/data/wordpress

# --- red docker propia, sin host ni links ---------------------------------
docker network inspect inception --format '{{.Driver}} {{range $k,$v := .Containers}}{{$v.Name}} {{end}}'
# (el ^[^#]* ignora los comentarios, que sí mencionan estas directivas)
grep -nE '^[^#]*(network_mode|links:|--link)' srcs/docker-compose.yml   # sin resultados

# --- dos usuarios de WordPress, el admin sin "admin" en el nombre ---------
docker exec wordpress su-exec www-data wp user list --path=/var/www/html \
       --fields=user_login,roles

# --- ninguna contraseña en el repositorio ni en los Dockerfiles -----------
git ls-files | grep -E 'secrets/|\.env$'            # sin resultados
grep -rniE 'password|passwd' srcs/requirements/*/Dockerfile   # sin resultados

# --- nada de hacks para mantener vivo el contenedor -----------------------
grep -rnE 'tail -f|sleep infinity|while true' srcs/requirements/*/tools/

# --- PID 1 es el demonio de verdad ---------------------------------------
docker exec nginx     ps -o pid,comm | head -3
docker exec wordpress ps -o pid,comm | head -3
docker exec mariadb   ps -o pid,comm | head -3

# --- persistencia: los datos sobreviven a parar y arrancar ---------------
make down && make up
curl -kI https://sarherna.42.fr        # 200, sin reinstalar nada
docker logs wordpress | tail -5        # "already installed", "already exists"

# --- reinicio automático tras un crash -----------------------------------
sudo kill -9 $(docker inspect -f '{{.State.Pid}}' wordpress)
sleep 15; docker inspect -f '{{.RestartCount}} {{.State.Status}}' wordpress
```

Y el atajo, que reúne varias de estas comprobaciones:

```bash
make check
```

### Sobre la prueba de reinicio

`docker kill wordpress` **no** sirve para demostrar `restart: always`: Docker
interpreta un kill vía su API como una parada manual y suspende la política de
reinicio. `kill -9 1` *dentro* del contenedor tampoco hace nada, porque el
kernel protege al PID 1 de un namespace frente a señales enviadas desde dentro.
Hay que matar el proceso desde el host, como en el comando de arriba. Es una
pregunta de defensa habitual y saber esto suma.

---

## 6. Antes de la defensa

1. **Haz un snapshot de la VM** con todo funcionando. Si algo se rompe montando
   la demo, vuelves atrás en treinta segundos.
2. **Prueba `make re` completo** al menos una vez: `fclean` borra los volúmenes
   y los datos, y la reconstrucción desde cero es exactamente lo que hará el
   evaluador. Si eso funciona, todo funciona.
3. **Confirma que el repo está limpio**: `git status` sin cambios sin subir, y
   `git ls-files` sin `secrets/` ni `.env`.
4. **Repasa el porqué de cada decisión.** Las preguntas que siempre caen:
   - ¿Por qué un contenedor no es una VM?
   - ¿Qué es PID 1 y por qué usas `exec` al final de los entrypoints?
   - ¿Por qué `depends_on: service_healthy` y no un bucle de espera?
   - ¿Diferencia entre un named volume y un bind mount? ¿Y entre `expose` y
     `ports`?
   - ¿Por qué las contraseñas están en `secrets/` y no en `.env`?
   - ¿Por qué `mariadbd --bootstrap` no puede crear usuarios? (implica
     `--skip-grant-tables`, y `CREATE USER` da error 1290)

   El apartado *Project description* del `README.md` responde a todas; léelo con
   calma, no lo recites.

---

## 7. Problemas frecuentes

| Síntoma | Causa y solución |
|---------|------------------|
| `docker compose` dice "unknown command" | Instalaste `docker.io` en vez del repo oficial. Instala `docker-compose-plugin`. |
| `permission denied ... docker.sock` | Falta tu usuario en el grupo `docker`, o no has vuelto a iniciar sesión desde que lo añadiste. |
| `mariadb` no llega a `healthy` en el primer arranque | `/home/sarherna/data/mariadb` no existe o no es tuyo. `make setup` lo crea; comprueba con `ls -ld`. |
| `wordpress` reinicia sin parar y el log dice `Host '172.x.x.x' is not allowed to connect` | El usuario de la base de datos no se creó. Casi siempre es por haber tocado el entrypoint de MariaDB. `make fclean && make`. |
| El navegador no encuentra `sarherna.42.fr` | Falta `make hosts`, o estás abriendo el navegador del host en vez del de la VM. |
| `502 Bad Gateway` | `wordpress` está caído o aún arrancando. `make ps` y `docker logs wordpress`. |
| `bind: address already in use` en el 443 | Algo más ocupa el puerto en la VM (¿un apache instalado por accidente?). `sudo ss -tlnp \| grep 443`. |
| Se llenó el disco de la VM | `docker system df` y luego `docker system prune -a`. Y considera ampliar el VDI. |
| El certificado caduca | Se genera a un año. `docker exec nginx rm -rf /etc/nginx/ssl && docker restart nginx` lo regenera. |

---

## 8. La teoría

Montar el proyecto y defenderlo son dos cosas distintas. Todo lo anterior es el
"cómo"; el "por qué" está en un documento aparte:

### → **[TEORIA.md](TEORIA.md)**

Cubre, atado a los archivos reales de este repositorio:

| # | Tema | Lo que hay que sacar de ahí |
|---|------|------------------------------|
| 1 | Virtualización y contenedores | namespaces, cgroups, VM vs contenedor |
| 2 | Arquitectura de Docker | dockerd, containerd, runc, OCI, el socket |
| 3 | Imágenes y capas | OverlayFS, copy-on-write, por qué borrar un secreto no lo borra |
| 4 | Dockerfile | ENTRYPOINT vs CMD, forma exec vs shell, ARG vs ENV, caché |
| 5 | PID 1, señales y demonios | por qué `exec "$@"`, por qué `tail -f` está prohibido |
| 6 | Docker Compose | `depends_on` + healthcheck, las tres formas de pasar variables |
| 7 | Redes | bridge propia vs `docker0`, DNS embebido, `expose` vs `ports` |
| 8 | Almacenamiento | named volume vs bind mount, y cómo se cumplen los dos requisitos a la vez |
| 9 | Secretos | dónde se filtra una variable de entorno, qué hace Compose de verdad |
| 10 | TLS y certificados | handshake 1.2 vs 1.3, X.509, SAN, autofirmados, SNI |
| 11 | NGINX | master/workers, prioridad de `location`, `try_files`, FastCGI vs proxy |
| 12 | CGI, FastCGI y php-fpm | por qué existe FastCGI, pools, `SCRIPT_FILENAME` |
| 13 | WordPress | `wp-config.php`, salts, roles, wp-cli, HTTPS tras un proxy |
| 14 | MariaDB | `usuario@host`, error 1130 vs 1045, `unix_socket`, `--init-file` |
| 15 | Make | `.PHONY`, `:=` vs `=`, por qué `-include` hace que make se reejecute |
| 16 | Linux de fondo | uid/gid (y por qué el 82 importa), permisos, señales |
| 17 | Banco de preguntas | las 20 preguntas de defensa, con respuesta corta |

**Cómo estudiarlo.** Léelo entero una vez sin prisa. Después vete solo a la
sección 17 e intenta responder en voz alta: cada pregunta que no salga sola te
dice qué sección releer. Los apartados **"en este proyecto"** de cada capítulo
son el puente entre la teoría y tu código — son los que hacen que la respuesta
suene a alguien que ha construido esto, y no a alguien que se ha leído un blog.
