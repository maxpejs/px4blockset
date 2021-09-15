#include "sd_card_logger.h"
#include <logger_ring_buffer.h>
#include <FatFs/Core/ff_gen_drv.h>
#include <sd_diskio.h>
// #include <cmsis_os.h>

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#define FILENAME_MAX_LENGTH 		13
#define MIN_PACKAGE_SIZE_CNT 		8
#define WRITE_BLOCK_SIZE_BYTES		512

typedef struct
{
	FIL fobj;
	char filename[FILENAME_MAX_LENGTH];
	char filename_prefix[FILENAME_MAX_LENGTH];
	uint32_t sample_time;
	uint32_t sample_cnt;
	uint32_t sig_cnt;
	uint32_t tick_last_call;
	ring_buff_data_st rbuff;

} sd_card_file_info_st;

typedef struct
{
	uint32_t signal_cnt;
	uint32_t sample_time;

} sd_card_file_meta_info_st;

/* FatFs variables */
FATFS _file_system; /* File system object for SD card logical drive */

static char _drive_path[4]; /* SD card logical drive path */
static uint32_t _module_state = DISABLE;
static uint32_t _state_before_stop = DISABLE;

static char _usercmd[20];
static int _new_command_received = 0;

static sd_card_file_info_st file_info;

static uint32_t _sd_card_logger_init_sdcard();
static uint32_t _get_next_id_for_logfile(char * filename_prefix);
static ErrorStatus _create_new_log_file();
static void _write_to_sd_card();
static void _delete_all_files();
static void _list_single_file(char * path);
static void _px4_sd_card_logger_stop();
static void _px4_sd_card_logger_resume();
static void _px4_sd_card_logger_process_cmd();
static FRESULT _scan_files(char* path);

void px4_sd_card_logger_init()
{
	_sd_card_logger_init_sdcard();
	memset(&file_info, 0, sizeof(file_info));
	px4debug("sd card logger init ok\n");
	_module_state = ENABLE;
}

void px4_sd_card_logger_add_new_logger(uint32_t sampleTime, uint32_t sigCnt, char * file_name)
{
	file_info.rbuff.read = 0;
	file_info.rbuff.write = 0;
	file_info.sample_time = sampleTime;
	file_info.sig_cnt = (sigCnt > SIGNAL_MAX_CNT) ? SIGNAL_MAX_CNT : sigCnt;

	strncpy(file_info.filename_prefix, file_name, FILENAME_MAX_LENGTH - 4); // 3 chars for id, 1 for termination

	if (_create_new_log_file(&file_info) == ERROR)
	{
		px4debug("#Error creating new log file... Disabling SD Card module! \n");
		_module_state = DISABLE;
	}
	else
	{
		px4debug("new logger %s created \n", file_info.filename);
	}
}

void px4_sd_card_logger_add_val(float * values)
{
	if (_module_state == DISABLE)
	{
		return;
	}

	uint32_t ts = tic();

	// check if cycle time elapsed
	if ((ts / 1000) < (file_info.sample_time * file_info.sample_cnt))
	{
		return;
	}

	// init counter of samples for exact timing for adding values according to sampling time
	if (file_info.sample_cnt == 0)
	{
		file_info.sample_cnt = (uint32_t) (ts / 1000) / file_info.sample_time;
	}

	if (ring_buffer_free_space(&(file_info.rbuff)) > 0)
	{
		file_info.sample_cnt++;
		file_info.tick_last_call = ts / 1000;

		uint32_t idx = file_info.rbuff.write;

		memcpy(file_info.rbuff.buff[idx].val, values, sizeof(float) * file_info.sig_cnt);
		file_info.rbuff.buff[idx].timestamp = ts;

		file_info.rbuff.write = ++idx % RING_BUFF_SIZE;
	}
	else
	{
		px4debug("sd card logger. not enough free buffer space\n");
	}
}

void px4_sd_card_logger_task(void)
{
	_px4_sd_card_logger_process_cmd();

	if (_module_state == ENABLE)
	{
		_write_to_sd_card();
	}
}

static void _write_to_sd_card()
{
	uint32_t idxRead, idxWrite;

	while (ring_buffer_count(&(file_info.rbuff)) >= MIN_PACKAGE_SIZE_CNT)
	{
		// px4debug("b.size %d r %d w %d", ring_buffer_count(&(info->rbuff)), info->rbuff.read, info->rbuff.write);

		idxRead = file_info.rbuff.read;
		idxWrite = file_info.rbuff.write;

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
		}

		numofsamples = numofsamples > MIN_PACKAGE_SIZE_CNT ? MIN_PACKAGE_SIZE_CNT : numofsamples;

		UINT totalBytesToWrite = sizeof(log_dataset_st) * numofsamples;

		UINT wBytesTotal = 0;
		UINT restBytesCnt = totalBytesToWrite;

		FRESULT fr = f_open(&file_info.fobj, file_info.filename, FA_OPEN_APPEND | FA_WRITE);
		f_sync(&file_info.fobj);
		if (fr != FR_OK)
		{
			px4debug("Error opening file \n");
			f_close(&file_info.fobj);
			return;
		}

		int loop_cnt = 0;
		do
		{
			UINT wBytes = 0;
			fr = f_write(&file_info.fobj, (const void*) ((&file_info.rbuff.buff[idxRead]) + wBytesTotal / sizeof(int)), restBytesCnt,
					&wBytes);
			f_sync(&file_info.fobj);

			wBytesTotal += wBytes;
			restBytesCnt -= wBytes;

			if (fr != FR_OK || restBytesCnt > 0)
			{
				size_t fistBytePos = (size_t) &file_info.rbuff.buff[idxRead];
				size_t currBytePos = (size_t) (&(file_info.rbuff.buff[idxRead])) + wBytesTotal;

				px4debug("written %d totalwritten %d total %d left %d\n", wBytes, wBytesTotal, totalBytesToWrite, restBytesCnt);
				px4debug("fistBytePos %d currBytePos %d\n", fistBytePos, currBytePos);
			}

			loop_cnt++;
		} while (restBytesCnt > 0 && loop_cnt < 3);

		f_close(&file_info.fobj);

		if (loop_cnt >= 3)
		{
			px4debug("err writing data\n");
			// _module_state = DISABLE;
			return;
		}

		file_info.rbuff.read = (idxRead + numofsamples) % RING_BUFF_SIZE;
	}
}

static uint32_t _sd_card_logger_init_sdcard()
{
// Link the micro SD disk I/O driver
	if (FATFS_LinkDriver(&SD_Driver, _drive_path) == 0)
	{
		px4debug("SD Drive %s linked \n", _drive_path);
	}

// Register the file system object to the FatFs module
	if (f_mount(&_file_system, (TCHAR const*) _drive_path, 0) != FR_OK)
	{
		/* FatFs Initialization Error */
		px4debug("Error mount sd card drive! \n");
		return ERROR;
	}
	return SUCCESS;
}

static ErrorStatus _create_new_log_file()
{
	px4debug("sdcard. create new logger file ... ");

	FRESULT res = 0;
	uint32_t log_idx;
	UINT writtenbytes;

	log_idx = _get_next_id_for_logfile(file_info.filename_prefix);
	snprintf(file_info.filename, 32, "%s%03d", file_info.filename_prefix, (int) log_idx);

	sd_card_file_meta_info_st mInfo = { file_info.sig_cnt, file_info.sample_time };

	// create and open new log file
	res = f_open(&file_info.fobj, file_info.filename, FA_CREATE_ALWAYS | FA_WRITE);

	// write number of signals sampling time of block as meta info
	res |= f_write(&file_info.fobj, &mInfo, sizeof(mInfo), &writtenbytes);
	f_sync(&(file_info.fobj));

	// check results
	if ((res != FR_OK) || (writtenbytes != sizeof(mInfo)))
	{
		px4debug("#Error open file, or error at writing file meta block\n");
	}
	else
	{
		px4debug("done \n");
	}

	f_close(&file_info.fobj);

	return (res == FR_OK) ? SUCCESS : ERROR;
}

static uint32_t _get_next_id_for_logfile(char * filename_prefix)
{
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
							ret = t + 1;
						}
					}
				}
			}
		}
		f_closedir(&dir);
	}
	else
	{
		px4debug("open dir failed \n");
	}

	return ret;
}

static void _copy_file_name_from_cmd(const char * cmd, char * file)
{
	while (*cmd != 0 && *cmd != ' ')
		cmd++;

	if (*cmd == 0)
		return;

	cmd++;

	while (*cmd != '\r' && *cmd != '\n' && *cmd != 0)
		*file++ = *cmd++;

	*file = 0;
}

void px4_sd_card_logger_add_user_cmd(const char * cmd)
{
	memcpy(_usercmd, cmd, strlen(cmd) + 1);
	_new_command_received = 1;
	px4debug("SDCARD: command received \n");
}

void _px4_sd_card_logger_process_cmd()
{
	if (_new_command_received == 0)
	{
		return;
	}

	_px4_sd_card_logger_stop();

	char path[20];

	if (strncmp(_usercmd, "list all", strlen("list all")) == 0)
	{
		px4debug("--- file list start ---\n");
		_scan_files("/");
		px4debug("--- file list end ---\n");
	}
	else if (strncmp(_usercmd, "list ", strlen("list ")) == 0)
	{
		path[0] = '/';
		_copy_file_name_from_cmd(_usercmd, &path[1]);
		_list_single_file(path);
	}
	else if (strncmp(_usercmd, "del all", strlen("del all")) == 0)
	{
		px4debug("==> Delete all files found on sd card ...");
		_delete_all_files();
		px4debug(" done \n");
	}
	else if (strncmp(_usercmd, "del ", strlen("del ")) == 0)
	{
		path[0] = '/';
		_copy_file_name_from_cmd(_usercmd, &path[1]);

		px4debug("==> deleting file ");
		px4debug(path);
		px4debug("... ");

		if (f_unlink(path) != FR_OK)
		{
			px4debug("Error \n");
		}
		else
		{
			px4debug(" ok\n");
		}
	}
//---------------------------------------------------------------------------
	else
	{
		px4debug("UNKNOWN SD command received! Type <help> for more information\n");
	}

	_new_command_received = 0;
	_px4_sd_card_logger_resume();
}

static FRESULT _scan_files(char* path)
{
	FRESULT res;
	FIL f;
	DIR dir;

	res = f_opendir(&dir, path); /* Open the directory */
	if (res == FR_OK)
	{
		for (;;)
		{
			FILINFO fno;
			res = f_readdir(&dir, &fno); /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0)
			{
				px4debug("end of files \n");
				break; /* Break on error or end of dir */
			}

			res = f_open(&f, fno.fname, FA_READ);
			f_sync(&f);

			if (res == FR_OK)
			{
				sd_card_file_meta_info_st mInfo = { 0, 0 };

				UINT bytesread = 0;
				res = f_read(&f, &mInfo, sizeof(mInfo), &bytesread);

				if ((res == FR_OK) && (bytesread == sizeof(mInfo)))
				{
					uint32_t sampletime = mInfo.sample_time;
					uint32_t signal_cnt = mInfo.signal_cnt;

					uint32_t timeline = (fno.fsize - 8) / (8 + signal_cnt * 4);
					timeline = (timeline * sampletime) / 1000;
					px4debug("/%s|%i|%i|%i|%i|%i\n", fno.fname, (int) fno.fsize, (int) (timeline / 60),
							(int) (timeline - (timeline / 60) * 60), (int) signal_cnt, (int) sampletime);
				}
				else
				{
					px4debug("Error read meta data res:%d bytesread%d \n", res, bytesread);
				}

				f_close(&f);
			}
			else if (res == FR_LOCKED)
			{
				px4debug("Error. File is in use: %s \n", fno.fname);
			}
			else
			{
				px4debug("error on f_open, code:%d \n", res);
			}

		}

		f_closedir(&dir);
	}
	else
	{
		px4debug("open directory failed\n");
	}

	return res;
}

static void _px4_sd_card_logger_stop()
{
	_state_before_stop = _module_state;
	_module_state = DISABLE;
}

static void _px4_sd_card_logger_resume()
{
	_module_state = _state_before_stop;
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
				px4debug("Error! ");
			}
		}
		f_closedir(&dir);
	}

	_create_new_log_file();
	_px4_sd_card_logger_resume();
}

static void _list_single_file(char * path)
{
	px4debug("SDCARD: start list single file \n");
	_px4_sd_card_logger_stop();

	FIL f;
	FRESULT res;

	res = f_open(&f, path, FA_READ);
	f_sync(&f);

	if (res == FR_NO_FILE)
	{
		px4debug("Error listing file => File doesn't exist \n");
		_px4_sd_card_logger_resume();
		return;
	}
	else if (res == FR_LOCKED)
	{
		px4debug("Error listing file => File currently used for logging \n");
		_px4_sd_card_logger_resume();
		return;
	}
	else if (res != FR_OK)
	{
		px4debug("Error listing file => Unknown case:%d \n", res);
		f_close(&f);
		_px4_sd_card_logger_resume();
		return;
	}

	UINT readBytes = 0;
	sd_card_file_meta_info_st mInfo = { 0, 0 };

	res = f_read(&f, (void*) (&mInfo), sizeof(mInfo), &readBytes);
	f_sync(&f);

	if ((res != FR_OK) || (readBytes != sizeof(mInfo)))
	{
		px4debug("Error reading meta data (signal counter + sample time");
		f_close(&f);
		return;
	}

	px4debug("=== BEGIN ===\n");

	log_dataset_st data;
	uint32_t packagesize = sizeof(log_dataset_st);
	do
	{
		readBytes = 0;
		res = f_read(&f, (void*) (&data), packagesize, &readBytes);
		f_sync(&f);

		if ((res != FR_OK) || (readBytes != packagesize))
		{
			px4debug("Error reading file");
			f_close(&f);
			break;
		}

		px4debug("%d.%03d", (int) (data.timestamp / 1000000), (int) ((data.timestamp / 1000) % 1000));

		for (uint32_t i = 0; i < mInfo.signal_cnt; i++)
		{
			px4debug(",%f", data.val[i]);
		}
		px4debug("\n");

	} while (!f_eof(&f));

	px4debug("=== END ===\n");
	f_close(&f);

	_px4_sd_card_logger_resume();
}
