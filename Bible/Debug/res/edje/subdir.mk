################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
EDC_SRCS += \
../res/edje/bible.edc 

EDJ_FILES += \
./res/edje/bible.edj 


# Each subdirectory must supply rules for building sources it contributes
res/edje/%.edj: ../res/edje/%.edc
	@echo 'Building file: $<'
	@echo 'Invoking: EDC Resource Compiler'
	edje_cc -id "$(PROJ_PATH)/edje/images" -id "$(SDK_TOOLPATH)/enventor/share/enventor/images" -sd "$(PROJ_PATH)/edje/sounds" -sd "$(SDK_TOOLPATH)/enventor/share/enventor/sounds" -fd "$(PROJ_PATH)/edje/fonts" -fd "$(SDK_TOOLPATH)/enventor/share/enventor/fonts" "$<" "$@"
	@echo 'Finished building: $<'
	@echo ' '


