clc;
c = tic;

fileList = dir;

for i = 1 : length(fileList)
    if ~fileList(i).isdir
        fileName = fileList(i).name;
        
        [pathstr, name, ext] = fileparts(fileName);
        
        if strcmpi(ext, '.mdl')
            rtwbuild(name)
            bdclose all
            try 
                rmdir([name '_px4blockset'], 's');
            catch
            end
        end
    end
end


rmdir('slprj', 's');
c = toc(c);
disp(['Block (gen+compile) tests took: ' num2str(c) ' s']);
msgbox('Tests OK', '', 'none');