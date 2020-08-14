# Names of the compiler and friends
APP_BASE = /opt/toolchains/x68k/bin/human68k
AS 		= $(APP_BASE)-as
CC 		= $(APP_BASE)-gcc
LD 		= $(APP_BASE)-ld
OBJCOPY	= $(APP_BASE)-objcopy
STRIP 	= $(APP_BASE)-strip

# Where to copy the output exe files to
DEST	= "/export/Games/Computer Games/X68000/Shared/launcher"

# libraries and paths
LIBS	 	= -ldos -lm
GFXLIBS		= $(LIBS)
TEXTLIBS	= $(LIBS) 
INCLUDES 	=  -I./src

# Compiler flags
ASM_FLAGS 	= -m68000 -mtune=68000 --register-prefx-optional
LDFLAGS 	=
CFLAGS 		= -m68000 -mtune=68000 -std=c99 -fomit-frame-pointer -Wall -Wno-unused-function -Wno-unused-variable -O3
LDSCRIPT 	=
OCFLAGS	= -O xfile

# What our application is named
TARGET			= launcher
EXE				= $(TARGET).X
GFXTARGET		= gfxtest
GFXEXE			= $(GFXTARGET).X
TEXTTARGET		= texttest
TEXTEXE			= $(TEXTTARGET).X

all: $(EXE) #$(GFXEXE) $(TEXTEXE)

gfx: $(GFXEXE)

text: $(TEXTEXE)

# The main application
OBJFILES = build/exnfiles.o build/exfiles.o build/nfiles.o build/files.o \
	build/utils.o build/fstools.o build/data.o build/ini.o build/gfx.o \
	build/ui.o build/bmp.o build/main.o build/textgfx.o build/input.o

# GFX demo test application
GFXOBJFILES = build/gfxdemo.o build/utils.o build/gfxtest.o build/gfx.o build/bmp.o

# Text GFX demo test application
TEXTOBJFILES = build/texttest.o build/textgfx.o build/bmp.o build/utils.o build/gfx.o

$(EXE):  $(OBJFILES)
	@echo ""
	@echo "========================================"
	@echo " -= $(EXE) =-"
	@echo ""
	@echo Linking ....
	$(CC) $(LDFLAGS) $(OBJFILES) $(LIBS) -o bin/$(TARGET)
	@echo ""
	@echo Dumping executable object ....
	$(OBJCOPY) $(OCFLAGS) bin/$(TARGET) bin/$(EXE)
	@echo ""
	@echo Copying application binary to X68000 shared folder ....
	cp -v bin/$(EXE) $(DEST) 

$(GFXEXE):  $(GFXOBJFILES)
	@echo ""
	@echo "========================================"
	@echo " -= $(GFXEXE) =-"
	@echo ""
	@echo Linking ....
	$(CC) $(LDFLAGS) $(GFXOBJFILES) $(GFXLIBS) -o bin/$(GFXTARGET)
	@echo ""
	@echo Dumping executable object ....
	$(OBJCOPY) $(OCFLAGS) bin/$(GFXTARGET) bin/$(GFXEXE)
	@echo ""
	@echo Copying GFX test binary to X68000 shared folder ....
	cp -v bin/$(GFXEXE) $(DEST) 
	
$(TEXTEXE):  $(TEXTOBJFILES)
	@echo ""
	@echo "========================================"
	@echo " -= $(TEXTEXE) =-"
	@echo ""
	@echo Linking ....
	$(CC) $(LDFLAGS) $(TEXTOBJFILES) $(TEXTLIBS) -o bin/$(TEXTTARGET)
	@echo ""
	@echo Dumping executable object ....
	$(OBJCOPY) $(OCFLAGS) bin/$(TEXTTARGET) bin/$(TEXTEXE)
	@echo ""
	@echo Copying Text GFX test binary to X68000 shared folder ....
	cp -v bin/$(TEXTEXE) $(DEST) 
	
	
################################
#
# Assembly stuff
#
################################
build/exfiles.o: asm/exfiles.S
	$(AS) $(ASMFLAGS) -c $< -o build/exfiles.o

build/exnfiles.o: asm/exnfiles.S
	$(AS) $(ASMFLAGS) -c $< -o build/exnfiles.o
	
build/files.o: asm/files.S
	$(AS) $(ASMFLAGS) -c $< -o build/files.o

build/nfiles.o: asm/nfiles.S
	$(AS) $(ASMFLAGS) -c $< -o build/nfiles.o

################################
#
# Main code
#
################################
build/bmp.o: src/bmp.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o build/bmp.o

build/data.o: src/data.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o build/data.o
	
build/fstools.o: src/fstools.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o build/fstools.o
	
build/gfx.o: src/gfx.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o build/gfx.o

build/gfxdemo.o: src/gfxdemo.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o build/gfxdemo.o
	
build/gfxtest.o: src/gfxtest.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o build/gfxtest.o

build/ini.o: src/ini.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o build/ini.o
	
build/input.o: src/input.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o build/input.o

build/main.o: src/main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o build/main.o
	
build/textgfx.o: src/textgfx.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o build/textgfx.o

build/texttest.o: src/texttest.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o build/texttest.o
	
build/ui.o: src/ui.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o build/ui.o	
	
build/utils.o: src/utils.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o build/utils.o

###############################
#
# Clean up
#
###############################
clean:
	rm -f build/*.o bin/$(EXE) bin/$(TARGET)
