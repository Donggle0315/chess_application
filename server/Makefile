# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g
DEPFLAGS = -MMD -MP

# Directories
SRCDIR = src
INCDIR = include
BUILDDIR = build
TARGETDIR = bin

# Source files
SRCS := $(shell find $(SRCDIR) -name '*.c')

# Object files
OBJS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRCS:.c=.o))

# Target executable
TARGET = $(TARGETDIR)/server

# Include directories
INC := -I$(INCDIR)

# Get all subdirectories of ../include
INC_SUBDIRS := $(shell find $(INCDIR) -type d)

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJS) | $(TARGETDIR)
	$(CC) $(CFLAGS) $(INC) -o $@ $^

# Rule to compile source files into object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(addprefix -I,$(INC_SUBDIRS)) -c -o $@ $<

# Include automatically generated dependency files
-include $(OBJS:.o=.d)


# Directories
$(BUILDDIR):
	mkdir $(BUILDDIR)

$(TARGETDIR): 
	mkdir $(TARGETDIR)

# Clean rule
clean:
	rm -r $(BUILDDIR) $(TARGETDIR)

.PHONY: all clean
