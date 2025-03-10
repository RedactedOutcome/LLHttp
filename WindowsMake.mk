MF 		= mkdir -p
RF      = rd /s /q
CC = cl
LK = lib
CFlags = /c /Fo$(INT_DIR) /std:c++17 /MT /utf-8
Defines = /DPLATFORM=Windows /DWINDOWS_IGNORE_PACKING_MISMATCH /DLLHTTP_USE_PCH /DLLHTTP_PCH_DIR="\"LLHttp/pch.h\""
IncludeDirs = /I"$(HBUFFER_LIB_SRC)/include/" /I"libs/gzip-hpp/include/" /I"include/" /I"include/LLHttp/"
LibDirs = 
Libs = GDI32.lib Shell32.lib kernel32.lib User32.lib
LFlags = $(INT_DIR)*.obj /out:$(OUTPUT_DIR)$(TargetName).lib
ProgramArgs = 

#Debugging
#Configuration Only
ifeq ($(Configuration), Debug)
CFlags += /Od /Z7
Defines += /DDEBUG
LFlags += /DEBUG
else
Defines += /DNDEBUG

ifeq ($(Configuration), Release)
Defines += /DWEB_RELEASE
CFlags += /Ot /Oi
LFlags += /LTCG /INCREMENTAL:NO /NODEFAULTLIB /Gy
else
CFlags += /Ot /Oi /O2 /GL /Gw
Defines += /DWEB_DIST
LFlags += /LTCG /INCREMENTAL:NO /NODEFAULTLIB /OPT:REF /OPT:ICF /Gy
endif
endif

CORE_DIR = $(SRC_DIR)Core/

#Libs
#Files += $(SRC_DIR)libs/HBuffer.cpp
#Files += $(SRC_DIR)libs/HBufferJoin.cpp

Files+= $(SRC_DIR)LLHttp.cpp
Files+= $(SRC_DIR)Cookie.cpp
Files+= $(SRC_DIR)HttpRequest.cpp
Files+= $(SRC_DIR)HttpResponse.cpp
Files+= $(SRC_DIR)Decoder.cpp
Files+= $(SRC_DIR)ParsedURL.cpp

ifneq (${VCPKG_ROOT},)
IncludeDirs += /I"${VCPKG_ROOT}\installed\$(Architecture)-windows\include" /I"${VCPKG_ROOT}\installed\$(Architecture)-windows-static\include"
LibDirs += /LIBPATH:"${VCPKG_LIB_DIR}" /LIBPATH:"${VCPKG_ROOT}\installed\$(Architecture)-windows-static\lib"
Libs += zlib.lib

endif