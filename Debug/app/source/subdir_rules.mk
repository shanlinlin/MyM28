################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
app/source/%.obj: ../app/source/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-c2000_18.1.4.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="G:/SLL/CODE/DSP28_HT/M28_HT/thirdparty/TI/28335/DSP2833x_headers/include" --include_path="G:/SLL/CODE/DSP28_HT/M28_HT/drive/28335/include" --include_path="G:/SLL/CODE/DSP28_HT/M28_HT/TemperLib/include" --include_path="G:/SLL/CODE/DSP28_HT/M28_HT/thirdparty/TI/28335/DSP2833x_common/include" --include_path="G:/SLL/CODE/DSP28_HT/M28_HT/kernel/EthercatSlave/include" --include_path="G:/SLL/CODE/DSP28_HT/M28_HT/platform/include" --include_path="G:/SLL/CODE/DSP28_HT/M28_HT/firmware/include" --include_path="G:/SLL/CODE/DSP28_HT/M28_HT/kernel/comm/include" --include_path="G:/SLL/CODE/DSP28_HT/M28_HT/kernel/EthercatMaster/include" --include_path="G:/SLL/CODE/DSP28_HT/M28_HT/app/include" --include_path="G:/SLL/CODE/DSP28_HT/M28_HT" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-c2000_18.1.4.LTS/include" --advice:performance=all --define=_FAST_FUNC --define=D_HYB5_335 --define=LARGE_MODEL -g --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="app/source/$(basename $(<F)).d_raw" --obj_directory="app/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


