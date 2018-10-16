% some header

% open file with paths
fid = fopen('addpathList'); 

% reab file line by line until EOF
tline = fgetl(fid); 

while ischar(tline)
	% Build path
    pathToAdd = [pwd, tline];
    % add to MATLAB paths
    addpath(pathToAdd);
    % read next line
    tline = fgetl(fid);
end 
fclose(fid); 

error = savepath;
if error
    disp('Warning! Couldn''t save paths. Start MATLAB as administrator');
end
                        
choice = questdlg('Would you like to install a PX4 drivers?', 'PX4 windows driver', 'Yes', 'No', 'Yes');

% Handle response
switch choice
    case 'Yes'
         status = dos('tools\px4_win_driver\px4driver.msi');
    case 'No'
end