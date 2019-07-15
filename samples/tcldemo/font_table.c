///////////////////////////////////////////////////////////////////////////////
//
//                          IMPORTANT NOTICE
//
// The following open source license statement does not apply to any
// entity in the Exception List published by FMSoft.
//
// For more information, please visit:
//
// https://www.fmsoft.cn/exception-list
//
//////////////////////////////////////////////////////////////////////////////

/*
 * Copyright (C) 2000-2007 Beijing Komoxo Inc.
 * All rights reserved.
 */

#include "fzxhgb_yh_ttf.h"
#include "dejavu_sans_ttf.h"

/* Script Easy font description type definition */

typedef struct{
    const char      *font_name;
    const int        font_id;
} se_font_name_map;

typedef struct
{
    int                        font_table_size;
    int                        font_name_map_size;
    const se_font_name_map     *name_map;
    const void                 **font_table;
} se_font_description;


/* Script Easy font description data definition */

static const void* font_table[] =
{
    &dejavu_sans_ttf,

    &fzxhgb_yh_ttf,

    0
};

#define FONT_TABLE_SIZE (sizeof(font_table) / sizeof(void*) - 1)

static const se_font_name_map font_name_map[] = {
    //{"ttf-dejuvu_sans-rrncnn-8-16-iso8859-1", 0},
    //{"ttf-fzxhgb_yh-rrncnn-*-16-GB2312", 1},
    {"sef-dejuvu_sans-rrncnn-0-0-iso8859-1", 0},
    {"sef-fzxhgb_yh-rrncnn-0-0-iso8859-1", 0},
    {"sef-fzxhgb_yh-rrncnn-0-0-GB2312,UTF-8", 1},
};

#define FONT_NAME_MAP_SIZE (sizeof(font_name_map) / sizeof(se_font_name_map))

const se_font_description se_font_desc = {
    FONT_TABLE_SIZE,
    FONT_NAME_MAP_SIZE,
    font_name_map,
    font_table
};

/* The following is the scripteasy mem pool definition */

#define SE_MEM_POOL_SIZE	        (64 * 1024)
#define SE_CACHE_MEM_POOL_SIZE      (256 * 1024)

int   se_minigui_mem_pool[SE_MEM_POOL_SIZE];
int   se_minigui_mem_pool_size = SE_MEM_POOL_SIZE;
int   se_minigui_cache_mem_pool[SE_CACHE_MEM_POOL_SIZE];
int   se_minigui_cache_mem_pool_size = SE_CACHE_MEM_POOL_SIZE;


