CXX = g++
TARGET = RconCLI

# Directories.
IDIR = "C:\Program Files\OpenSSL-Win64\include"
SUBDIR = http json sockets sos
BUILD_DIR = build
SUB_BUILD_DIR = $(addprefix $(BUILD_DIR)\, $(SUBDIR))

# Detect the OS for a proper clean rule; this is because I'm using MSYS on Windows.
ifeq '$(findstring ;,$(PATH))' ';'
detected_OS := Windows
else
detected_OS := $(shell uname 2>/dev/null || echo Unknown)
detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
endif

# Create subdirectories to build object files into.
ifeq ($(detected_OS),Windows)
$(foreach d, $(SUB_BUILD_DIR), $(shell md $(d)))
else 
$(foreach d, $(SUB_BUILD_DIR), $(shell mkdir -p $(d)))
endif

# Get current directory from root for the needed include.
MPWD := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

# GCC flags.
CXXFLAGS = -I$(IDIR) -I$(MPWD) -g
LDFLAGS = -lws2_32 -lcrypto -static-libgcc -static-libstdc++ 

# Source files.
SOURCE = $(wildcard RconCLI/*.cpp) $(wildcard RconCLI/http/*.cpp) $(wildcard RconCLI/json/*.cpp) $(wildcard RconCLI/sockets/*.cpp) $(wildcard RconCLI/sos/*.cpp)
INCLUDE = $(wildcard RconCLI/*.h) $(wildcard RconCLI/http/*.h) $(wildcard RconCLI/json/*.h) $(wildcard RconCLI/sockets/*.h) $(wildcard RconCLI/sos/*.h)

# Object files to be built.
_OBJ = $(SOURCE:%.cpp=%.o)
OBJ = $(subst RconCLI,build,$(_OBJ))

# Rules.
all : $(TARGET)

$(TARGET) : $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)

$(OBJ) : $(SOURCE) $(INCLUDE)
	$(CXX) $(CXXFLAGS) -c $(subst build,RconCLI,$(subst .o,.cpp,$@)) -o $@

# Clean rule.
.PHONY : clean
clean :
ifeq ($(detected_OS),Windows)
	rmdir /q /s $(BUILD_DIR)
	del /s /q $(TARGET).exe
else 
	rm -f -r $(BUILD_DIR) $(TARGET).exe
endif