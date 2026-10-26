LIB_SOURCES := $(filter-out src/main.c src/benchmarks.c,$(wildcard src/*.c))
LIB_OBJECTS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(LIB_SOURCES))
APP_OBJECTS := $(BUILD_DIR)/main.o $(BUILD_DIR)/benchmarks.o

$(TARGET): $(LIB_OBJECTS) $(APP_OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(LIB_OBJECTS:.o=.d) $(APP_OBJECTS:.o=.d)
