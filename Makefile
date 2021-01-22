##############################################################################
# File: Generic makefile for arm c/c++ Program
# Author :  athina
# Date: 2020-06-20 (v1.0, original version)
# Usage:
# 		Make the Yolov3 Demo
##############################################################################
# gcc compiler
CC = arm-himix200-linux-gcc

# g++ compiler
CXX = arm-himix200-linux-g++

# program name
PROGRAM	=yolo_demo
# source directory
SRCDIRS	=./src
# header directory
HDRDIRS	=./inc ./include ./third
# binary directory
BINDIRS	=./bin
# library directory
LIBDIRS	=./lib ./third/libopencv

##############################################################################
# empty character
EMPTY	=

# gcc option
CFLAGS =-g -Wall -O0 
CFLAGS	+=$(addprefix -I,$(HDRDIRS))

# g++ option
CXXFLAGS=$(CFLAGS) -std=c++11 

# Link flags
LDFLAGS	=  $(addprefix -L,$(LIBDIRS)) -Wl,-z,relro,-z,noexecstack,--disable-new-dtags,-rpath,/lib/:/usr/lib/:/usr/app/lib
LDFLAGS += -lrt -lm -lsecurec -lpthread -Wformat-extra-args

# dependence flag
DEPFLAGS=-MM -MP

# source extended
SRCEXTS	=.c .cpp
# header extended
HDREXTS	=.h

# source files 
SOURCE	=$(foreach d, $(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
# head files
HEADER	=$(foreach d, $(HDRDIRS),$(wildcard $(addprefix $(d)/*,$(HDREXTS))))
# object files
OBJECT	=$(foreach d, $(basename $(notdir $(SOURCE))),$(BINDIRS)/$(d).o)
# dependance files
DEPEND	=$(patsubst %.o,%.d, $(OBJECT))

##############################################################################
# Stable, don't change
ifeq ($(PROGRAM), $(EMPTY))
	PROGRAM:=$(BINDIRS)/program
else
	PROGRAM:=$(BINDIRS)/$(PROGRAM)
endif

.PHONY: all object depend clean vclean show help
# target file
all:$(PROGRAM)
$(PROGRAM): $(OBJECT)
ifeq ($(filter-out %.c, $(SOURCE)), $(EMPTY))
ifneq ($(SOURCE), $(EMPTY))
	$(CC) -o $@ $(DBGFLAGS) $^ $(LDFLAGS)
endif
else
ifneq ($(SOURCE), $(EMPTY))
	$(CXX) -o $@ $(DBGFLAGS) $^ $(LDFLAGS)
endif
endif

ifndef NODEP
sinclude $(DEPEND)
endif

# object files
object:$(OBJECT)
$(BINDIRS)/%.o: $(SRCDIRS)/%.c
	$(CC) $(OPTFLAGS) $(CFLAGS) -o $@ -c $<
$(BINDIRS)/%.o: $(SRCDIRS)/%.cpp
	$(CXX) $(OPTFLAGS) $(CXXFLAGS) -o $@ -c $<

# dependance files
depend:$(DEPEND)
$(DEPEND): $(SOURCE)
$(BINDIRS)/%.d: $(SRCDIRS)/%.c
	@$(CC) $(CFLAGS) $(DEPFLAGS) $< | sed 's,\($*\).o[ :]*,$(BINDIRS)/\1.o $@ :,g' >$@
$(BINDIRS)/%.d: $(SRCDIRS)/%.cpp
	@$(CXX) $(CXXFLAGS) $(DEPFLAGS) $< | sed 's,\($*\).o[ :]*,$(BINDIRS)/\1.o $@ :,g' >$@

# clean the temporary files
clean:
	$(RM) $(OBJECT) $(DEPEND)

vclean:
	$(RM) $(OBJECT) $(DEPEND) $(PROGRAM)

show:
	@echo '[File list]:'
	@echo '	PROGRAM NAME	:$(PROGRAM)'
	@echo '	SOURCE FILES	:$(SOURCE)'
	@echo '	HEADER FILES	:$(HEADER)'
	@echo '	OBJECT FILES	:$(OBJECT)'
	@echo '	DEPEND FILES	:$(DEPEND)'

help:
	@echo 'Generic Makefile for C/C++ Programs (gcmakefile) version 2.1'
	@echo '[Usage]: make[TARGET]'
	@echo '	all	:(=make)compile and link'
	@echo '	object	:compile only (not linking)'
	@echo '	depend	:(NODEP=yes)[do|donot] generate the dependencies(not compiling)'
	@echo '	clean	:clean up the object files and the dependence files'
	@echo '	vclean	:clean up all of the generated files(including executable file)'
	@echo '	show	:show the hierarchical of the files list'
	@echo '	help	:print how to use the makefile manual'
