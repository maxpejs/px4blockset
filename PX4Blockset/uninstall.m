% some header

% open file with paths
fid = fopen('addpathList'); 

% reab file line by line until EOF
tline = fgetl(fid); 
while ischar(tline)
	% Build path
    pathToAdd = [pwd, tline];
    % add to MATLAB paths
    rmpath(pathToAdd);
    % read next line
    tline = fgetl(fid); 
end 
fclose(fid); 

error = savepath;
if error
    disp('Warning! Couldn''t save paths. Start MATLAB as administrator');
end