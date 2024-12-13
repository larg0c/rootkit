# Nom du module
MODULE_NAME := neko

# Chemins de base
SRC_DIR := src
UTILS_DIR := $(SRC_DIR)/utils
BUILD_DIR := build
KERNEL_DIR := ../../linux-6.11.5

# Fichiers sources et objets
MODULE_SRC := $(SRC_DIR)/main.c $(wildcard $(UTILS_DIR)/*.c)
MODULE_OBJ := $(BUILD_DIR)/$(MODULE_NAME).ko

# Compiler et flags
CC := gcc
CFLAGS := -Wall -Wextra -O2 -g

# Cible par défaut
all: $(MODULE_OBJ)

# Compilation du module noyau
$(MODULE_OBJ): $(MODULE_SRC)
	@echo "Building kernel module: $(MODULE_OBJ)"
	@mkdir -p $(BUILD_DIR)
	make -C $(KERNEL_DIR) M=$(PWD)/$(BUILD_DIR) SRC_DIR=$(PWD)/$(SRC_DIR) modules

# Nettoyage des fichiers générés
clean:
	@echo "Cleaning up build artifacts..."
	make -C $(KERNEL_DIR) M=$(PWD)/$(BUILD_DIR) clean
	rm -rf $(BUILD_DIR)

# Cibles pour la génération d'images Docker ou QEMU si nécessaire
docker-build:
	@echo "Building Docker image for isolated environment..."
	docker build -t rootkit-vm -f Dockerfile .

docker-run:
	@echo "Running Docker container to set up environment..."
	docker run --rm -v $(PWD)/$(BUILD_DIR):/my-rootfs rootkit-vm /scripts/init.sh

.PHONY: all clean docker-build docker-run