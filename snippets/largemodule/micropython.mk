USERMODULES_DIR := $(USERMOD_DIR)

# Add all C files to SRC_USERMOD.
SRC_USERMOD += $(USERMODULES_DIR)/helper.c
SRC_USERMOD += $(USERMODULES_DIR)/largemodule.c

# We can add our module folder to include paths if needed
# This is not actually needed in this example.
CFLAGS_USERMOD += -I$(USERMODULES_DIR)