############################## Config Module ###################################
CONFIG_PREFIX := $(JUCE_OBJDIR)/Config_
CONFIG_ROOT = Source/Config
CONFIG_TEST_ROOT = Tests/Config

OBJECTS_CONFIG := \
  $(CONFIG_PREFIX)FileResource.o \
  $(CONFIG_PREFIX)DataKey.o \
  $(CONFIG_PREFIX)AlertWindow.o \
  $(CONFIG_PREFIX)MainResource.o \
  $(CONFIG_PREFIX)MainFile.o \
  $(CONFIG_PREFIX)MainListener.o

OBJECTS_CONFIG_TEST :=

ifeq ($(BUILD_TESTS), 1)
    OBJECTS_CONFIG := $(OBJECTS_CONFIG) $(OBJECTS_CONFIG_TEST)
endif

MODULES := $(MODULES) config

OBJECTS_APP := $(OBJECTS_APP) $(OBJECTS_CONFIG)

config : $(OBJECTS_CONFIG)
	@echo "Built Config module"

$(CONFIG_PREFIX)FileResource.o: \
    $(CONFIG_ROOT)/Config_FileResource.cpp
$(CONFIG_PREFIX)DataKey.o: \
    $(CONFIG_ROOT)/Config_DataKey.cpp
$(CONFIG_PREFIX)AlertWindow.o: \
    $(CONFIG_ROOT)/Implementation/Config_AlertWindow.cpp
$(CONFIG_PREFIX)MainResource.o: \
    $(CONFIG_ROOT)/Implementation/Config_MainResource.cpp
$(CONFIG_PREFIX)MainFile.o: \
    $(CONFIG_ROOT)/Config_MainFile.cpp
$(CONFIG_PREFIX)MainListener.o: \
    $(CONFIG_ROOT)/Config_MainListener.cpp
