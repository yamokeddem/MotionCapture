SHELL       = /bin/sh

# default config:
#    release
#    x86
#    shared
#    clang
#    
#    to build 64 bit:                make -e M64=-m64
#    to build static:                make -e STATIC_LINK=1
#    to build 64 bit static debug:   make -e M64=-m64 STATIC_LINK=1 VARIANT=debug
#    to build using gcc:             make -e GCC_COMPILER=1
#


VARIANT = release
ifeq "$(VARIANT)" "debug"
	GCC_DEBUG_FLAGS += -g -D_DEBUG
endif 


# Don't leave M64 empty on Mac OS 10.5 and above, the default link architecture 
# is X86_64 but the GLEW lib is only compiled for either X86 or X64 and will
# fail to link
M64 = -m64
ARCH = x86
ifeq "$(M64)" "-m64"
    ARCH=x64
endif

EXAMPLE     = MotionCapture
PROGNAME = Motion Capture Kinect
VERSION = 1.0
BINDIR      = .
##### Mettre le chemin vers les include et les libs du sdk
LIBDIR = /Applications/Autodesk/FBXSDK/2015.1/lib/clang/release
LIBDIR += $(OPENNI2_REDIST)
LOG4CXXLIB = /usr/local/Cellar/log4cxx/0.10.0_1/lib


INCDIR = /Applications/Autodesk/FBXSDK/2015.1/include
INC = ./NiTE2
LOG4CDIR = /usr/local/include/log4cxx
FBXDIR = ./FBXLoader
GL4DUDIR = ./FBXLoader/GL4D
OPENNI2DIR = $(OPENNI2_INCLUDE)

NITEDIR = ./Include
COMMONDIR = FBXLoader/Common

##### Fin des modifs a effectuer

TARGET      = $(BINDIR)/$(EXAMPLE)

OBJS  = $(COMMONDIR)/Common.o\
	./FBXLoader/DrawScene.o\
	./FBXLoader/GetPosition.o\
	./FBXLoader/SceneCache.o\
	./FBXLoader/SceneContext.o\
	./FBXLoader/glimage.o\
	./Src/Game.o\
	./Src/Skybox.o\
	./Src/Menu.o\
	./Src/Device.o\
	./Src/Mocap.o\
	./Src/SoundEffect.o\
	./Src/CalibrationMode.o\
	./Src/Window.o

CP          = cp -f
RM          = /bin/rm -rf


#add by Vince
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	CC = cc 
	LD = cc 
	STDC = c++
	COMPILER = clang
	MAC_CFLAGS += -mmacosx-version-min=$(MACOSX_DEPLOYMENT_TARGET)
	MAC_LDFLAGS +=  -mmacosx-version-min=$(MACOSX_DEPLOYMENT_TARGET)
	COPY_LIB = $(CP) ./FBXLoader/Object/libfbxsdk.dylib $(BINDIR)   
else
	COMPILER = gcc4
	CC       = gcc
	LD       = gcc
	STDC     = c++
	COPY_LIB = $(CP) $(LIBDIR)/libfbxsdk.so $(BINDIR)
endif	

SDL_CFLAGS = $(shell sdl2-config --cflags) 
SDL_LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image
#end

CXXFLAGS    = -w -pipe $(M64) $(GCC_DEBUG_FLAGS)  $(SDL_CFLAGS) $(OPENNI2_INCLUDE) -IGL4D 
CFLAGS      = $(CXXFLAGS)
LDFLAGS     = $(M64) $(SDL_LDFLAGS) -lGL4Dummies 

OPENNI_DIRS += $(OPENNI2_REDIST) 


STATIC_LINK  =
FBXSDK_LIB   = -lfbxsdk
ifeq "$(STATIC_LINK)" ""
    CXXFLAGS += -DFBXSDK_SHARED
else
    FBXSDK_LIB = $(LIBDIR)/libfbxsdk.a
endif

ifeq ($(UNAME), Darwin)
	LIBS = $(FBXSDK_LIB) -lm -l$(STDC) -liconv -fexceptions -lz -framework GLUT -framework OpenGL -lOpenNI2 -lNiTE2 -llog4cxx -lcppunit -lSDL2_mixer -framework Cocoa -framework SystemConfiguration
else
	LIBS = $(FBXSDK_LIB) -lm -lrt -lc++ -lpthread -ldl $(EXTRA_LIBS) -lGLU -lGL -llog4cxx -lglut -lcppunit -lSDL2_mixer
endif

.SUFFIXES: .cpp

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD)  -o $@ $(OBJS) -L$(LIBDIR) -L$(LOG4CXXLIB) -L. $(LIBS) $(LDFLAGS)
	$(COPY_LIB)

%.o: %.c
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(INC) -I$(NITEDIR) -I$(FBXDIR) $(OPENNI2_INCLUDE)  -c $< -o $@ 

%.o: %.cpp
	$(CC) $(CXXFLAGS) -I$(INCDIR) -I$(INC) -I$(NITEDIR) -I$(FBXDIR) $(OPENNI2_INCLUDE)  -c $< -o $@ 

clean:
	$(RM) $(OBJS) MotionCapture

doc: $(DOXYFILE)
	cd Documentation && doxygen && cd ..
