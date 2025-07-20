# Trouve tous les dossiers contenant un Makefile (les modules)
MODULE_DIRS := $(shell find . -mindepth 1 -maxdepth 1 -type d -exec test -f '{}/Makefile' ';' -print | sed 's|^\./||')

all:
	@echo "Building modules: $(MODULE_DIRS)"
	@for dir in $(MODULE_DIRS); do \
		$(MAKE) -C $$dir || exit 1; \
	done

clean:
	@echo "Cleaning modules: $(MODULE_DIRS)"
	@for dir in $(MODULE_DIRS); do \
		$(MAKE) -C $$dir clean || exit 1; \
	done

.PHONY: all clean
