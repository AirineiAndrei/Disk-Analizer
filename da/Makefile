CC := gcc
CFLAGS := -pthread
LINKER = gcc

SRCDIR   = .
OBJDIR   = ./obj
BINDIR   = ./bin
SHAREDDIR = ./../Shared

TARGET   = da

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
SHAREDOBJECTS := $(wildcard $(SHAREDDIR)/obj/*.o)

rm		 = rm -fr

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $(OBJECTS) $(SHAREDOBJECTS) $(LFLAGS) -o $@
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
	$(MAKE) -C $(SHAREDDIR) $@
	@$(rm) $(OBJDIR)
	@echo "Executable removed!"

.PHONY: all
all: 
	mkdir -p bin
	mkdir -p obj
	$(MAKE) -C $(SHAREDDIR) $@ # compile shared files
	make $(BINDIR)/$(TARGET)
