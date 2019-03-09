############################## Config Module ###################################
CONFIG_DIR = Source/Files/Config
CONFIG_IMPL_DIR := $(CONFIG_DIR)/Implementation
CONFIG_TEST_DIR = Tests/Files/Config

CONFIG_PREFIX = Config_
CONFIG_OBJ := $(JUCE_OBJDIR)/$(CONFIG_PREFIX)

OBJECTS_CONFIG_IMPL := \
  $(CONFIG_OBJ)AlertWindow.o \
  $(CONFIG_OBJ)MainResource.o

OBJECTS_CONFIG := \
  $(OBJECTS_CONFIG_IMPL) \
  $(CONFIG_OBJ)FileResource.o \
  $(CONFIG_OBJ)DataKey.o \
  $(CONFIG_OBJ)MainFile.o \
  $(CONFIG_OBJ)MainListener.o

CONFIG_TEST_PREFIX := $(CONFIG_PREFIX)Test_
CONFIG_TEST_OBJ := $(CONFIG_OBJ)Test_
OBJECTS_CONFIG_TEST :=

ifeq ($(BUILD_TESTS), 1)
    OBJECTS_CONFIG := $(OBJECTS_CONFIG) $(OBJECTS_CONFIG_TEST)
endif

MODULES := $(MODULES) config

OBJECTS_APP := $(OBJECTS_APP) $(OBJECTS_CONFIG)

config : $(OBJECTS_CONFIG)
	@echo "Built Config module"

$(CONFIG_OBJ)AlertWindow.o: \
    $(CONFIG_IMPL_DIR)/$(CONFIG_PREFIX)AlertWindow.cpp
$(CONFIG_OBJ)MainResource.o: \
    $(CONFIG_IMPL_DIR)/$(CONFIG_PREFIX)MainResource.cpp
$(CONFIG_OBJ)FileResource.o: \
    $(CONFIG_DIR)/$(CONFIG_PREFIX)FileResource.cpp
$(CONFIG_OBJ)DataKey.o: \
    $(CONFIG_DIR)/$(CONFIG_PREFIX)DataKey.cpp
$(CONFIG_OBJ)MainFile.o: \
    $(CONFIG_DIR)/$(CONFIG_PREFIX)MainFile.cpp
$(CONFIG_OBJ)MainListener.o: \
    $(CONFIG_DIR)/$(CONFIG_PREFIX)MainListener.cpp
