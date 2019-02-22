/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id pf.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2017/07/13 11:23
 * @uses The framework all inlcudes.
 *       This group defines just can define one.
 *             group1: PF_OPEN_ICOP|PF_OPEN_EPOLL
*/
#ifndef PF_H_
#define PF_H_

/* basic */
#include "pf/basic/hashmap/template.h"
#include "pf/basic/type/variable.h"
#include "pf/basic/base64.h"
#include "pf/basic/global.h"
#include "pf/basic/io.tcc"
#include "pf/basic/logger.h"
#include "pf/basic/md5.h"
#include "pf/basic/monitor.h"
#include "pf/basic/rang.h"
#include "pf/basic/rlutil.h"
#include "pf/basic/singleton.tcc"
#include "pf/basic/string.h"
#include "pf/basic/stringstream.h"
#include "pf/basic/time_manager.h"
#include "pf/basic/tinytimer.h"
#include "pf/basic/util.h"

/* cache */
#include "pf/cache/packet/db_query.h"
#include "pf/cache/packet/db_result.h"
#include "pf/cache/db_define.h"
#include "pf/cache/db_store.h"
#include "pf/cache/manager.h"
#include "pf/cache/repository.h"
#include "pf/cache/storeinterface.h"

/* db */
#include "pf/db/concerns/builds_queries.h"
#include "pf/db/query/grammar.h"
#include "pf/db/query/mysql_grammar.h"
#include "pf/db/query/postgres_grammar.h"
#include "pf/db/query/sqlite_grammar.h"
#include "pf/db/query/sqlserver_grammar.h"
#include "pf/db/query/builder.h"
#include "pf/db/query/expression.h"
#include "pf/db/query/join_clause.h"
#include "pf/db/schema/grammars/change_column.h"
#include "pf/db/schema/grammars/grammar.h"
#include "pf/db/schema/grammars/mysql_grammar.h"
#include "pf/db/schema/grammars/postgres_grammar.h"
#include "pf/db/schema/grammars/rename_column.h"
#include "pf/db/schema/grammars/sqlite_grammar.h"
#include "pf/db/schema/grammars/sqlserver_grammar.h"
#include "pf/db/schema/blueprint.h"
#include "pf/db/schema/builder.h"
#include "pf/db/schema/mysql_builder.h"
#include "pf/db/schema/postgres_builder.h"
#include "pf/db/connection.h"
#include "pf/db/grammar.h"
#include "pf/db/seeder.h"
#include "pf/db/define.h"
#include "pf/db/interface.h"
#include "pf/db/factory.h"
#include "pf/db/query.h"

/* engine */
#include "pf/engine/application.h"
#include "pf/engine/kernel.h"

/* file */
#include "pf/file/api.h"
#include "pf/file/ini.h"
#include "pf/file/tab.h"
#include "pf/file/library.h"

/* net */
#include "pf/net/connection/basic.h"
#include "pf/net/connection/pool.h"
#include "pf/net/connection/manager/interface.h"
#include "pf/net/connection/manager/connector.h"
#include "pf/net/connection/manager/epoll.h"
#include "pf/net/connection/manager/iocp.h"
#include "pf/net/connection/manager/basic.h"
#include "pf/net/connection/manager/connector.h"
#include "pf/net/connection/manager/listener.h"
#include "pf/net/connection/manager/listener_factory.h"
#include "pf/net/packet/interface.h"
#include "pf/net/packet/dynamic.h"
#include "pf/net/packet/factory.h"
#include "pf/net/packet/factorymanager.h"
#include "pf/net/protocol/interface.h"
#include "pf/net/protocol/basic.h"
#include "pf/net/socket/api.h"
#include "pf/net/socket/listener.h"
#include "pf/net/socket/basic.h"
#include "pf/net/stream/basic.h"
#include "pf/net/stream/input.h"
#include "pf/net/stream/output.h"
#include "pf/net/stream/compressor.h"
#include "pf/net/stream/encryptor.h"

/* script */
#include "pf/script/interface.h"
#include "pf/script/factory.h"

/* sys */
#include "pf/sys/memory/static_allocator.h"
#include "pf/sys/memory/dynamic_allocator.h"
#include "pf/sys/memory/share.h"
#include "pf/sys/memory/sharemap.h"
#include "pf/sys/assert.h"
#include "pf/sys/process.h"
#include "pf/sys/thread.h"
#include "pf/sys/util.h"

/* util */
#include "pf/util/compressor/assistant.h"
#include "pf/util/compressor/minimanager.h"
#include "pf/util/bitflag.h"
#include "pf/util/random.h"

#endif //PF_H_
