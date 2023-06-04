.PHONY: default all clean veryclean

PHONY :=

THIS_MAKE_FILE     := $(abspath $(lastword $(MAKEFILE_LIST)))
THIS_MAKE_FILE_DIR := $(realpath $(patsubst %/,%,$(dir $(THIS_MAKE_FILE))))

MKDIR := mkdir -p
RM    := rm -rf

PROJ_NAME := todothis
PROJ_DIR  := $(THIS_MAKE_FILE_DIR)
FRNT_DIR  := $(PROJ_DIR)/frontend
BACK_DIR  := $(PROJ_DIR)/backend

PHONY += default all
default: all

all:

# === Back-end targets
CXX   := /bin/clang++
CMAKE := cmake

BACK_BUILD_DIR := $(BACK_DIR)/build
BACK_EXEC      := $(BACK_BUILD_DIR)/Debug/$(PROJ_NAME)

$(BACK_BUILD_DIR):
	$(CMAKE) -S $(BACK_DIR) \
	         -B $(BACK_BUILD_DIR) \
	         -G "Ninja Multi-Config" \
	         -D CMAKE_CXX_COMPILER=$(CXX) \
	         -D CMAKE_EXPORT_COMPILE_COMMANDS=true

PHONY += build-backend configure-backend reconfigure-backend run-backend
PHONY += clean-backend veryclean-backend
clean-backend:
	$(RM) $(BACK_BUILD_DIR)

veryclean-backend: clean-backend
	$(RM) $(BACK_DIR)/.cache

configure-backend: $(BACK_BUILD_DIR)

reconfigure-backend: clean-backend configure-backend

build-backend: configure-backend
	$(CMAKE) --build $(BACK_BUILD_DIR)

run-backend: build-backend
	@exec $(BACK_EXEC)

# === Utility targets:

clean: clean-backend

veryclean: clean

.PHONY: $(PHONY)
