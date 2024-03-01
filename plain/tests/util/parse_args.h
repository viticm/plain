/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id parse_args.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2024 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2024/02/26 20:11
 * @uses The parse args from command line utility functions.
 */

#ifndef PARSE_ARGS_H_
#define PARSE_ARGS_H_

#include "plain/net/config.h"

bool parse_net_setting(
  plain::net::setting_t &setting, int32_t argc, char **argv) noexcept;

#endif // PARSE_ARGS_H_
