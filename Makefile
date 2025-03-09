#Architecture can be [x64]
Platform = Windows
Configuration = Release
TargetName = LLHttp
Architecture = x64

OUTPUT_DIR = bin/$(Configuration)/$(Architecture)-$(Platform)/
INT_DIR = bin-int/$(Configuration)/$(Architecture)-$(Platform)/
Include = include/

MF =
RF = 
CC = 
LK = 
Files =
Defines =
CFlags = 
LFlags = 
Files = 
SRC_DIR = src/

BoostDir = ${BOOST_ROOT}
HBUFFER_LIB_SRC = libs/HBuffer/

ifeq ($(Platform), Windows)
include WindowsMake.mk
else
$(error Invalid Platform)
endif

default: build

clean:
	$(RF) bin-int
make_folders:
	$(MF) $(INT_DIR)
	$(MF) $(OUTPUT_DIR)
build_pch: make_folders
	$(CC) $(CFlags) $(Defines) $(IncludeDirs) /YcLLHttp/pch.h /Fp$(INT_DIR)pch.pch src/pch.cpp
build: make_folders
	$(CC) $(Files) $(CFlags) $(Defines) $(IncludeDirs) /YuLLHttp/pch.h /Fp$(INT_DIR)pch.pch 
	$(LK) $(LFlags) $(LibDirs) $(Libs)

buildnrun: build run
rebuild: make_folders build_pch build
rebuildnrun:make_folders build_pch build run
