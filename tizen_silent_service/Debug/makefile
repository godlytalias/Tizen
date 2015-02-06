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
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: tizen_silent_service secondary-outputs

# Tool invocations
tizen_silent_service: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: C++ Linker'
	$(CXX) -o"tizen_silent_service" $(OBJS) $(USER_OBJS) $(LIBS) -L"$(PROJ_PATH)/lib" $(TC_LINKER_MISC) -pie -lpthread -Xlinker -rpath="/home/developer/sdk_tools/lib" --sysroot="$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core" -L"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/lib" $(RS_LIBRARIES) -Xlinker -rpath="/opt/usr/apps/$(APPID)/lib" -Xlinker -rpath="/opt/usr/apps/mwtpyWDgYs/lib" -Werror-implicit-function-declaration
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(AST)$(CC_DEPS)$(C++_DEPS)$(MO_FILES)$(EXECUTABLES)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(FNMAP)$(EDJ_FILES)$(CPP_DEPS)$(C_DEPS) tizen_silent_service
	-@echo ' '

secondary-outputs: $(MO_FILES) $(EDJ_FILES)

sa_all: all sa

sa: $(AST) $(FNMAP)
	-$(RM) $(SA_REPORT_PATH)/externalFnMap.txt
	-$(RM) $(SA_REPORT_PATH)/report-*.xml
	$(foreach fn,$(FNMAP),cat $(fn) >> $(SA_REPORT_PATH)/externalFnMap.txt ; )
	$(foreach fn,$(C_SRCS)$(CPP_SRCS),tizenSA -analyzer-output=$(SA_REPORT_TYPE) -o $(SA_REPORT_PATH) $(fn) -I"pch" -D_DEBUG -I"$(PROJ_PATH)/inc" -O0 -g3 -Wall -c -fmessage-length=0 $(TC_COMPILER_MISC) -fPIE --sysroot="$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core" -Xlinker --version-script=$(PROJ_PATH)/.exportMap -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/libxml2" -I"$(SDK_PATH)/library" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/AL" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/appcore-agent" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/appfw" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/base" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/cairo" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/calendar-service2" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/ckm" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/contacts-svc" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/content" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/curl" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/dbus-1.0" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/lib/dbus-1.0/include" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/dlog" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/ecore-1" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/e_dbus-1" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/edje-1" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/eet-1" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/efreet-1" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/eina-1/eina" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/eina-1" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/elementary-1" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/ethumb-1" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/evas-1" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/fontconfig" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/freetype2" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/gio-unix-2.0" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/glib-2.0" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/lib/glib-2.0/include" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/json-glib-1.0" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/json-glib-1.0/json-glib" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/libexif" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/media-content" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/media" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/minizip" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/network" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/notification" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/shortcut" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/storage" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/system" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/ui" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/vconf" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/web" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/EGL" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/badge" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/eio-1" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/email-service" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/embryo-1" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/emotion-1" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/GL" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/GLES" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/GLES2" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/GLES3" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/KHR" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/messaging" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/msg-service" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/ug-1" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/context-manager" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/telephony" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/telephony-client" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/ewebkit2-0" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/location" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/sensor" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/efl-extension" -I"$(SDK_PATH)/tools/smart-build-interface/../../platforms/mobile-2.3/rootstraps/mobile-2.3-emulator.core/usr/include/wifi-direct" -Werror-implicit-function-declaration -MMD -MP -MF"$(@:%.c.fn=%.d)" -MT"$(@:%.c.fn=%.d)" ; )

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
