# =============================================================================
#  Inception — 42
#  Everything goes through docker compose; this Makefile only prepares the host
#  (env file, secrets, volume directories) and drives the stack.
# =============================================================================

NAME         := inception

COMPOSE_FILE := srcs/docker-compose.yml
ENV_FILE     := srcs/.env
ENV_EXAMPLE  := srcs/.env.example
SECRETS_DIR  := secrets

COMPOSE      := docker compose --project-name $(NAME) \
                --file $(COMPOSE_FILE) --env-file $(ENV_FILE)

SECRET_FILES := $(SECRETS_DIR)/db_root_password.txt \
                $(SECRETS_DIR)/db_password.txt \
                $(SECRETS_DIR)/wp_admin_password.txt \
                $(SECRETS_DIR)/wp_user_password.txt

# Pull DATA_PATH / DOMAIN_NAME out of the env file. If it is missing, the rule
# below creates it from the example and GNU make re-executes itself, so the
# variables are always available — even on a freshly cloned repository.
-include $(ENV_FILE)
export

GREEN := \033[0;32m
BLUE  := \033[0;34m
RED   := \033[0;31m
NC    := \033[0m

.DEFAULT_GOAL := all

.PHONY: all setup up build down stop start restart re \
        clean fclean prune logs ps status shell-% hosts check help

# -----------------------------------------------------------------------------
#  Build + run everything
# -----------------------------------------------------------------------------
all: setup
	@printf "$(BLUE)[inception]$(NC) building images and starting the stack...\n"
	@$(COMPOSE) up --build --detach
	@printf "$(GREEN)[inception]$(NC) up. Open https://$(DOMAIN_NAME)\n"

# -----------------------------------------------------------------------------
#  Host preparation
# -----------------------------------------------------------------------------
setup: $(ENV_FILE) $(SECRET_FILES) volumes-dir

$(ENV_FILE):
	@printf "$(BLUE)[inception]$(NC) creating $(ENV_FILE) from $(ENV_EXAMPLE)\n"
	@cp $(ENV_EXAMPLE) $(ENV_FILE)
	@printf "$(RED)[inception]$(NC) review $(ENV_FILE) (DATA_PATH, DOMAIN_NAME) before going further.\n"

# Random 24-character alphanumeric passwords, created once, never committed.
$(SECRETS_DIR)/%.txt:
	@mkdir -p $(SECRETS_DIR)
	@openssl rand -base64 32 | tr -dc 'A-Za-z0-9' | head -c 24 > $@
	@echo "" >> $@
	@chmod 600 $@
	@printf "$(BLUE)[inception]$(NC) generated secret $@\n"

# The `local` volume driver binds to these directories; they must pre-exist.
volumes-dir:
	@if [ -z "$(DATA_PATH)" ]; then \
		printf "$(RED)[inception]$(NC) DATA_PATH is not set in $(ENV_FILE)\n"; exit 1; \
	fi
	@mkdir -p "$(DATA_PATH)/mariadb" "$(DATA_PATH)/wordpress" 2>/dev/null || { \
		printf "$(RED)[inception]$(NC) cannot create $(DATA_PATH) — fix DATA_PATH in $(ENV_FILE)\n"; exit 1; }
	@printf "$(BLUE)[inception]$(NC) volume directories ready under $(DATA_PATH)\n"

# Maps the domain to the loopback address so the browser can find the site.
hosts:
	@if grep -q "[[:space:]]$(DOMAIN_NAME)\$$" /etc/hosts; then \
		printf "$(GREEN)[inception]$(NC) /etc/hosts already knows $(DOMAIN_NAME)\n"; \
	else \
		printf "$(BLUE)[inception]$(NC) adding $(DOMAIN_NAME) to /etc/hosts (sudo)\n"; \
		echo "127.0.0.1 $(DOMAIN_NAME)" | sudo tee -a /etc/hosts > /dev/null; \
	fi

# -----------------------------------------------------------------------------
#  Lifecycle
# -----------------------------------------------------------------------------
build: setup
	@$(COMPOSE) build

up: setup
	@$(COMPOSE) up --detach

down:
	@$(COMPOSE) down

stop:
	@$(COMPOSE) stop

start:
	@$(COMPOSE) start

restart:
	@$(COMPOSE) restart

re: fclean all

# -----------------------------------------------------------------------------
#  Cleaning
#  clean  : containers, network and images built by this project
#  fclean : the above + named volumes + the data on the host (destructive)
# -----------------------------------------------------------------------------
clean:
	@printf "$(BLUE)[inception]$(NC) removing containers, network and images...\n"
	@$(COMPOSE) down --rmi local --remove-orphans

fclean: clean
	@printf "$(RED)[inception]$(NC) removing named volumes and everything under $(DATA_PATH)\n"
	@docker volume rm -f mariadb_data wordpress_data 2>/dev/null || true
	@if [ -n "$(DATA_PATH)" ] && [ -d "$(DATA_PATH)" ]; then \
		rm -rf "$(DATA_PATH)/mariadb" "$(DATA_PATH)/wordpress" 2>/dev/null \
		|| sudo rm -rf "$(DATA_PATH)/mariadb" "$(DATA_PATH)/wordpress"; \
	fi
	@printf "$(GREEN)[inception]$(NC) clean slate.\n"

# Wipes *every* unused docker object on the machine, not just this project.
prune: fclean
	@docker system prune --all --force --volumes

# -----------------------------------------------------------------------------
#  Inspection
# -----------------------------------------------------------------------------
ps status:
	@$(COMPOSE) ps

logs:
	@$(COMPOSE) logs --follow --tail=100

# `make shell-nginx`, `make shell-mariadb`, `make shell-wordpress`
shell-%:
	@docker exec -it $* sh

# Quick sanity pass before pushing / before a defense.
check:
	@printf "$(BLUE)[inception]$(NC) compose file validity:\n"
	@$(COMPOSE) config --quiet && printf "  $(GREEN)ok$(NC)\n"
	@printf "$(BLUE)[inception]$(NC) secrets tracked by git (must be empty):\n"
	@git ls-files $(SECRETS_DIR) $(ENV_FILE) | sed 's/^/  /' ; \
		if [ -z "$$(git ls-files $(SECRETS_DIR) $(ENV_FILE))" ]; then printf "  $(GREEN)none$(NC)\n"; \
		else printf "  $(RED)LEAK — untrack these files$(NC)\n"; exit 1; fi
	@printf "$(BLUE)[inception]$(NC) forbidden patterns in entrypoints (comments ignored):\n"
	@if grep -rnE '^[^#]*(tail +-f|sleep +infinity|while +true|sleep +[0-9])' \
		srcs/requirements/*/tools/*.sh srcs/requirements/*/Dockerfile ; then \
		printf "  $(RED)found$(NC)\n"; exit 1; else printf "  $(GREEN)none$(NC)\n"; fi
	@printf "$(BLUE)[inception]$(NC) no ':latest' base image:\n"
	@if grep -rn 'FROM .*:latest' srcs/requirements/*/Dockerfile ; then \
		printf "  $(RED)found$(NC)\n"; exit 1; else printf "  $(GREEN)none$(NC)\n"; fi

help:
	@printf "$(BLUE)Inception$(NC) — available targets\n\n"
	@printf "  make            build the images and start the stack\n"
	@printf "  make setup      create .env, secrets and the volume directories\n"
	@printf "  make hosts      point $(DOMAIN_NAME) at 127.0.0.1 (sudo)\n"
	@printf "  make ps         container status\n"
	@printf "  make logs       follow the logs\n"
	@printf "  make shell-X    open a shell in container X (nginx|wordpress|mariadb)\n"
	@printf "  make down       stop and remove the containers\n"
	@printf "  make clean      containers + images\n"
	@printf "  make fclean     everything, including the data volumes\n"
	@printf "  make re         fclean then a full rebuild\n"
	@printf "  make check      pre-defense sanity checks\n"
