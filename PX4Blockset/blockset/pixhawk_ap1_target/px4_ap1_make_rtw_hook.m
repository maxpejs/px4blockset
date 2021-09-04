function px4_ap1_make_rtw_hook(hookMethod, modelName, rtwroot, templateMakefile, buildOpts, buildArgs)
% This is the hook file for the build process (make_rtw)
% Based on ert_make_rtw_hook(...)
% for more informations see ert_make_rtw_hook.m

switch hookMethod
    case 'error'
        customPrint('Error');
    case 'entry'
        customPrint('Start build process ...');
    
	case 'before_tlc'
        customPrint('Start code generation process ...');

    case 'after_tlc'
        customPrint('Code generation process done ...');

    case 'before_make'
		customPrint('Start build process ...');
	
        if(~ModelCheckSucessful)
            customPrint('ERROR: Model check failed. Codegen process aborted');
            return;
        end
        customPrint('Prepare source files...');
        try
            basePath    = getBasePath();
            blk         = find_system(bdroot, 'FollowLinks', 'on',  'LookUnderMasks', 'all', 'MaskType', 'PX4_Target_Setup');
            if length(blk) == 1
                buildEna    = get_param(blk, 'cbBuild');
                uploadEna   = get_param(blk, 'cbUpload');
                cbPort      = get_param(blk, 'cbPort');
                copySources(basePath);
                genMakeParamFile(basePath, cbPort);
                genBatchFiles(basePath);

                if strcmpi(buildEna, 'on')
                    customPrint('Start build software...');
                    try
                        % check debug mode changed flag
                        if evalin('base', 'PX4DebugModeChanged') == 1
                            % Debug mode was changed, so we need to recompile all sources.
                            % Therefor clean project (delete object files)
                            status = customDos('makeClean.bat');
                            if(status == 2)
                                % compile error
                                msgbox(['makeClean.bat error at' modelName '. See command window for more informations. ', ...
                                    'Compilation terminated'],'error');
                                return;
                            end
                            % clear flag
                            evalin('base', 'PX4DebugModeChanged = 0;');
                        end
                    catch e

                    end

                    status = customDos('makeBuild.bat');

                    if(status == 2)
                        % compile error
                        msgbox(['makeBuild.bat error at ' modelName '. See command window for more informations. ', ...
                            'Compilation terminated'],'error');
                        return;
                    end
                    
                    if strcmpi(uploadEna, 'on')
                        customPrint('Start upload software...');
                        customPrint('Wait for bootloader. Please reset PXFMU-board...');

                        % inform about the need to reboot the board before upload
                        h = msgbox('Wait for bootloader. Please reset PXFMU-board...', 'Ready to upload', 'none');
                        customDos('makeUpload.bat');
                        % kill message box after uploading firmware
                        delete(h);
                    else
						customPrint('"Upload" option is disabled ...');
					end
                else
					customPrint('"Build" option is disabled ...');
				end
            else
                errordlg('Oops');
            end

        catch e
            errordlg(['Error: ' e.message]);
        end

    case 'after_make'
        % Upload

    case 'exit'
        customPrint('END!');
end
end

%%
% Kopiert alle Peripherie-Treiber in den Build-Ordner
%
function copySources(basePath)
filenameList = [];

%% HAL driver source files
customPrint('Prepare list with files to copy');
path = [basePath 'src\drivers\CMSIS\Include\'];
filenameList{length(filenameList) + 1} = [path 'core_cm4.h'];
filenameList{length(filenameList) + 1} = [path 'core_cmFunc.h'];
filenameList{length(filenameList) + 1} = [path 'core_cmInstr.h'];
filenameList{length(filenameList) + 1} = [path 'core_cmSimd.h'];
filenameList{length(filenameList) + 1} = [path 'cmsis_gcc.h'];

path = [basePath 'src\drivers\CMSIS\Device\ST\STM32F4xx\Include\'];
filenameList{length(filenameList) + 1} = [path 'stm32f4xx.h'];
filenameList{length(filenameList) + 1} = [path 'stm32f427xx.h'];
filenameList{length(filenameList) + 1} = [path 'system_stm32f4xx.h'];

path = [basePath 'src\drivers\CMSIS\Device\ST\STM32F4xx\Source\'];
filenameList{length(filenameList) + 1} = [path 'system_stm32f4xx.c'];

path = [basePath 'src\drivers\STM32F4xx_HAL_Driver\'];
filenameList{length(filenameList) + 1} = [path 'stm32f4xx_hal_conf.h'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal.c'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal.h'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal_rcc.c'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_rcc.h'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_rcc_ex.h'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal_pwr_ex.c'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_pwr_ex.h'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal_cortex.c'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_cortex.h'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_def.h'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_flash.h'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_flash_ex.h'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_flash_ramfunc.h'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_pwr.h'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_gpio.h'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_gpio_ex.h'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal_gpio.c'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_uart.h'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal_uart.c'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_dma.h'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal_dma.c'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_dma_ex.h'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_tim.h'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal_tim.c'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_tim_ex.h'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal_tim_ex.c'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_spi.h'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal_spi.c'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_i2c.h'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_i2c_ex.h'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal_i2c.c'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal_i2c_ex.c'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_hal_sd.h'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal_sd.c'];
filenameList{length(filenameList) + 1} = [path 'inc\stm32f4xx_ll_sdmmc.h'];
filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_ll_sdmmc.c'];
% filenameList{length(filenameList) + 1} = [path 'src\stm32f4xx_hal_timebase_tim.c'];

% module source files
path = [basePath 'src\modules\'];
filenameList{length(filenameList) + 1} = [path 'inttypes.h'];
filenameList{length(filenameList) + 1} = [path 'defines.h'];
filenameList{length(filenameList) + 1} = [path 'macros.h'];
filenameList{length(filenameList) + 1} = [path 'target_setup\target_setup.h'];
filenameList{length(filenameList) + 1} = [path 'target_setup\target_setup.c'];
filenameList{length(filenameList) + 1} = [path 'target_setup\cpu_load.h'];
filenameList{length(filenameList) + 1} = [path 'target_setup\cpu_load.c'];
filenameList{length(filenameList) + 1} = [path 'target_setup\timestamp.h'];
filenameList{length(filenameList) + 1} = [path 'target_setup\timestamp.c'];
filenameList{length(filenameList) + 1} = [path 'target_setup\i2c_drv.h'];
filenameList{length(filenameList) + 1} = [path 'target_setup\i2c_drv.c'];
filenameList{length(filenameList) + 1} = [path 'target_setup\spi_drv.h'];
filenameList{length(filenameList) + 1} = [path 'target_setup\spi_drv.c'];
filenameList{length(filenameList) + 1} = [path 'comm_itf\comm_itf.h'];
filenameList{length(filenameList) + 1} = [path 'comm_itf\comm_itf.c'];
filenameList{length(filenameList) + 1} = [path 'error_handler\error_handler.h'];
filenameList{length(filenameList) + 1} = [path 'error_handler\error_handler.c'];
filenameList{length(filenameList) + 1} = [path 'interrupt\stm32f4xx_it.h'];
filenameList{length(filenameList) + 1} = [path 'interrupt\stm32f4xx_it.c'];
filenameList{length(filenameList) + 1} = [path 'fmu_amber_led\fmu_amber_led.h'];
filenameList{length(filenameList) + 1} = [path 'fmu_amber_led\fmu_amber_led.c'];
filenameList{length(filenameList) + 1} = [path 'pwm_aux_out\pwm_aux_out.h'];
filenameList{length(filenameList) + 1} = [path 'pwm_aux_out\pwm_aux_out.c'];
filenameList{length(filenameList) + 1} = [path 'pxio_driver\pxio_driver.h'];
filenameList{length(filenameList) + 1} = [path 'pxio_driver\pxio_driver.c'];
filenameList{length(filenameList) + 1} = [path 'rc_ppm_input\rc_ppm_input.c'];
filenameList{length(filenameList) + 1} = [path 'rc_ppm_input\rc_ppm_input.h'];
filenameList{length(filenameList) + 1} = [path 'pwm_main_out\pwm_main_out.c'];
filenameList{length(filenameList) + 1} = [path 'pwm_main_out\pwm_main_out.h'];
filenameList{length(filenameList) + 1} = [path 'mpu6000\mpu6000.c'];
filenameList{length(filenameList) + 1} = [path 'mpu6000\mpu6000.h'];
filenameList{length(filenameList) + 1} = [path 'gps\gps.h'];
filenameList{length(filenameList) + 1} = [path 'gps\gps.c'];
filenameList{length(filenameList) + 1} = [path 'gps\gps_rmc_parcer.h'];
filenameList{length(filenameList) + 1} = [path 'gps\gps_rmc_parcer.c'];
filenameList{length(filenameList) + 1} = [path 'color_power_led\color_power_led.h'];
filenameList{length(filenameList) + 1} = [path 'color_power_led\color_power_led.c'];
filenameList{length(filenameList) + 1} = [path 'utilities\utilities.h'];
filenameList{length(filenameList) + 1} = [path 'utilities\utilities.c'];
filenameList{length(filenameList) + 1} = [path 'utilities\logger_ring_buffer.h'];
filenameList{length(filenameList) + 1} = [path 'utilities\logger_ring_buffer.c'];
filenameList{length(filenameList) + 1} = [path 'signal_output\signal_output.h'];
filenameList{length(filenameList) + 1} = [path 'signal_output\signal_output.c'];
filenameList{length(filenameList) + 1} = [path 'ms5611\ms5611.h'];
filenameList{length(filenameList) + 1} = [path 'ms5611\ms5611.c'];
filenameList{length(filenameList) + 1} = [path 'hmc5883\hmc5883.h'];
filenameList{length(filenameList) + 1} = [path 'hmc5883\hmc5883.c'];
filenameList{length(filenameList) + 1} = [path 'tasks\tasks.h'];
filenameList{length(filenameList) + 1} = [path 'tasks\tasks.c'];
filenameList{length(filenameList) + 1} = [path 'sd_card_logger\sd_card_logger.h'];
filenameList{length(filenameList) + 1} = [path 'sd_card_logger\sd_card_logger.c'];

filenameList{length(filenameList) + 1} = [basePath 'src\third party\protocol.h'];

path = [basePath 'src\target\'];
filenameList{length(filenameList) + 1} = [path 'ffconf.h'];
filenameList{length(filenameList) + 1} = [path 'sd_diskio.c'];
filenameList{length(filenameList) + 1} = [path 'sd_diskio.h'];
filenameList{length(filenameList) + 1} = [path 'stm324xg_eval_sd.h'];
filenameList{length(filenameList) + 1} = [path 'stm324xg_eval_sd.c'];

% filenameList{length(filenameList) + 1} = [basePath 'src\third party\FatFs\Target\stm324xg_eval_sd.h'];
% filenameList{length(filenameList) + 1} = [basePath 'src\third party\FatFs\Target\stm324xg_eval_sd.c'];


%% startup and toolchain files
path = [basePath 'tools\px4_toolchain\'];
filenameList{length(filenameList) + 1} = [path 'px_mkfw.py'];
filenameList{length(filenameList) + 1} = [path 'px_uploader.py'];
filenameList{length(filenameList) + 1} = [path 'px4fmu-v2.prototype'];

path = [basePath 'src\startup\'];
filenameList{length(filenameList) + 1} = [path 'linker.ld'];
filenameList{length(filenameList) + 1} = [path 'startup_stm32f427xx.s'];

%% workaround
% files needed by matlab for code compilation, but will not copy to target
% folder atomatically by RTW, so we will do it manually
% checked for R2008 only, not sure if works for other MATLAB versions
if strcmp(version('-release'),'2008a')
	filenameList{length(filenameList) + 1} = [matlabroot '\rtw\c\libsrc\rtlibsrc.h'];
end

% remove double entries
filenameList = unique(filenameList);

%% copy all files to target folder
for i = 1:length(filenameList)
    file2copy = filenameList{i};
    try
        customPrint(['Copy file: ' file2copy]);
        copyfile(file2copy,  pwd, 'f');
    catch e
        customPrint(e.message);
        rethrow(e);
    end
end

% Workaround: copy files into scecific folder in the target folder
copyfile([basePath 'src\drivers\STM32F4xx_HAL_Driver\inc\Legacy'],  [pwd '\Legacy'], 'f');
copyfile([basePath 'src\third party\FatFs'],  [pwd '\FatFs'], 'f');
copyfile([basePath 'src\third party\FreeRTOS'],  [pwd '\FreeRTOS'], 'f');
end

%%
%
%
function genMakeParamFile(basePath, comPort)
try
    customPrint('Generating "makeParams.mk"');
    f = fopen('makeParams.mk', 'w');
    fprintf(f, '# Auto generated by px4_ap1_make_rtw_hook.m\n ');
    fprintf(f, '# Changes to this file may cause incorrect behavior\n');
    fprintf(f, '# and will be lost if the code is regenerated\n');
    fprintf(f, '# PX4-Blockset\n');
    fprintf(f, '# Toolchain paths for make file\n\n');

    text = ['ARMTOOLCHAIN = ' basePath 'tools\arm_toolchain_embedded\bin\arm-none-eabi-'];
    text = protectPathSeparator(text);
    fprintf(f, text);

    text = ['PYTHON = ' basePath 'tools\python_3.5.1\python.exe'];
    text = protectPathSeparator(text);
    fprintf(f, text);

    comPort = valueFromCell(comPort);
    text = ['COMPORT = ' comPort '\n'];

    fprintf(f, text);
    fclose(f);
catch e
    fclose(f);
    customPrint(e.message);
    rethrow(e);
end
end

function genBatchFiles(basePath)
% gen "makeAll" batch file
try
    customPrint('Generate build batch file "makeAll.bat"');
    f = fopen('makeAll.bat', 'w');
    writeHeaderToFile(f, 'REM Batch file for run "make all"\n');
    makeTooPath = [basePath 'tools\GnuWin32\bin\make.exe'];
    cmd = [makeTooPath ' -j8 --makefile=Makefile all'];
    cmd = protectPathSeparator(cmd);
    fprintf(f, cmd);
    fclose(f);
catch e
    fclose(f);
    customPrint(e.message);
    rethrow(e)
end

% gen "makeBuild" batch file
try
    customPrint('Generate build batch file "makeBuild.bat"');
    f = fopen('makeBuild.bat', 'w');
    writeHeaderToFile(f, 'REM Batch file for run "make build"\n');
    makeTooPath = [basePath 'tools\GnuWin32\bin\make.exe'];
    cmd = [makeTooPath ' -j8 --makefile=Makefile build'];
    cmd = protectPathSeparator(cmd);
    fprintf(f, cmd);
    fclose(f);
catch e
    fclose(f);
    customPrint(e.message);
    rethrow(e);
end

% gen "makeClean" batch file
try
    customPrint('Generate build batch file "makeClean.bat"');
    f = fopen('makeClean.bat', 'w');
    writeHeaderToFile(f, 'REM Batch file for run "make clean"\n');
    makeTooPath = [basePath 'tools\GnuWin32\bin\make.exe'];
    cmd = [makeTooPath ' --makefile=Makefile clean'];
    cmd = protectPathSeparator(cmd);
    fprintf(f, cmd);
    fclose(f);
catch e
    fclose(f);
    customPrint(e.message);
    rethrow(e);
end

% gen "makeUpload" batch file
try
    customPrint('Generate build batch file "makeUpload.bat"');
    f = fopen('makeUpload.bat', 'w');
    writeHeaderToFile(f, 'REM Batch file for uploading created px4 image\n');
    makeTooPath = [basePath 'tools\GnuWin32\bin\make.exe'];
    cmd = [makeTooPath ' -j8 --makefile=Makefile upload'];
    cmd = protectPathSeparator(cmd);
    fprintf(f, cmd);
    fclose(f);
catch e
    fclose(f);
    customPrint(e.message);
    rethrow(e);
end
end

%%
%
%
function retValue = blockExistsInModel(blockType)
res = find_system(bdroot, 'FollowLinks', 'on',  'LookUnderMasks', 'all', 'MaskType', blockType);
retValue = ~isempty(res);
end

%%
% Krücke um Pfad zu der Toolkette zu bekommen
%
function basePath = getBasePath()
hookPath = which('px4_ap1_make_rtw_hook.m');
splitted = regexp(hookPath, '\\', 'split');
splitted_new = splitted(1:length(splitted)-3);
basePath = '';
for i = 1:length(splitted_new)
    filePart = splitted_new{i};
    basePath = [basePath filePart '\'];
end
end

%%
%
%
function writeHeaderToFile(fid, desc)
fprintf(fid, '@echo OFF\n');
fprintf(fid, 'setlocal enabledelayedexpansion\n');
fprintf(fid, 'REM Auto generated\n');
fprintf(fid, 'REM Changes to this file may cause incorrect behavior\n');
fprintf(fid, 'REM and will be lost if the code is regenerated\n');
fprintf(fid, 'REM Created with PX4-Blockset\n\n\n');
fprintf(fid, desc);
end

%%
%
%
function text = protectPathSeparator(text)
text = strrep(text, '\', '\\');
text = [text '\n'];
end

function ret = ModelCheckSucessful()
try
    % Let's see if the modell check was OK
    ret = evalin('base', 'PX4ModelCheckSuccessful');
catch e
    customPrint('Toolchain error. No workspace variable found in ');
    ret = 0;
end
end

%%
function status = customDos(cmd)

[status, cmdout] = dos(cmd);
customPrint(cmdout);
end


%%
%
%
function value = valueFromCell(value)
while iscell(value)
    value = value{:};
end
end

%%
%
%
function customPrint(text)
fprintf(2, '%s\n', text);
end


