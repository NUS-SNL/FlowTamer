# CREDITS: 
# This boiler plate code is heavily adapted from Intel Connectivity 
# Academy course ICA-1132: "Barefoot Runtime Interface & PTF"

ifndef SDE_INSTALL
$(error Environment variable SDE_INSTALL is not set)
endif

PROG=inNetworkCC

CPPFLAGS = -I$(SDE_INSTALL)/include -DSDE_INSTALL=\"$(SDE_INSTALL)\" \
           -DPROG_NAME=\"$(PROG)\" -D__DIR__=\"${CURDIR}\"
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror
BF_LIBS  = -L$(SDE_INSTALL)/lib -lbf_switchd_lib -ldriver -lbfutils -lbfsys
LDLIBS   = $(BF_LIBS) -lm -ldl -lpthread
LDFLAGS  = -Wl,-rpath,$(SDE_INSTALL)/lib

ifndef $(BUILD)
	BUILD := release
endif

CURR_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BASE_BUILDDIR:=$(CURR_DIR)/build
BUILDDIR=$(BASE_BUILDDIR)/$(BUILD)

SOURCES = $(wildcard $(CURR_DIR)/*.cpp)
OBJECTS = $(patsubst $(CURR_DIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.o))
DEPENDS = $(patsubst $(CURR_DIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.d))

RELEASE_BINARY := $(PROG)
DEBUG_BINARY := $(PROG)-debug


ifeq ($(BUILD), release)
CXXFLAGS += -O3
OUTPUT_BINARY := $(RELEASE_BINARY)
endif

ifeq ($(BUILD), debug)
CXXFLAGS += -O0 -g3 -DDEBUG
OUTPUT_BINARY := $(DEBUG_BINARY)
endif

.PHONY: all clean cleaner


# Final target
all: $(OUTPUT_BINARY)

clean:
	@$(RM) -rf $(BUILDDIR)
	@$(RM) $(OUTPUT_BINARY)
	@$(RM) -rf *.tofino *.tofino2 zlog-cfg-cur bf_drivers.log

cleaner:
	@$(RM) -rf $(BASE_BUILDDIR)
	@$(RM) $(RELEASE_BINARY) $(DEBUG_BINARY)
	@$(RM) -rf *.tofino *.tofino2 zlog-cfg-cur bf_drivers.log

$(OUTPUT_BINARY): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

-include $(DEPENDS)

# -MMD -MP are related to generating the .d depends file
$(BUILDDIR)/%.o: $(CURR_DIR)/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

