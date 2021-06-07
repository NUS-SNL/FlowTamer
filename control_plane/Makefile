# CREDITS: 
# This boiler plate code is heavily adapted from Intel Connectivity 
# Academy course ICA-1132: "Barefoot Runtime Interface & PTF"

ifndef SDE_INSTALL
$(error Environment variable SDE_INSTALL is not set)
endif

PROG=inNetworkCC

RTE_SDK := /home/cirlab/generate-traffic/dpdk-stable-19.11.5
PCAPPP_LIBS_DIR := -L$(RTE_SDK)/build/lib -L/lib64

PCAPPP_INCLUDES := -I/usr/local/include/pcapplusplus
PCAPPP_INCLUDES += -I$(RTE_SDK)/build/include
PCAPPP_BUILD_FLAGS := -fPIC
PCAPPP_BUILD_FLAGS += -msse -msse2 -msse3
PCAPPP_LIBS := $(PCAPPP_LIBS_DIR)
PCAPPP_LIBS += -lPcap++ -lPacket++ -lCommon++
PCAPPP_LIBS += -lpcap
PCAPPP_LIBS += -Wl,--whole-archive -lrte_pmd_bond -lrte_pmd_vmxnet3_uio -lrte_pmd_virtio -lrte_pmd_enic -lrte_pmd_i40e -lrte_pmd_fm10k -lrte_pmd_ixgbe -lrte_net -lrte_pmd_e1000 -lrte_pmd_ring -lrte_pmd_af_packet -lrte_kni -lrte_ethdev -lrte_eal -lrte_mbuf -lrte_mempool -lrte_ring -lrte_kvargs -lrte_hash -lrte_cmdline -lrt -lm -ldl -lpthread -Wl,--no-whole-archive
PCAPPP_LIBS += -Wl,--whole-archive -lrte_pci -lrte_bus_pci -lrte_bus_vdev -lrte_mempool_ring -lnuma -Wl,--no-whole-archive

CPPFLAGS = -I$(SDE_INSTALL)/include -I$(CURR_DIR) $(PCAPPP_INCLUDES) -DSDE_INSTALL=\"$(SDE_INSTALL)\" \
           -DPROG_NAME=\"$(PROG)\" -D__DIR__=\"${CURDIR}\"
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror $(PCAPPP_BUILD_FLAGS)
BF_LIBS  = -L$(SDE_INSTALL)/lib -lbf_switchd_lib -ldriver -lbfutils -lbfsys
LDLIBS   = $(BF_LIBS) $(PCAPPP_LIBS) -lm -ldl -lpthread
LDFLAGS  = -Wl,-rpath,$(SDE_INSTALL)/lib

ifndef $(BUILD)
	BUILD := release
endif

CURR_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BASE_BUILDDIR:=$(CURR_DIR)/build
BUILDDIR=$(BASE_BUILDDIR)/$(BUILD)

SOURCES = $(wildcard $(CURR_DIR)/*.cpp) \
		  $(wildcard $(CURR_DIR)/*/*.cpp)
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
	@$(RM) -rf *.tofino *.tofino2 zlog-cfg-cur bf_drivers.log*

cleaner:
	@$(RM) -rf $(BASE_BUILDDIR)
	@$(RM) $(RELEASE_BINARY) $(DEBUG_BINARY)
	@$(RM) -rf *.tofino *.tofino2 zlog-cfg-cur bf_drivers.log* __pycache__

$(OUTPUT_BINARY): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

-include $(DEPENDS)

# -MMD -MP are related to generating the .d depends file
$(BUILDDIR)/%.o: $(CURR_DIR)/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

