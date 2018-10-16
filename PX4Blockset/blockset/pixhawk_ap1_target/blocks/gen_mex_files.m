clc;
fileList = dir;
for i = 1 : length(fileList)
    if ~fileList(i).isdir
        fileName = fileList(i).name;
        splitted = regexp(fileName, '\.', 'split');
        extension = splitted(end);
        if strcmpi(extension, 'cpp')
            eval(['mex ' fileName]);
        end
    end
end
