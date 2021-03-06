#include "sd_card_logger.h"
#include <logger_ring_buffer.h>
#include <FatFs/Core/ff_gen_drv.h>
#include <FatFs/Drivers/sd_diskio.h>
#include <cmsis_os.h>

#define FILENAME_MAX_LENGTH 	13
#define FILE_LOGGER_MAX_CNT 	FF_FS_LOCK

#define MIN_PACKAGE_SIZE_CNT 	5
#define MAX_PACKAGE_SIZE_CNT  	10

#if MIN_PACKAGE_SIZE_CNT > MAX_PACKAGE_SIZE_CNT
#error	check min and max package lengths in sd card module
#endif

typedef struct
{
	FIL		 	fi;     	/* File object */
	char 		filename[FILENAME_MAX_LENGTH];
	char 		filename_prefix[FILENAME_MAX_LENGTH];
	uint32_t 	id;
	uint32_t 	sample_time;
	uint32_t 	sample_cnt;
	uint32_t 	sig_cnt;
	uint32_t	tick_last_call;
	uint32_t 	file_logger_state;
	ring_buff_data_st rbuff;

}sd_card_file_info_st;

/* FatFs variables */
FATFS 	_sd_file_system;  			/* File system object for SD card logical drive */

static char 	_drive_path[4]; 	/* SD card logical drive path */
static uint32_t _module_state 		= DISABLE;
static uint32_t _file_logger_cnt 	= 0;

static char 	_usercmd[20];
static int 		_new_command_received = 0;

static sd_card_file_info_st * file_data_arr[FILE_LOGGER_MAX_CNT];

static uint32_t 	_sd_card_logger_init_sdcard();
static uint32_t 	_get_next_id_for_logfile(char * filename_prefix);
static ErrorStatus 	_create_new_log_file(sd_card_file_info_st * info);
static void 		_write_to_sd_card(sd_card_file_info_st * info);
static void 		_delete_all_files();
static void 		_list_single_file(char * path);
static void 		_px4_sd_card_logger_stop();
static void 		_px4_sd_card_logger_resume();
static void 		_px4_sd_card_logger_process_cmd();
static FRESULT 		_scan_files(char* path);

void px4_sd_card_logger_init()
{
	_sd_card_logger_init_sdcard();
	memset(file_data_arr, 0 , sizeof(file_data_arr));
	px4debug(eCOMMITF, "sd card logger init ok\r\n");
	_module_state = ENABLE;
}

uint32_t px4_sd_card_logger_add_new_logger(uint32_t sampleTime, uint32_t sigCnt, char * file_name)
{
	if (_file_logger_cnt >= FILE_LOGGER_MAX_CNT)
	{
		px4debug(eCOMMITF, "ERROR! sd card logger. max logger counter limit reached \r\n");
		return 0xFFFFFFFF;
	}

	uint32_t ret = _file_logger_cnt;

	sd_card_file_info_st * fi = calloc(1, sizeof(sd_card_file_info_st));

	fi->rbuff.read 	= 0;
	fi->rbuff.write = 0;

	fi->id 				= _file_logger_cnt;
	fi->sample_time 	= sampleTime;
	fi->sig_cnt 		= (sigCnt > SIGNAL_MAX_CNT) ? SIGNAL_MAX_CNT : sigCnt;

	strncpy(fi->filename_prefix, file_name, FILENAME_MAX_LENGTH-4); // 3 chars for id, 1 for termination

	if (_create_new_log_file(fi) == ERROR)
	{
		px4debug(eCOMMITF, "#Error creating new log file... Disabling SD Card module! \r\n");
		_module_state = DISABLE;
		return 0xFFFFFFFF;
	}
	else
	{
		fi->file_logger_state = ENABLE;

			file_data_arr[_file_logger_cnt] = fi;
			_file_logger_cnt++;
			px4debug(eCOMMITF, "new logger %s created \r\n", fi->filename);
			return ret;
	}

	return ret;
}

void px4_sd_card_logger_add_val(uint32_t log_id, float * values)
{
	if (_module_state == DISABLE)
	{
		return;
	}

	uint32_t ts = tic();

	if (log_id >= FILE_LOGGER_MAX_CNT)
	{
		return;
	}

	if (file_data_arr[log_id] == 0)
	{
		px4debug(eCOMMITF, "error at px4_sd_card_logger_add_val => NULL-pointer access! Log id:%d \r\n", log_id);
		_module_state = DISABLE;
		return;
	}

	// check if cycle time elapsed
	if ((ts / 1000) < (file_data_arr[log_id]->sample_time * file_data_arr[log_id]->sample_cnt))
	{
		return;
	}

	// init counter of samples for exact timing for adding values according to sampling time
	if(file_data_arr[log_id]->sample_cnt == 0)
	{
		file_data_arr[log_id]->sample_cnt = (uint32_t) (ts / 1000) / file_data_arr[log_id]->sample_time;
	}

	file_data_arr[log_id]->sample_cnt++;
	file_data_arr[log_id]->tick_last_call = ts/1000;

	if (ring_buffer_free_space(&(file_data_arr[log_id]->rbuff)) > 0)
	{
		uint32_t idx = file_data_arr[log_id]->rbuff.write;

		memcpy(file_data_arr[log_id]->rbuff.buff[idx].val, values, sizeof(float) * file_data_arr[log_id]->sig_cnt);
		file_data_arr[log_id]->rbuff.buff[idx].timestamp = ts;

		file_data_arr[log_id]->rbuff.write = ++idx % RING_BUFF_SIZE;
	}
	else
	{
		px4debug(eCOMMITF, "sd card logger. not enough free buffer space\r\n");
	}
}

void px4_sd_card_logger_task(void)
{
	if (_module_state == DISABLE)
	{
		return;
	}

	for (uint32_t i = 0; i < _file_logger_cnt; i++)
	{
		if (file_data_arr[i] == 0)
		{
			px4debug(eCOMMITF, "error at px4_sd_card_logger_task => NULL-pointer access!");
			_module_state = DISABLE;
			continue;
		}

		if (file_data_arr[i]->file_logger_state == DISABLE)
		{
			continue;
		}

		_write_to_sd_card(file_data_arr[i]);

	}

	_px4_sd_card_logger_process_cmd();

}

static void _write_to_sd_card(sd_card_file_info_st * info)
{
	FRESULT fr;
	UINT writtenbytes = 0;
	uint32_t idxRead, idxWrite;

//	px4debug(eCOMMITF, "Buff len:%d \r\n", SD_RING_BUFF_SIZE - _ring_buffer_free_space(&(info->rbuff)) - 1);

	while (ring_buffer_count(&(info->rbuff)) >= MIN_PACKAGE_SIZE_CNT)
	{
		// px4debug(eCOMMITF, "b.size %d r %d w %d", ring_buffer_count(&(info->rbuff)), info->rbuff.read, info->rbuff.write);

		idxRead = info->rbuff.read;
		idxWrite= info->rbuff.write;

		uint32_t numofsamples = 0;
		
		// we want to write a couple of samples stored in ring buffer as single block on SD card
		// we should be aware that we don't leave the array bound of the ring buffer during write process
		if (idxWrite > idxRead)
		{
			// read index is behind the write index, like this
			//							 w
			//							 |
			//	[x] [x] [x] [x] [x] [x] [x] [x] [x]
			//				 |
			//				 r
			
			// count of samples stored in memory one after another
			numofsamples = idxWrite - idxRead;
			// px4debug(eCOMMITF, " w>r ");
		}
		else
		{
			// write index is behind the read index, like this
			//				 w
			//				 |
			//	[x] [x] [x] [x] [x] [x] [x] [x] [x]
			//				 			 |
			//				 			 r
			
			// count of samples stored in memory one after another until the end of array is reached
			numofsamples = RING_BUFF_SIZE - idxRead;
			// px4debug(eCOMMITF, " r>w ");
		}

		// px4debug(eCOMMITF, "\r\n");

		numofsamples = numofsamples > MAX_PACKAGE_SIZE_CNT ? MAX_PACKAGE_SIZE_CNT : numofsamples;

		fr = f_write(&(info->fi), (const void*) &info->rbuff.buff[idxRead], sizeof(log_dataset_st) * numofsamples, &writtenbytes);
		f_sync(&(info->fi));

		if ((fr != FR_OK) || (writtenbytes != sizeof(log_dataset_st) * numofsamples))
		{
			px4debug(eCOMMITF, "err writing data"
					"%d vs. %d\r\n", sizeof(log_dataset_st) * numofsamples, writtenbytes);
		}
		else
		{
			info->rbuff.read = (idxRead + numofsamples) % RING_BUFF_SIZE;
			// px4debug(eCOMMITF, "written %d samples \r\n", numofsamples);
		}
	}
}

static uint32_t _sd_card_logger_init_sdcard()
{
	// Link the micro SD disk I/O driver
	if (FATFS_LinkDriver(&SD_Driver, _drive_path) == 0)
	{
		px4debug(eCOMMITF, "SD Drive %s linked \r\n", _drive_path);
	}

	// Register the file system object to the FatFs module
	if (f_mount(&_sd_file_system, (TCHAR const*) _drive_path, 0) != FR_OK)
	{
		/* FatFs Initialization Error */
		px4debug(eCOMMITF, "Error mount sd card drive! \r\n");
		return ERROR;
	}
	return SUCCESS;
}

static ErrorStatus _create_new_log_file(sd_card_file_info_st * info)
{
	FRESULT res = 0;
	uint32_t log_idx;
	UINT writtenbytes;

	log_idx = _get_next_id_for_logfile(info->filename_prefix);
	snprintf(info->filename, 32, "%s%03d", info->filename_prefix, (int) log_idx);

	// create and open new log file
	px4debug(eCOMMITF, "sdcard. create new logger file ... ");
	res = f_open(&(info->fi), info->filename, FA_CREATE_ALWAYS | FA_WRITE);
	f_sync(&(info->fi));
	px4debug(eCOMMITF, "done \r\n");


	// write number of signals as meta info
	res |= f_write(&(info->fi), &(info->sig_cnt), sizeof(info->sig_cnt), &writtenbytes);
	f_sync(&(info->fi));

	// write sampling time of block as meta info
	res |= f_write(&(info->fi), &(info->sample_time), sizeof(info->sample_time), &writtenbytes);
	f_sync(&(info->fi));

	// check results
	if ((res != FR_OK) || (writtenbytes != sizeof(info->sample_time)))
	{
		px4debug(eCOMMITF, "#Error open file, or error at writing file meta block\r\n");
		f_close(&(info->fi));
	}

	return (res == FR_OK) ? SUCCESS : ERROR;
}

static uint32_t _get_next_id_for_logfile(char * filename_prefix)
{
	// TODO use meta file to store id

	uint32_t ret = 0;

	FRESULT res;
	DIR dir;
	static FILINFO fno;
	char* path = "/";
	char buffArr[200];

	res = f_opendir(&dir, path); /* Open the directory */
	if (res == FR_OK)
	{
		for (;;)
		{
			res = f_readdir(&dir, &fno); /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0)
				break; /* Break on error or end of dir */
			if ((fno.fattrib & AM_DIR) == 0)
			{
				/* It is a file. */
				sprintf(buffArr, "%s", fno.fname);

				// compare strings
				uint32_t a = strlen(filename_prefix);
				uint32_t b = strlen(buffArr);

				if (strncmp(filename_prefix, buffArr, a) == 0)
				{
					if ((b - a) > 0)
					{
						uint32_t t = atoi(&buffArr[a]);
						if (t >= ret)
						{
							ret = t+1;
						}
					}
				}
			}
		}
		f_closedir(&dir);
	}
	else
	{
		px4debug(eCOMMITF, "open dir failed \r\n");
	}

	return ret;
}

static void _copy_file_name_from_cmd(const char * cmd, char * file)
{
	while (*cmd != 0 && *cmd != ' ')
		cmd++;

	if(*cmd == 0)
		return;

	cmd++;

	while(*cmd != '\r' && *cmd != '\n' && *cmd != 0)
		*file++ = *cmd++;

	*file = 0;
}

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

void px4_sd_card_logger_add_user_cmd(const char * cmd)
{
	memcpy(_usercmd, cmd, strlen(cmd)+1);
	_new_command_received = 1;
}

void _px4_sd_card_logger_process_cmd()
{
	if (_new_command_received == 0)
	{
		return;
	}
	char path[20];

	if (strncmp(_usercmd, "list all", strlen("list all")) == 0)
	{
		px4debug(eCOMMITF, "--- file list start ---\r\n");
		_scan_files("/");
		px4debug(eCOMMITF, "--- file list end ---\r\n");
	}
	else if (strncmp(_usercmd, "list ", strlen("list ")) == 0)
	{
		path[0] = '/';
		_copy_file_name_from_cmd(_usercmd, &path[1]);
		_list_single_file(path);
	}
	else if (strncmp(_usercmd, "del all", strlen("del all")) == 0)
	{
		px4debug(eCOMMITF, "==> Delete all files found on sd card ...");
		_delete_all_files();
		px4debug(eCOMMITF, " done \r\n");
	}
	else if (strncmp(_usercmd, "del ", strlen("del ")) == 0)
	{
		path[0] = '/';
		_copy_file_name_from_cmd(_usercmd, &path[1]);

		px4debug(eCOMMITF, "==> deleting file ");
		px4debug(eCOMMITF, path);
		px4debug(eCOMMITF, "... ");

		if (f_unlink(path) != FR_OK)
		{
			px4debug(eCOMMITF, "Error \r\n");
		}
		else
		{
			px4debug(eCOMMITF, " ok\r\n");
		}
	}
	//---------------------------------------------------------------------------
	else
	{
		px4debug(eCOMMITF, "UNKNOWN SD command received! Type <help> for more information\r\n");
	}

	_new_command_received = 0;
}

static FRESULT _scan_files(char* path)
{
	FRESULT res;
	FIL f;
	DIR dir;
	static FILINFO fno;

	UINT bytesread;

	uint32_t sig_cnt;
	uint32_t timeline;
	uint32_t sampletime;

	res = f_opendir(&dir, path); /* Open the directory */
	if (res == FR_OK)
	{
		for (;;)
		{
			sig_cnt = 0;
			sampletime = 0;
			timeline = 0;

			res = f_readdir(&dir, &fno); /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0)
			{
				// px4debug(eCOMMITF, "end of files\r\n");
				break; /* Break on error or end of dir */
			}

			res = f_open(&f, fno.fname, FA_READ);
			f_sync(&f);

			if (res == FR_OK)
			{
				res = f_read(&f, &sig_cnt, sizeof(sig_cnt), &bytesread);
				res |= f_read(&f, &sampletime, sizeof(sampletime), &bytesread);

				if ((res == FR_OK) && (bytesread == sizeof(sig_cnt)))
				{
					timeline = (fno.fsize - 8) / (8 + sig_cnt * 4);
//					px4debug(eCOMMITF, "sample time: %d fno.fsize: %d timeline: %d\r\n", sampletime, fno.fsize, timeline);
					timeline = (timeline * sampletime) / 1000;
				}
			}
			else if (res == FR_LOCKED)
			{
//				px4debug(eCOMMITF, "Error. File is in use: %s \r\n", fno.fname);
			}
			else
			{
//				 px4debug(eCOMMITF, "error on f_open, errcode:%d \r\n", res);
			}

			f_close(&f);

			px4debug(eCOMMITF, "/%s|%i|%i|%i|%i|%i\r\n", fno.fname, (int) fno.fsize,
					(int) (timeline / 60), (int) (timeline - (timeline / 60) * 60), (int) sig_cnt, (int) sampletime);
		}

		f_closedir(&dir);
	}
	else
	{
		px4debug(eCOMMITF, "open dir failed\r\n");
	}

	return res;
}

//========================================================================================
// Stops all logging activities and closes opened files, because of accessing to sd card
//========================================================================================
static void _px4_sd_card_logger_stop()
{
	for (uint32_t i = 0; i < _file_logger_cnt; i++)
	{
		if (file_data_arr[i] == 0)
		{
			px4debug(eCOMMITF, "error at _px4_sd_card_logger_stop => NULL-pointer access!");
			continue;
		}

		if (file_data_arr[i]->file_logger_state == ENABLE)
		{
			if (f_close(&(file_data_arr[i]->fi)) == FR_OK)
			{
				file_data_arr[i]->file_logger_state = DISABLE;
			}
			else
			{
				px4debug(eCOMMITF, "error at closing file on px4_sd_card_logger_stop");
				continue;
			}
		}
	}
}

//========================================================================================
// Resume stopped logging activities open new logger files
//========================================================================================
static void _px4_sd_card_logger_resume()
{
	for (uint32_t i = 0; i < _file_logger_cnt; i++)
	{
		if (file_data_arr[i] == 0)
		{
			px4debug(eCOMMITF, "error at _px4_sd_card_logger_resume => NULL-pointer access!");
			continue;
		}

		if (file_data_arr[i]->file_logger_state == DISABLE)
		{
			if (_create_new_log_file(file_data_arr[i]) == SUCCESS)
			{
				file_data_arr[i]->file_logger_state = ENABLE;
			}
			else
			{
				px4debug(eCOMMITF, "#Error creating new log file... Disabling SD Card module! \r\n");
				_module_state = DISABLE;
			}
		}

	}
}

//========================================================================================
// walks through the disk and deletes all found files
//========================================================================================
static void _delete_all_files()
{
	FRESULT res;
	char buff[50];
	UINT i, j;
	DIR dir;
	FILINFO fno;

	_px4_sd_card_logger_stop();

	strcpy(buff, "/");

	res = f_opendir(&dir, buff);
	if (res == FR_OK)
	{
		for (i = 0; buff[i]; i++)
			;
		for (;;)
		{
			res = f_readdir(&dir, &fno);
			if (res != FR_OK || !fno.fname[0])
				break;

			if (FF_FS_RPATH && fno.fname[0] == '.')
				continue;

			j = 0;
			do
				buff[i + j] = fno.fname[j];
			while (fno.fname[j++]);

			if (fno.fattrib & AM_DIR)
			{
				continue;
			}

			res = f_unlink(buff);
			if (res != FR_OK)
			{
				px4debug(eCOMMITF, "Error! ");
			}
		}
		f_closedir(&dir);
	}

	_px4_sd_card_logger_resume();
}

static void _list_single_file(char * path)
{
	FRESULT res;

	FIL f;
	log_dataset_st data;
	UINT bytesread_sigcnt, bytesread_sampletime;
	// char arr[150];
	uint32_t sig_cnt, sampletime;
	uint32_t packagesize;

	if ( FILE_LOGGER_MAX_CNT == _file_logger_cnt)
	{
		px4debug(eCOMMITF, "Error, max count of parallel allowed opened files is reached\r\n");
		return;
	}

	res = f_open(&f, path, FA_READ);
	f_sync(&f);

	if (res == FR_NO_FILE)
	{
		px4debug(eCOMMITF, "Error listing file => File doesn't exist \r\n");
		return;
	}
	else if (res == FR_LOCKED)
	{
		px4debug(eCOMMITF, "Error listing file => File currently used for logging \r\n");
		return;
	}
	else if (res != FR_OK)
	{
		px4debug(eCOMMITF, "Error listing file => Unknown case:%d \r\n", res);
		f_close(&f);
		return;
	}

	res = f_read(&f, &sig_cnt, sizeof(sig_cnt), &bytesread_sigcnt);
	f_sync(&f);

	res |= f_read(&f, &sampletime, sizeof(sampletime), &bytesread_sampletime);
	f_sync(&f);

	if ((res != FR_OK) || (bytesread_sigcnt != sizeof(sig_cnt)) || (bytesread_sampletime != sizeof(sampletime)))
	{
		px4debug(eCOMMITF, "Error reading sig cnt");
		f_close(&f);
		return;
	}

	packagesize = sizeof(log_dataset_st);

	px4debug(eCOMMITF, "=== BEGIN ===\r\n");

	do
	{
		res = f_read(&f, (void*) &data, packagesize, &bytesread_sigcnt);
		f_sync(&f);

		if ((res != FR_OK) || (bytesread_sigcnt != packagesize))
		{
			px4debug(eCOMMITF, "Error reading file");
			f_close(&f);
			break;
		}

		px4debug(eCOMMITF, "%d.%03d", (int) (data.timestamp / 1000000), (int) ((data.timestamp / 1000) % 1000));

		for (uint32_t i = 0; i < sig_cnt; i++)
		{
			// char fl[20];
			// my_ftoa(data.val[i], fl, 3);
			px4debug(eCOMMITF, ",%f", data.val[i]);
		}
		px4debug(eCOMMITF, "\r\n");

	} while (!f_eof(&f));

	px4debug(eCOMMITF, "=== END ===\r\n");
	f_close(&f);
}


#if FF_FS_REENTRANT
int ff_cre_syncobj(BYTE vol, FF_SYNC_t* sobj)
{
	*sobj = xSemaphoreCreateMutex();
	return (int) (*sobj != NULL);
}

int ff_del_syncobj(FF_SYNC_t sobj)
{
	vSemaphoreDelete(sobj);
	return 1;
}

int ff_req_grant(FF_SYNC_t sobj)
{
	return (int) (xSemaphoreTake(sobj, FF_FS_TIMEOUT) == pdTRUE);
}

void ff_rel_grant(FF_SYNC_t sobj)
{
	xSemaphoreGive(sobj);
}

#endif

