/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id plain.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/07/13 11:23
 * @uses The framework all inlcudes.
 *       This group defines just can define one.
 *             group1: PLAIN_OPEN_ICOP|PLAIN_OPEN_EPOLL
 *
 * ------------------------------------------------------------------------------
 *  Licensing information can be found at the end of the file.
 * ------------------------------------------------------------------------------
*/
#ifndef PLAIN_H_
#define PLAIN_H_

/* basic */
#include "plain/basic/type/variable.h"
#include "plain/basic/base64.h"
#include "plain/basic/endian.h"
#include "plain/basic/global.h"
#include "plain/basic/io.h"
#include "plain/basic/logger.h"
#include "plain/basic/md5.h"
#include "plain/basic/monitor.h"
#include "plain/basic/singleton.h"
#include "plain/basic/stringstream.h"
#include "plain/basic/utility.h"

/* console */
#include "plain/console/application.h"
#include "plain/console/argv_input.h"
#include "plain/console/array_input.h"
#include "plain/console/command.h"
#include "plain/console/generator_command.h"
#include "plain/console/input.h"
#include "plain/console/input_argument.h"
#include "plain/console/input_definition.h"
#include "plain/console/input_option.h"
#include "plain/console/net_output.h"
#include "plain/console/output.h"
#include "plain/console/parser.h"
#include "plain/console/string_input.h"

/* cache */
#include "plain/cache/packet/db_query.h"
#include "plain/cache/packet/db_result.h"
#include "plain/cache/db_define.h"
#include "plain/cache/db_store.h"
#include "plain/cache/manager.h"
#include "plain/cache/repository.h"
#include "plain/cache/storeinterface.h"

/* db */
#include "plain/db/concerns/builds_queries.h"
#include "plain/db/query/grammars/grammar.h"
#include "plain/db/query/grammars/mysql_grammar.h"
#include "plain/db/query/grammars/postgres_grammar.h"
#include "plain/db/query/grammars/sqlite_grammar.h"
#include "plain/db/query/grammars/sqlserver_grammar.h"
#include "plain/db/query/builder.h"
#include "plain/db/query/expression.h"
#include "plain/db/query/join_clause.h"
#include "plain/db/schema/blueprint.h"
#include "plain/db/schema/grammars/change_column.h"
#include "plain/db/schema/grammars/grammar.h"
#include "plain/db/schema/grammars/mysql_grammar.h"
#include "plain/db/schema/grammars/postgres_grammar.h"
#include "plain/db/schema/grammars/rename_column.h"
#include "plain/db/schema/grammars/sqlite_grammar.h"
#include "plain/db/schema/grammars/sqlserver_grammar.h"
#include "plain/db/schema/builder.h"
#include "plain/db/schema/mysql_builder.h"
#include "plain/db/schema/postgres_builder.h"
#include "plain/db/connection.h"
#include "plain/db/grammar.h"
#include "plain/db/seeder.h"
#include "plain/db/define.h"
#include "plain/db/interface.h"
#include "plain/db/factory.h"
#include "plain/db/query.h"

/* engine */
#include "plain/engine/application.h"
#include "plain/engine/kernel.h"

/* events */
#include "plain/events/bus.h"

/* file */
#include "plain/file/api.h"
#include "plain/file/ini.h"
#include "plain/file/tab.h"
#include "plain/file/library.h"

/* net */
#include "plain/net/connection/basic.h"
#include "plain/net/connection/pool.h"
#include "plain/net/connection/manager/interface.h"
#include "plain/net/connection/manager/connector.h"
#include "plain/net/connection/manager/epoll.h"
#include "plain/net/connection/manager/iocp.h"
#include "plain/net/connection/manager/basic.h"
#include "plain/net/connection/manager/connector.h"
#include "plain/net/connection/manager/listener.h"
#include "plain/net/connection/manager/listener_factory.h"
#include "plain/net/packet/interface.h"
#include "plain/net/packet/dynamic.h"
#include "plain/net/packet/factory.h"
#include "plain/net/packet/factorymanager.h"
#include "plain/net/protocol/interface.h"
#include "plain/net/protocol/basic.h"
#include "plain/net/socket/api.h"
#include "plain/net/socket/listener.h"
#include "plain/net/socket/basic.h"
#include "plain/net/stream/basic.h"
#include "plain/net/stream/input.h"
#include "plain/net/stream/output.h"
#include "plain/net/stream/compressor.h"
#include "plain/net/stream/encryptor.h"

/* script */
#include "plain/script/interface.h"
#include "plain/script/factory.h"

/* sys */
#include "plain/sys/memory/static_allocator.h"
#include "plain/sys/memory/dynamic_allocator.h"
#include "plain/sys/memory/share.h"
#include "plain/sys/memory/sharemap.h"
#include "plain/sys/assert.h"
#include "plain/sys/process.h"
#include "plain/sys/thread.h"
#include "plain/sys/util.h"

/* util */
#include "plain/util/compressor/assistant.h"
#include "plain/util/compressor/minimanager.h"
#include "plain/util/bitflag.h"
#include "plain/util/random.h"

#endif //PLAIN_H_

/*
MIT License

Copyright (c) 2023 viticm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
