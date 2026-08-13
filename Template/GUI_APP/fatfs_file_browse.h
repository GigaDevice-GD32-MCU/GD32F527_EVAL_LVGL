
#ifndef FATFS_FILE_BROWSE_H_
#define FATFS_FILE_BROWSE_H_

#include "ff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lvgl.h"

#define FILE_NUM    5  /* 显示页面文件的最大数量 */
#define FOLDER_NUM  5  /* 显示页面文件夹的最大数量 */
#define NAME_LENGTH 64  /* 文件/文件夹名称的最大长度(含结束符) */
#define PATH_LENGTH 128 /* 路径的最大长度(含结束符) */
#define PATH_LEN  PATH_LENGTH

/*  FLASH_ROOT */
/*  SD_ROOT    */
typedef struct {
    int folderlen;
    lv_obj_t *Folder_btn[];
} Folder_btnArray;

typedef struct {
    int filelen;
    lv_obj_t *File_btn[];
} File_btnArray;

uint16_t scan_files(char *path, char *buff);
void scanfiles_test(void);
uint8_t mf_scan_file(char *path);
void refersh_parameter(void);
void deinit_list(void);
void deinit_list2(void);
uint8_t get_back_path(void);

/* 安全地把一个子目录名追加到当前路径上，长度不够时返回 false 且不修改路径 */
bool path_push(const char *name);

#endif /* FATFS_FILE_BROWSE_H_ */
