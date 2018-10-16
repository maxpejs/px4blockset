function create_delivery()
revision = '1.1';   % umstieg von svn-Revision auf x.x Versionvergabe (10.04.2018)

c = tic;
clc;
basePath = getBasePath();

% create folder Delivery
deliveryPath = [basePath '..\Delivery\PX4Blockset\'];

fid = fopen('zip_package_list.txt');
line = fgetl(fid);
while ischar(line)
    if ~isempty(line)
        [pathstr, ~, ~] = fileparts(line);

        src = fullfile(basePath, line);
        dst = fullfile(deliveryPath, line);

        dstPath = fullfile(deliveryPath, pathstr);

        if ~exist(dstPath, 'dir')
            mkdir(dstPath);
        end
        disp(['copy from ' src]);
        disp(['copy to ' dst]);
        copyfile(src, dst, 'f');
    end
    line = fgetl(fid);
end
fclose(fid);

PX4Downloader = '..\PX4FileDownloader\Released\PX4FileDownloader 1.0';
copyfile([basePath PX4Downloader], [deliveryPath 'tools\PX4FileDownloader'], 'f');

copyfile([basePath '..\Release_Notes.txt'], deliveryPath, 'f');

zipfile = [basePath '..\Delivery\PX4Blockset ' revision '.zip'];
zip(zipfile, deliveryPath);

rmdir(deliveryPath, 's')
c = toc(c);
disp(['took: ' num2str(c) 'ms']);
end


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