CC := gcc
CFLAGS := -pthread
LINKER = gcc

SRCDIR   = .
OBJDIR   = ./obj


SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

rm		 = rm -fr

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"


.PHONY: remove
remove: 
	@$(rm) $(OBJDIR)
	@echo "Shared objects removed!"

.PHONY: all
all: 
	mkdir -p obj
	make $(OBJECTS)