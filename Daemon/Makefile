CC := gcc
CFLAGS :=
LINKER = gcc

SRCDIR   = .
OBJDIR   = ./obj
BINDIR   = ./bin

TARGET   = daemon

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

rm		 = rm -fr

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $(OBJECTS) $(LFLAGS) -o $@
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	@$(rm) $(BINDIR)
	@echo "Cleanup complete!"

.PHONY: remove
remove: clean
	@$(rm) $(OBJDIR)
	@echo "Executable removed!"

.PHONY: all
all: 
	mkdir -p bin
	mkdir -p obj
	make $(BINDIR)/$(TARGET)
