#######################################################################################
.PHONY: help
help:
	@echo "Makefile Usage:"
	@echo ""
	@echo "  make all"
	@echo "      Command to build client, encoder and decoder."
	@echo ""
	@echo "  make client"
	@echo "      Command to build client."
	@echo ""
	@echo "  make encoder"
	@echo "      Command to build encoder."
	@echo ""
	@echo "  make decoder"
	@echo "      Command to build decoder."
	@echo ""
	@echo "  make clean"
	@echo "      Command to remove the generated files."
	@echo ""
#######################################################################################

# Compiler tools
HOST_CXX ?= aarch64-linux-gnu-g++
RM = rm -f
RMDIR = rm -rf

VITIS_PLATFORM = u96v2_sbc_base
VITIS_PLATFORM_DIR = ${PLATFORM_REPO_PATHS}
VITIS_PLATFORM_PATH = $(VITIS_PLATFORM_DIR)/u96v2_sbc_base.xpfm

# Host compiler global settings
CXXFLAGS += -march=armv8-a+simd -mtune=cortex-a53 -std=c++11 -DVITIS_PLATFORM=$(VITIS_PLATFORM) -D__USE_XOPEN2K8 -I$(XILINX_VIVADO)/include/ -I$(VITIS_PLATFORM_DIR)/sw/u96v2_sbc_base/PetaLinux/sysroot/aarch64-xilinx-linux/usr/include/xrt/ -O3 -g -Wall -c -fmessage-length=0 --sysroot=$(VITIS_PLATFORM_DIR)/sw/u96v2_sbc_base/PetaLinux/sysroot/aarch64-xilinx-linux -I./server
LDFLAGS += -lxilinxopencl -lpthread -lrt -ldl -lcrypt -lstdc++ \
-L$(VITIS_PLATFORM_DIR)/sw/u96v2_sbc_base/PetaLinux/sysroot/aarch64-xilinx-linux/usr/lib/ \
--sysroot=$(VITIS_PLATFORM_DIR)/sw/u96v2_sbc_base/PetaLinux/sysroot/aarch64-xilinx-linux

#
# Host files
#
CLIENT_SOURCES = Client/client.cpp
CLIENT_EXE = client

# Include all necessary source files for the encoder
SERVER_SOURCES = Server/encoder.cpp Server/server.cpp \
                 Server/cdc.cpp Server/sha256.cpp \
                 Server/deduplication.cpp Server/lzw.cpp \

SERVER_OBJECTS = $(SERVER_SOURCES:.cpp=.o)
SERVER_EXE = encoder

DECODER_SOURCES = Decoder/Decoder.cpp
DECODER_OBJECTS = $(DECODER_SOURCES:.cpp=.o)
DECODER_EXE = decoder

all: $(CLIENT_EXE) $(SERVER_EXE) $(DECODER_EXE)

$(CLIENT_EXE):
	g++ -O3 $(CLIENT_SOURCES) -o "$@"

$(SERVER_EXE): $(SERVER_OBJECTS)
	$(HOST_CXX) -o "$@" $(SERVER_OBJECTS) $(LDFLAGS)

$(DECODER_EXE): $(DECODER_OBJECTS)
	$(HOST_CXX) -o "$@" $(DECODER_OBJECTS) $(LDFLAGS)

# Compile .cpp files into .o files
%.o: %.cpp
	$(HOST_CXX) $(CXXFLAGS) -o "$@" "$<"

.NOTPARALLEL: clean

clean:
	-$(RM) $(SERVER_EXE) $(SERVER_OBJECTS) $(DECODER_EXE) $(DECODER_OBJECTS) $(CLIENT_EXE)
