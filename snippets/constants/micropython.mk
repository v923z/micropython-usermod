USERMODULES_DIR := $(USERMOD_DIR)

# Add all C files to SRC_USERMOD.
SRC_USERMOD_C += $(USERMODULES_DIR)/constants.c

CFLAGS_USERMOD += -I$(USERMODULES_DIR)