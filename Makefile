#Architecture can be [x64]
Platform = Windows
Configuration = Release
TargetName = LLHttp
Architecture = x64

OutputDir = bin/$(Configuration)/$(Architecture)-$(Platform)/
IntDir = bin-int/$(Configuration)/$(Architecture)-$(Platform)/
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
	$(MF) $(IntDir)
	$(MF) $(OutputDir)
build_pch: make_folders
	$(CC) $(CFlags) $(Defines) $(IncludeDirs) /YcLLHttp/pch.h /Fp$(IntDir)pch.pch src/pch.cpp
build: make_folders
	$(CC) $(Files) $(CFlags) $(Defines) $(IncludeDirs) /YuLLHttp/pch.h /Fp$(IntDir)pch.pch 
	$(LK) $(LFlags) $(LibDirs) $(Libs)

buildnrun: build run
rebuild: make_folders build_pch build
rebuildnrun:make_folders build_pch build run
