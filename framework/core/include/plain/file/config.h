/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 * $Id config.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2023- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.it@gmail.com>
 * @date 2023/04/01 21:05
 * @uses common file base config
 */
#ifndef PLAIN_FILE_CONFIG_H_
#define PLAIN_FILE_CONFIG_H_

#include "plain/basic/config.h"

//定义此宏则文件数据库读取的字符串将从GBK转为UTF-8
#define FILE_DATABASE_CONVERT_GBK_TO_UTF8
//文件数据标识，只有该类型的二进制文件才会认为正确
#define FILE_DATABASE_INDENTIFY 0XDDBBCC00 

#endif //PLAIN_FILE_CONFIG_H_
