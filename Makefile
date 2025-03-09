#Architecture can be [x64]
Platform = Windows
Configuration = Release
TargetName = LLHttp
Architecture = x64

OUTPUT_DIR = bin/$(Configuration)-$(Platform)/
INT_DIR = bin-int/$(Configuration)-$(Platform)/

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
	$(MF) cert/
	$(MF) $(INT_DIR)
	$(MF) $(OUTPUT_DIR)
build_pch: make_folders
	$(CC) $(CFlags) $(Defines) $(IncludeDirs) /Ycpch.h /Fp$(INT_DIR)pch.pch src/$(Type)/pch.cpp
build: make_folders
	$(CC) $(Files) $(CFlags) $(Defines) $(IncludeDirs) /Yupch.h /Fp$(INT_DIR)pch.pch 
	$(LK) $(LFlags) $(LibDirs) $(Libs)
run:
ifneq ($(Configuration), Dist)
	gdb -ex run -ex quit -ex "set args $(ProgramArgs)" $(OUTPUT_DIR)$(TargetName).exe
else
	$(OUTPUT_DIR)$(TargetName).exe
endif

buildnrun: build run
rebuild: make_folders build_pch build
rebuildnrun:make_folders build_pch build run

prepare_test_server:
	npm install --prefix test/httptest fs express
run_test_server:
	node test/httptest/server.js