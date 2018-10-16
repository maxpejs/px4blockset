/****************************************************************************
*
*   Copyright (C) 2018, Max Pejs <max.pejs@googlemail.com>
*	All rights reserved.
*
* 	Redistribution and use in source and binary forms, with or without
*	modification, are permitted provided that the following conditions
*	are met:
*
*	1. 	Redistributions of source code must retain the above copyright
*		notice, this list of conditions and the following disclaimer.
*
*	2. 	Redistributions in binary form must reproduce the above copyright
*		notice, this list of conditions and the following disclaimer in
*		the documentation and/or other materials provided with the
*		distribution.
*
*	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*	FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*	COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
*	OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
*	AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*	THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
*	DAMAGE.
*****************************************************************************/

#include "sd_card_logger.h"

#include <FatFs/Core/ff_gen_drv.h>
#include <FatFs/Drivers/sd_diskio.h>

#define SIGNAL_MAX_CNT 			10
#define FILENAME_MAX_LENGTH 	13
#define FILE_LOGGER_MAX_CNT 	_FS_LOCK

#define WRITE_ASCII_FORMATTED	DISABLE
#define SD_RING_BUFF_SIZE		1000

typedef struct
{
	uint64_t timestamp;
	float val[SIGNAL_MAX_CNT];
}sd_write_data_st;

typedef struct
{
	uint32_t read;
	uint32_t write;
	sd_write_data_st buff[SD_RING_BUFF_SIZE];
}sd_data_ring_buff;

typedef struct
{
	FIL		 	fi;     	/* File object */
	char 		filename[FILENAME_MAX_LENGTH];
	char 		filename_prefix[FILENAME_MAX_LENGTH];
	uint32_t 	id;
	uint32_t 	sample_time;
	uint32_t 	sample_cnt;
	uint32_t 	sig_cnt;
	uint32_t 	runtime;
	uint64_t	tick_last_call;
	uint32_t 	file_logger_state;
	sd_data_ring_buff rbuff;

}sd_card_file_info_st;

/* FatFs variables */
FATFS 	_sd_file_system;  			/* File system object for SD card logical drive */

char 	_drive_path[4]; 			/* SD card logical drive path */
static uint32_t _runtime 			= 0U;
static uint32_t _module_state 		= DISABLE;
static uint32_t _file_logger_cnt 	= 0;

static sd_card_file_info_st * file_data_arr[FILE_LOGGER_MAX_CNT];

static uint32_t 	_sd_card_logger_init_board();
static uint32_t 	_sd_card_logger_init_sdcard();
static uint32_t 	_get_next_id_for_logfile(char * filename_prefix);
static ErrorStatus 	_create_new_log_file(sd_card_file_info_st * info);
static void 		_write_to_sd_card(sd_card_file_info_st * info);
static void 		_delete_all_files();
static void 		_list_single_file(char * path);
static void 		_px4_sd_card_logger_stop();
static void 		_px4_sd_card_logger_resume();
static FRESULT 		_scan_files(char* path);


static inline uint32_t 	_ring_buffer_empty(sd_data_ring_buff * b);
static uint32_t 	_ring_buffer_free_space(sd_data_ring_buff * b);


void px4_sd_card_logger_init()
{
	_sd_card_logger_init_board();
	_sd_card_logger_init_sdcard();

	memset(file_data_arr, 0 , sizeof(file_data_arr));

	debug_print_string("sd card logger init ok\r\n");
	_module_state = ENABLE;
}

uint32_t px4_sd_card_logger_add_new_logger(uint32_t sampleTime, uint32_t sigCnt, char * file_name)
{
	if (_file_logger_cnt >= FILE_LOGGER_MAX_CNT)
	{
		debug_print_string("ERROR! sd card logger. max logger counter limit reached \r\n");
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
		debug_print_string("#Error creating new log file... Disabling SD Card module! \r\n");
		_module_state = DISABLE;
	}

	fi->file_logger_state = ENABLE;

	file_data_arr[_file_logger_cnt] = fi;
	_file_logger_cnt++;

	debug_print_string("new logger created ");
	debug_print_string(fi->filename);
	debug_print_string("\r\n");

	return ret;
}

void px4_sd_card_logger_add_val(uint32_t log_id, float * values)
{
	uint64_t ts = tic();

	if (log_id >= FILE_LOGGER_MAX_CNT)
	{
		return;
	}

	if (file_data_arr[log_id] == 0)
	{
		debug_print_string("error at px4_sd_card_logger_set => NULL-pointer access!\r\n");
		return;
	}

	// check if cycle time elapsed
	if ((ts / 1000) < (file_data_arr[log_id]->sample_time * file_data_arr[log_id]->sample_cnt))
	{
		return;
	}

	// init counter of samples for exact adding values according to sampling time
	if(file_data_arr[log_id]->sample_cnt == 0)
	{
		file_data_arr[log_id]->sample_cnt = (uint32_t) (ts / 1000) / file_data_arr[log_id]->sample_time;
	}

	file_data_arr[log_id]->sample_cnt++;
	file_data_arr[log_id]->tick_last_call = ts/1000;

	if (_ring_buffer_free_space(&(file_data_arr[log_id]->rbuff)) > 0)
	{
		uint32_t idx = file_data_arr[log_id]->rbuff.write;

		memcpy(file_data_arr[log_id]->rbuff.buff[idx].val, values, sizeof(float) * file_data_arr[log_id]->sig_cnt);
		file_data_arr[log_id]->rbuff.buff[idx].timestamp = ts;

		file_data_arr[log_id]->rbuff.write = ++idx % SD_RING_BUFF_SIZE;
	}
	else
	{
		// debug_print_string("sd card logger. not enough free buffer space\r\n");
	}

	// debug_print_string("added new sample!\r\n");
}

void px4_sd_card_logger_task(void)
{
	_runtime = 0;

	if (_module_state == DISABLE)
	{
		return;
	}

	for (uint32_t i = 0; i < _file_logger_cnt; i++)
	{
		if (file_data_arr[i] == 0)
		{
			debug_print_string("error at px4_sd_card_logger_task => NULL-pointer access!");
			_module_state = DISABLE;
			continue;
		}

		if (file_data_arr[i]->file_logger_state == DISABLE)
		{
			continue;
		}

		_write_to_sd_card(file_data_arr[i]);

		_runtime += file_data_arr[i]->runtime;
	}
}

static void _write_to_sd_card(sd_card_file_info_st * info)
{
	FRESULT 	fr;
	UINT 		writtenbytes = 0;
	uint32_t 	packagesize;
	uint32_t	idx;
	uint64_t 	timestampt = 0;

	uint32_t elem_cnt = 1000;

	packagesize = sizeof(uint64_t) + info->sig_cnt * sizeof(float);

//	debug_print_string("Buff len:");
//	debug_print_int(SD_RING_BUFF_SIZE - _ring_buffer_free_space(&(info->rbuff)) - 1);
//	debug_print_string("\r\n");

	while (!_ring_buffer_empty(&(info->rbuff)) && elem_cnt-- > 0)
	{
		if (elem_cnt == 999)
		{
			timestampt = tic();
		}

		idx =info->rbuff.read;

		/* copy data into temp array to keep time for minimize time inside critical section */
		if (WRITE_ASCII_FORMATTED == ENABLE)
		{
//			char arr[100];
//			uint64_t ts = info->rbuff.buff[idx].timestamp;
//
//			snprintf(arr, sizeof(arr), "%d.%03d", (int) (ts / 1000000), (int) ((ts / 1000) % 1000));
//
//			for (uint32_t i = 0; i < info->sig_cnt; i++)
//			{
//				char fl[20];
//				my_ftoa(info->rbuff.buff[idx].val[i], fl, 20, 3);
//				snprintf(&arr[strlen(arr)], sizeof(arr) - strlen(arr), "%s", fl);
//			}
//
//			snprintf(&arr[strlen(arr)], sizeof(arr) - strlen(arr), "\r\n");
//
//			f_puts(arr, &(info->fi));
//			f_sync(&(info->fi));
		}
		else
		{
			fr = f_write(&(info->fi), (const void*) &info->rbuff.buff[idx], packagesize, &writtenbytes);
			f_sync(&(info->fi));

			if ((fr != FR_OK) || (writtenbytes != packagesize))
			{
				debug_print_string("Error at writing data entry, or number of written bytes doesn't match\r\n");
			}
		}

		info->rbuff.read = ++idx % SD_RING_BUFF_SIZE;
	}

	info->runtime = (timestampt == 0) ? info->runtime : (uint32_t) toc(timestampt);
}

static uint32_t _sd_card_logger_init_board()
{
	return SUCCESS;
}

static uint32_t _sd_card_logger_init_sdcard()
{
	// Link the micro SD disk I/O driver
	if (FATFS_LinkDriver(&SD_Driver, _drive_path) == 0)
	{
		debug_print_string("SD Drive ");
		debug_print_string(_drive_path);
		debug_print_string("  linked\r\n");
	}

	// Register the file system object to the FatFs module
	if (f_mount(&_sd_file_system, (TCHAR const*) _drive_path, 0) != FR_OK)
	{
		/* FatFs Initialization Error */
		debug_print_string("#Error mount drive\r\n");
		return ERROR;
	}
	return SUCCESS;
}

static ErrorStatus _create_new_log_file(sd_card_file_info_st * info)
{
	FRESULT 	res = 0;
	uint32_t 	log_idx;
	UINT 		writtenbytes;

	log_idx = _get_next_id_for_logfile(info->filename_prefix);
	snprintf(info->filename, 32, "%s%03d", info->filename_prefix, (int) log_idx);

	res = f_open(&(info->fi), info->filename, FA_CREATE_ALWAYS | FA_WRITE);
	f_sync(&(info->fi));


	if (WRITE_ASCII_FORMATTED == ENABLE)
	{
//		char buff[30];
//		snprintf(buff, sizeof(buff), "%i %i\r\n", (int)info->sig_cnt, (int)info->sample_time);
//		f_puts(buff, &(info->fi));
//		f_sync(&(info->fi));
	}
	else
	{
		res |= f_write(&(info->fi), &(info->sig_cnt), sizeof(info->sig_cnt), &writtenbytes);
		f_sync(&(info->fi));

		res |= f_write(&(info->fi), &(info->sample_time), sizeof(info->sample_time), &writtenbytes);
		f_sync(&(info->fi));

		if ((res != FR_OK) || (writtenbytes != sizeof(info->sig_cnt)))
		{
			debug_print_string("#Error open file, or error at writing init block\r\n");
			f_close(&(info->fi));
		}

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
		debug_print_string("open dir failed\r\n");
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

void px4_sd_card_logger_process_cmd(const char * cmd)
{
	char 		path[20];

	if (strncmp(cmd, "list all", strlen("list all")) == 0)
	{
		comm_itf_print_string("--- file list start ---\r\n");

		// TODO print file size and runtime length
		_scan_files("/");
		comm_itf_print_string("--- file list end ---\r\n");
	}
	else if (strncmp(cmd, "list ", strlen("list ")) == 0)
	{
		path[0] = '/';
		_copy_file_name_from_cmd(cmd, &path[1]);
		_list_single_file(path);
	}
	else if (strncmp(cmd, "del all", strlen("del all")) == 0)
	{
		comm_itf_print_string("==> Delete all files found on sd card ...");
		_delete_all_files();
		comm_itf_print_string(" done \r\n");
	}
	else if (strncmp(cmd, "del ", strlen("del ")) == 0)
	{
		path[0] = '/';
		_copy_file_name_from_cmd(cmd, &path[1]);

		comm_itf_print_string("==> deleting file ");
		comm_itf_print_string(path);
		comm_itf_print_string("... ");

		if (f_unlink(path) != FR_OK)
		{
			comm_itf_print_string(" Error \r\n");
		}
		else
		{
			comm_itf_print_string(" ok\r\n");
		}
	}
	//---------------------------------------------------------------------------
	else
	{
		comm_itf_print_string("UNKNOWN SD command received! Type <help> for more information\r\n");
	}
}

static FRESULT _scan_files(char* path)
{
	FRESULT res;
	FIL f;
	DIR dir;
	static FILINFO fno;
	char buffArr[100];

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
				// debug_print_string("end of files\r\n");
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
//					debug_print_string("sample time: "); debug_print_int(sampletime); debug_print_string("\r\n");
//					debug_print_string("fno.fsize: "); debug_print_int(fno.fsize); debug_print_string("\r\n");
//					debug_print_string("timeline: "); debug_print_int(timeline); debug_print_string("\r\n");

					timeline = (timeline * sampletime) / 1000;
				}
			}
			else if (res == FR_LOCKED)
			{
//				debug_print_string("Error. File used\r\n");
//				debug_print_string(fno.fname);
//				debug_print_string("\r\n");
			}
			else
			{
//				 debug_print_string("error on f_open code:");
//				 debug_print_int(res);
//				 debug_print_string("\r\n");
			}

			f_close(&f);

			sprintf(buffArr, "/%s|%i|%i|%i|%i|%i\r\n", fno.fname, (int) fno.fsize,
					(int) (timeline / 60), (int) (timeline - (timeline / 60) * 60), (int) sig_cnt, (int) sampletime);

			comm_itf_print_string(buffArr);
		}

		f_closedir(&dir);
	}
	else
	{
		debug_print_string("open dir failed\r\n");
	}

	return res;
}

uint32_t px4_sd_card_logger_getruntime(void)
{
	return _runtime;
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
			debug_print_string("error at px4_sd_card_logger_task => NULL-pointer access!");
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
				debug_print_string("error at closing file on px4_sd_card_logger_stop");
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
			debug_print_string("error at px4_sd_card_logger_task => NULL-pointer access!");
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
				debug_print_string("#Error creating new log file... Disabling SD Card module! \r\n");
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

			if (_FS_RPATH && fno.fname[0] == '.')
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
				comm_itf_print_string("Error! ");
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
	sd_write_data_st data;
	UINT bytesread_sigcnt, bytesread_sampletime;
	char arr[150];
	uint32_t sig_cnt, sampletime;
	uint32_t packagesize;

//	comm_itf_print_string("==> list file ");
//	comm_itf_print_string(path);
//	comm_itf_print_string(" ... \r\n");

	// TODO: Check if file is current opened for logging
	if ( FILE_LOGGER_MAX_CNT == _file_logger_cnt)
	{
		comm_itf_print_string("Error, max count of parallel allowed opened files is reached\r\n");
		return;
	}

	res = f_open(&f, path, FA_READ);
	f_sync(&f);

	if (res == FR_NO_FILE)
	{
		comm_itf_print_string("Error listing file => File doesn't exist \r\n");
		return;
	}
	else if (res == FR_LOCKED)
	{
		comm_itf_print_string("Error listing file => File currently used for logging \r\n");
		return;
	}
	else if (res != FR_OK)
	{
		comm_itf_print_string("Error listing file => Unknown case: ");
		comm_itf_print_int(res);
		comm_itf_print_string("\r\n");
		f_close(&f);
		return;
	}

	res = f_read(&f, &sig_cnt, sizeof(sig_cnt), &bytesread_sigcnt);
	f_sync(&f);

	res |= f_read(&f, &sampletime, sizeof(sampletime), &bytesread_sampletime);
	f_sync(&f);

	if ((res != FR_OK) || (bytesread_sigcnt != sizeof(sig_cnt)) || (bytesread_sampletime != sizeof(sampletime)))
	{
		comm_itf_print_string("Error reading sig cnt");
		f_close(&f);
		return;
	}

	packagesize = sizeof(data.timestamp) + sig_cnt * sizeof(data.val[0]);

//	comm_itf_print_string("signal counter: ");
//	comm_itf_print_int(sig_cnt);
//	comm_itf_print_string("\r\n");

	comm_itf_print_string("=== BEGIN ===\r\n");

	if ( WRITE_ASCII_FORMATTED == ENABLE)
	{
//		do
//		{
//			res = f_read(&f, (void*) &data, packagesize, &bytesread_sigcnt);
//			f_sync(&f);
//
//			if ((res != FR_OK) || (bytesread_sigcnt != packagesize))
//			{
//				comm_itf_print_string("Error reading file");
//				f_close(&f);
//				return;
//			}
//
//			snprintf(arr, sizeof(arr), "%d,%03d", (int) (data.timestamp / 1000000), (int) ((data.timestamp / 1000) % 1000));
//			comm_itf_print_string(arr);
//
//			for (uint32_t i = 0; i < sig_cnt; i++)
//			{
//				char fl[20];
//				my_ftoa(data.val[i], fl, 20, 3);
//
//				snprintf(arr, sizeof(arr), "\t%s", fl);
//				comm_itf_print_string(arr);
//			}
//			comm_itf_print_string("\r\n");
//
//		} while (!f_eof(&f));
	}
	else
	{
		do
		{
			res = f_read(&f, (void*) &data, packagesize, &bytesread_sigcnt);
			f_sync(&f);

			if ((res != FR_OK) || (bytesread_sigcnt != packagesize))
			{
				comm_itf_print_string("Error reading file");
				f_close(&f);
				return;
			}

			snprintf(arr, sizeof(arr), "%d.%03d", (int) (data.timestamp / 1000000), (int) ((data.timestamp / 1000) % 1000));
			comm_itf_print_string(arr);

			for (uint32_t i = 0; i < sig_cnt; i++)
			{
				char fl[20];
				my_ftoa(data.val[i], fl, 3);

				snprintf(arr, sizeof(arr), ",%s", fl);
				comm_itf_print_string(arr);
			}
			comm_itf_print_string("\r\n");

		} while (!f_eof(&f));
	}

	comm_itf_print_string("=== END ===\r\n");
	f_close(&f);
}

static inline uint32_t _ring_buffer_empty(sd_data_ring_buff * b)
{
	return b->read == b->write;
}

static uint32_t _ring_buffer_free_space(sd_data_ring_buff * b)
{
	return (SD_RING_BUFF_SIZE + b->read - b->write - 1) % SD_RING_BUFF_SIZE;
}
