# FLAGS will be passed to both the C and C++ compiler
FLAGS += -Isrc/stk
CFLAGS +=
CXXFLAGS +=

# Careful about linking to libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/stk/*.cpp)
SOURCES += $(wildcard src/*.cpp)

DISTRIBUTABLES += $(wildcard LICENSE*) res soundfonts samples rawwaves

FLAGS += -w

RACK_DIR ?= ../..
include $(RACK_DIR)/plugin.mk
