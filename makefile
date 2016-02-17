#---------------------------------------------------------------------------------
# General-purpose makefile for Psy-Q PlayStation projects by Lameguy64
# 2016 Meido-Tek Productions

# Supports compiling C and MIP assembly source files as well as disc image
# creation for convenience (requires Dosbox if you're using a 64-bit version of windows).

# NOTE: Do not use PSYMAKE that came with the Psy-Q SDK to execute this makefile!
# Use MSys' make and its utilities cp (copy) and rm (remove/delete).
# http://www.mingw.org/wiki/msys

#---------------------------------------------------------------------------------

# Target name (or project name) for compiled executable and iso image file
TARGET		= psxnet

# File name of CTI script file (optional, required for iso creation)
CTIFILE		=

# Source directories
# (can be blank if your source files are in the same directory as the makefile)
SOURCES		=

# C compiler flags
CFLAGS		= -O3 -Wall

# Executable load address (0x80010000 is the standard load address)
PROGADDR	= 0x80010000

#---------------------------------------------------------------------------------

# Executable names of the C compiler and assembler
CC			= ccpsx
ASM			= asmpsx

# DOS emulator path (starting from the Program Files directory)
DOSEMU		= DOSBox-0.74\DOSBox.exe

# Search directories for C and MIP files
CFILES		= $(notdir $(wildcard ./*.c)) $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
AFILES		= $(notdir $(wildcard ./*.mip)) $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.mip))

# Generate file names for the object files
OFILES		= $(CFILES:.c=.obj) $(AFILES:.mip=.obj)

#---------------------------------------------------------------------------------

# Default target, compiles all C and MIP source files found
all: $(OFILES)
	$(CC) -Xo$(PROGADDR) $(CFLAGS) $(OFILES) -o $(TARGET).cpe
	cpe2x $(TARGET).cpe

# Compile C source
%.obj: %.c
	rm -f $(TARGET).cpe
	$(CC) $(CFLAGS) -c $< -o $@

# Compile MIP assembler source
# For some reason, asmpsx would not take program arguments properly when
# executed through GNU Make. As a workaround, a temporary batch file had to be
# created so that asmpsx can take the arguments properly.
%.obj: %.mip
	rm -f $(TARGET).cpe
	@echo $(ASM) /l $<,$@ >_asm.bat
	@echo exit >>_asm.bat
	cmd < _asm.bat
	rm -f _asm.bat

#---------------------------------------------------------------------------------

# Build iso image
# Produces buildcd.log on x64 platforms
iso:
	rm -f $(TARGET).img
	cp -f \psyq\cdgen\lcnsfile\licensea.dat licensea.dat
ifdef PROGRAMW6432
	cp -f \psyq\bin\buildcd.exe _buildcd.exe
	@echo @echo off >_buildcd.bat
	@echo cls >>_buildcd.bat
	@echo echo Image file is being created... >>_buildcd.bat
	@echo _buildcd -l $(CTIFILE) -i$(TARGET).img \>buildcd.log >>_buildcd.bat
	@echo exit >>_buildcd.bat
	@"$(PROGRAMFILES)\$(DOSEMU)" _buildcd.bat
	rm -f _buildcd.exe
else
	buildcd -l $(CTIFILE) -i$(TARGET).img
endif
	rm -f CDW900E.TOC
	rm -f QSHEET.TOC
	@echo stripiso s 2352 $(TARGET).img $(TARGET).iso >_buildcd.bat
	@echo exit >>_buildcd.bat
	@cmd < _buildcd.bat
	rm -f $(TARGET).img
	rm -f _buildcd.bat
	rm -f licensea.dat

#---------------------------------------------------------------------------------

# Clean target, delete all object files, target executable and disc image
clean:
	rm -f $(OFILES) $(TARGET).cpe $(TARGET).exe $(TARGET).iso
