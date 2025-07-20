# Search for all folder in modules with a Makefile --> The modules
MODULE_DIRS := $(shell find . modules -mindepth 1 -maxdepth 1 -type d -exec test -f '{}/Makefile' ';' -print | sed 's|^\./||')

BUILD_DIR := build
BUILD_MODULES_DIR := $(BUILD_DIR)/modules

all:
	@echo "Building modules: $(MODULE_DIRS)"
	@for dir in $(MODULE_DIRS); do \
		$(MAKE) -C $$dir || exit 1; \
	done

	@echo "Copying built files..."
	@mkdir -p $(BUILD_MODULES_DIR)
	@for dir in $(MODULE_DIRS); do \
		if [ -d $$dir/bin ]; then \
			for file in $$dir/bin/*; do \
				if [ -f $$file ]; then \
					case $$file in \
						*.so) cp $$file $(BUILD_MODULES_DIR)/ ;; \
						*) cp $$file $(BUILD_DIR)/ ;; \
					esac \
				fi \
			done \
		fi \
	done

clean:
	@echo "Cleaning modules: $(MODULE_DIRS)"
	@for dir in $(MODULE_DIRS); do \
		$(MAKE) -C $$dir clean || exit 1; \
	done

	@echo "Cleaning build directory"
	@if [ -d $(BUILD_DIR) ]; then \
		find $(BUILD_DIR) -mindepth 1 ! -name 'config.json' -exec rm -rf {} +; \
	fi

.PHONY: all clean