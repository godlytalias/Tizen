################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs
-include makefile.sa

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: tizen_silent_service secondary-outputs

# Tool invocations
tizen_silent_service: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: C++ Linker'
	$(CXX) -o"tizen_silent_service" $(OBJS) $(USER_OBJS) $(LIBS) $(TC_LINKER_MISC) -pie -lpthread -Xlinker -rpath="/home/developer/sdk_tools/lib" --sysroot="$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core" -Xlinker --version-script=$(PROJ_PATH)/.exportMap -L"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/lib" $(RS_LIBRARIES) -Xlinker -rpath="/opt/usr/apps/$(APPID)/lib" -Xlinker -rpath="/opt/usr/apps/mwtpyWDgYs/lib" -Werror-implicit-function-declaration
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean: sa_clean
	-$(RM) $(MO_FILES)$(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(AST)$(FNMAP)$(EDJ_FILES)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) tizen_silent_service
	-@echo ' '

secondary-outputs: $(MO_FILES) $(EDJ_FILES)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
