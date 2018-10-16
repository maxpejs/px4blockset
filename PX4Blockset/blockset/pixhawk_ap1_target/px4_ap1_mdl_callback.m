function px4_ap1_mdl_callback(mode, varargin)

if strcmp(mode, 'check_model')
    try
        % check the model for correct px4 block configuration
        checkConfigConsistency();
        % seems to be ok, create a workspace variable and assing value "1" it for keeping 
        % code building process continue
        evalin('base', 'PX4ModelCheckSuccessful = 1;');
    catch e
        % something went wrong, create a workspace variable and assing
        % value "0" it to stop code building process
        evalin('base', 'PX4ModelCheckSuccessful = 0;');
        errordlg(e.message);
    end

elseif strcmp(mode, 'SampleTimeValue')
    %%
    SampleTimeValue = get_param(gcb, 'SampleTimeValue');
    % SampleTimeValue = str2double(SampleTimeValue);
    set_param(bdroot, 'Solver', 'FixedStepDiscrete','FixedStep', num2str(SampleTimeValue));

elseif strcmp(mode, 'test')


elseif strcmp(mode, 'CopyPX4SetupCallback')
    %% called by block copy or block add
    px4_ap1_mdl_callback('SampleTimeValue'); % copy sample time to model parameter

    % set system target file
    set_param(bdroot, 'SystemTargetFile', 'px4_ap1.tlc');

    % no makefile auto generate
    set_param(bdroot, 'GenerateMakefile', 'off');

    % set templates
    set_param(bdroot, 'ERTCustomFileTemplate',      'px4_ap1_main.tlc');
    set_param(bdroot, 'ERTDataHdrFileTemplate',     'px4_ap1_code_template.cgt');
    set_param(bdroot, 'ERTDataSrcFileTemplate',     'px4_ap1_code_template.cgt');
    set_param(bdroot, 'ERTHdrFileBannerTemplate',   'px4_ap1_code_template.cgt');
    set_param(bdroot, 'ERTSrcFileBannerTemplate',   'px4_ap1_code_template.cgt');
    set_param(bdroot, 'IncludeMdlTerminateFcn',     'off');

    % no classic call interface
    set_param(bdroot, 'GRTInterface', 'off');

    % single "in"- and "out"-functions
    set_param(bdroot, 'CombineOutputUpdateFcns', 'on');

    % don't generate example main programm
    set_param(bdroot, 'GenerateSampleERTMain', 'off');

    % set programming language
    set_param(bdroot, 'TargetLang', 'C');

    % generate only code
    set_param(bdroot, 'GenCodeOnly', 'on');

    % add model callback for checking model blocks configuration on "Model Update"
    set_param(bdroot, 'InitFcn', 'px4_ap1_mdl_callback(''check_model'');');
    
	% no warnings on inherited source blocks
    set_param(bdroot, 'InheritedTsInSrcMsg', 'none');
	
	% C99 (ISO) standard
    set_param(bdroot, 'TargetFunctionLibrary', 'C99 (ISO)');
    
    % set data type lenght
    set_param(bdroot, 'ProdHWDeviceType', 'ARM Compatible->ARM 7');
	
	% little endian byte ordering
	set_param(bdroot, 'ProdEndianess', 'LittleEndian');		  
	
elseif strcmp(mode, 'cbBuildChange')
    cbBuildChkBoxVal = get_param(gcb, 'cbBuild');
    parEnas = get_param(gcb, 'MaskEnables');

    if strcmpi(cbBuildChkBoxVal, 'off')
        set_param(gcb, 'cbUpload', 'off');
        parEnas(3) = {'off'};
        parEnas(4) = {'off'};
    else
        parEnas(3) = {'on'};
%        parEnas(4) = {'on'};
    end
    set_param(gcb, 'MaskEnables', parEnas)

elseif strcmp(mode, 'cbUploadChange')
    % TODO: Setzen der Checkboxen in die Blockmaske von px4setup auslagern?
    cbUploadChkBoxVal = get_param(gcb, 'cbUpload');
    parEnas = get_param(gcb, 'MaskEnables');

    if strcmpi(cbUploadChkBoxVal, 'off')
        parEnas(4) = {'off'};
    else
        parEnas(4) = {'on'};
    end
    set_param(gcb, 'MaskEnables', parEnas)

elseif strcmp(mode, 'cbDebugChange')
    
    try
        cbDebugChkBoxVal_last = evalin('base', 'debugmode_last');
    catch
        cbDebugChkBoxVal_last = 'off';
    end

    cbDebugChkBoxVal = get_param(gcb, 'debugmode');

    if strcmpi(cbDebugChkBoxVal, cbDebugChkBoxVal_last) ~= 1
        evalin('base', 'PX4DebugModeChanged = 1;');
    end
    
    evalin('base', ['debugmode_last = ''' cbDebugChkBoxVal ''';']);

end
end

function checkConfigConsistency()
checkSingletonBlock('PX4_Target_Setup');
checkSingletonBlock('PX4_FMU_AMBER_LED');
checkSingletonBlock('PX4_PWM_AUX_OUT');
checkSingletonBlock('PX4_PWM_MAIN_OUT');
checkSingletonBlock('PX4_MPU6000');
checkSingletonBlock('PX4_GPS');
checkSingletonBlock('PX4_SDCARD');
checkSingletonBlock('PX4_COLOR_POWER_LED');
checkSingletonBlock('PX4_SIGNAL_OUTPUT');
checkSingletonBlock('PX4_HMC5883');
checkSingletonBlock('PX4_MS5611');
end

function checkSingletonBlock(blockType)
res = find_system(bdroot, 'FollowLinks', 'on',  'LookUnderMasks', 'all', 'MaskType', blockType);
res = unique(res);
if length(res) > 1
    msgID = 'PX4Blockset:InconsistentConfig';
    msg = ['Es darf nur ein "' blockType '"-Block im Model existieren'];
    baseException = MException(msgID, msg);
    throw(baseException);
end
end

% function value = valueFromCell(value)
% while iscell(value)
%     value = value{:};
% end
% end





















