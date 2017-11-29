# This file generate by plain framework admin tool, date: 2017-07-05 23:53:56
SET(LIBRARY_OUTPUT_PATH ./../dll/)

ADD_DEFINITIONS(-DPF_CORE)
ADD_DEFINITIONS(-DPF_OPEN_EPOLL)
SET(USE_32BITS 0)

IF(CMAKE_SYSTEM MATCHES Linux)
  ADD_DEFINITIONS(-D_REENTRANT)
  ADD_DEFINITIONS(-DDONT_TD_VOID)
  ADD_DEFINITIONS(-DLUA_USE_LINUX)
ELSE(CMAKE_SYSTEM MATCHES Linux)
ENDIF(CMAKE_SYSTEM MATCHES Linux)

LINK_DIRECTORIES(
  "./"
  "./../../dependencies/dll/"
)

INCLUDE_DIRECTORIES(
  "../include/"
  "../../dependencies/lua/5.3.4/include/"
  "../../dependencies/iconv/include/"
  "/usr/local/unixODBC/include/"
)


SET (INCLUDE_LIST
  ../include/pf.h
)

SET (INCLUDE_PF_BASIC_LIST
  ../include/pf/basic/base64.h
  ../include/pf/basic/config.h
  ../include/pf/basic/global.h
  ../include/pf/basic/io.tcc
  ../include/pf/basic/logger.h
  ../include/pf/basic/logger.tcc
  ../include/pf/basic/md5.h
  ../include/pf/basic/monitor.h
  ../include/pf/basic/singleton.tcc
  ../include/pf/basic/string.h
  ../include/pf/basic/stringstream.h
  ../include/pf/basic/task_queue.tcc
  ../include/pf/basic/time_manager.h
  ../include/pf/basic/tinytimer.h
  ../include/pf/basic/ts_string.h
  ../include/pf/basic/util.h
)

SET (INCLUDE_PF_BASIC_HASHMAP_LIST
  ../include/pf/basic/hashmap/config.h
  ../include/pf/basic/hashmap/template.h
  ../include/pf/basic/hashmap/template.tcc
)

SET (INCLUDE_PF_BASIC_MACROS_LIST
  ../include/pf/basic/macros/export.h
  ../include/pf/basic/macros/function.h
  ../include/pf/basic/macros/limit.h
  ../include/pf/basic/macros/other.h
  ../include/pf/basic/macros/platform.h
)

SET (INCLUDE_PF_BASIC_TYPE_LIST
  ../include/pf/basic/type/config.h
  ../include/pf/basic/type/variable.h
  ../include/pf/basic/type/variable.tcc
)

SET (INCLUDE_PF_CACHE_LIST
  ../include/pf/cache/config.h
  ../include/pf/cache/db_define.h
  ../include/pf/cache/db_store.h
  ../include/pf/cache/manager.h
  ../include/pf/cache/repository.h
  ../include/pf/cache/storeinterface.h
)

SET (INCLUDE_PF_CACHE_PACKET_LIST
  ../include/pf/cache/packet/config.h
  ../include/pf/cache/packet/db_query.h
  ../include/pf/cache/packet/db_result.h
)

SET (INCLUDE_PF_DB_LIST
  ../include/pf/db/config.h
  ../include/pf/db/define.h
  ../include/pf/db/factory.h
  ../include/pf/db/interface.h
  ../include/pf/db/query.h
)

SET (INCLUDE_PF_ENGINE_LIST
  ../include/pf/engine/application.h
  ../include/pf/engine/config.h
  ../include/pf/engine/kernel.h
  ../include/pf/engine/kernel.tcc
  ../include/pf/engine/thread.h
)

SET (INCLUDE_PF_FILE_LIST
  ../include/pf/file/api.h
  ../include/pf/file/config.h
  ../include/pf/file/ini.h
  ../include/pf/file/library.h
  ../include/pf/file/tab.h
)

SET (INCLUDE_PF_NET_LIST
  ../include/pf/net/config.h
)

SET (INCLUDE_PF_NET_CONNECTION_LIST
  ../include/pf/net/connection/basic.h
  ../include/pf/net/connection/config.h
  ../include/pf/net/connection/pool.h
)

SET (INCLUDE_PF_NET_CONNECTION_MANAGER_LIST
  ../include/pf/net/connection/manager/basic.h
  ../include/pf/net/connection/manager/config.h
  ../include/pf/net/connection/manager/connector.h
  ../include/pf/net/connection/manager/epoll.h
  ../include/pf/net/connection/manager/interface.h
  ../include/pf/net/connection/manager/iocp.h
  ../include/pf/net/connection/manager/listener.h
  ../include/pf/net/connection/manager/select.h
)

SET (INCLUDE_PF_NET_PACKET_LIST
  ../include/pf/net/packet/config.h
  ../include/pf/net/packet/dynamic.h
  ../include/pf/net/packet/factory.h
  ../include/pf/net/packet/factorymanager.h
  ../include/pf/net/packet/interface.h
)

SET (INCLUDE_PF_NET_PROTOCOL_LIST
  ../include/pf/net/protocol/basic.h
  ../include/pf/net/protocol/config.h
  ../include/pf/net/protocol/interface.h
)

SET (INCLUDE_PF_NET_SOCKET_LIST
  ../include/pf/net/socket/api.h
  ../include/pf/net/socket/basic.h
  ../include/pf/net/socket/config.h
  ../include/pf/net/socket/extend.inl
  ../include/pf/net/socket/listener.h
)

SET (INCLUDE_PF_NET_STREAM_LIST
  ../include/pf/net/stream/basic.h
  ../include/pf/net/stream/compressor.h
  ../include/pf/net/stream/config.h
  ../include/pf/net/stream/encryptor.h
  ../include/pf/net/stream/input.h
  ../include/pf/net/stream/output.h
)

SET (INCLUDE_PF_SCRIPT_LIST
  ../include/pf/script/config.h
  ../include/pf/script/factory.h
  ../include/pf/script/interface.h
)

SET (INCLUDE_PF_SYS_LIST
  ../include/pf/sys/assert.h
  ../include/pf/sys/config.h
  ../include/pf/sys/process.h
  ../include/pf/sys/thread.h
  ../include/pf/sys/thread.tcc
  ../include/pf/sys/util.h
)

SET (INCLUDE_PF_SYS_MEMORY_LIST
  ../include/pf/sys/memory/config.h
  ../include/pf/sys/memory/dynamic_allocator.h
  ../include/pf/sys/memory/share.h
  ../include/pf/sys/memory/share.tcc
  ../include/pf/sys/memory/sharemap.h
  ../include/pf/sys/memory/static_allocator.h
)

SET (INCLUDE_PF_UTIL_LIST
  ../include/pf/util/bitflag.h
  ../include/pf/util/config.h
  ../include/pf/util/random.h
)

SET (INCLUDE_PF_UTIL_COMPRESSOR_LIST
  ../include/pf/util/compressor/assistant.h
  ../include/pf/util/compressor/config.h
  ../include/pf/util/compressor/lzoconf.h
  ../include/pf/util/compressor/lzodefs.h
  ../include/pf/util/compressor/mini.h
  ../include/pf/util/compressor/minilzo.h
  ../include/pf/util/compressor/minimanager.h
)

SET (SRC_BASIC_LIST
  ../src/basic/base64.cc
  ../src/basic/global.cc
  ../src/basic/logger.cc
  ../src/basic/monitor.cc
  ../src/basic/string.cc
  ../src/basic/time_manager.cc
  ../src/basic/tinytimer.cc
  ../src/basic/ts_string.cc
  ../src/basic/util.cc
)

SET (SRC_CACHE_LIST
  ../src/cache/db_store.cc
  ../src/cache/manager.cc
  ../src/cache/repository.cc
)

SET (SRC_CACHE_PACKET_LIST
  ../src/cache/packet/db_query.cc
  ../src/cache/packet/db_result.cc
)

SET (SRC_DB_LIST
  ../src/db/define.cc
  ../src/db/factory.cc
  ../src/db/query.cc
)

SET (SRC_ENGINE_LIST
  ../src/engine/application.cc
  ../src/engine/kernel.cc
  ../src/engine/thread.cc
)

SET (SRC_FILE_LIST
  ../src/file/api.cc
  ../src/file/ini.cc
  ../src/file/library.cc
  ../src/file/tab.cc
)

SET (SRC_NET_CONNECTION_LIST
  ../src/net/connection/basic.cc
  ../src/net/connection/pool.cc
)

SET (SRC_NET_CONNECTION_MANAGER_LIST
  ../src/net/connection/manager/basic.cc
  ../src/net/connection/manager/connector.cc
  ../src/net/connection/manager/epoll.cc
  ../src/net/connection/manager/interface.cc
  ../src/net/connection/manager/iocp.cc
  ../src/net/connection/manager/listener.cc
  ../src/net/connection/manager/select.cc
)

SET (SRC_NET_PACKET_LIST
  ../src/net/packet/dynamic.cc
  ../src/net/packet/factorymanager.cc
  ../src/net/packet/interface.cc
)

SET (SRC_NET_PROTOCOL_LIST
  ../src/net/protocol/basic.cc
)

SET (SRC_NET_SOCKET_LIST
  ../src/net/socket/api.cc
  ../src/net/socket/basic.cc
  ../src/net/socket/listener.cc
)

SET (SRC_NET_STREAM_LIST
  ../src/net/stream/basic.cc
  ../src/net/stream/compressor.cc
  ../src/net/stream/encryptor.cc
  ../src/net/stream/input.cc
  ../src/net/stream/output.cc
)

SET (SRC_SCRIPT_LIST
  ../src/script/factory.cc
)

SET (SRC_SYS_LIST
  ../src/sys/assert.cc
  ../src/sys/process.cc
  ../src/sys/util.cc
)

SET (SRC_SYS_MEMORY_LIST
  ../src/sys/memory/dynamic_allocator.cc
  ../src/sys/memory/share.cc
  ../src/sys/memory/sharemap.cc
  ../src/sys/memory/static_allocator.cc
)

SET (SRC_UTIL_LIST
  ../src/util/bitflag.cc
  ../src/util/random.cc
)

SET (SRC_UTIL_COMPRESSOR_LIST
  ../src/util/compressor/assistant.cc
  ../src/util/compressor/mini.cc
  ../src/util/compressor/minilzo.c
  ../src/util/compressor/minimanager.cc
)


ADD_LIBRARY(pf_core SHARED
  ${SRC_BASIC_LIST}
  ${SRC_CACHE_LIST}
  ${SRC_CACHE_PACKET_LIST}
  ${SRC_DB_LIST}
  ${SRC_ENGINE_LIST}
  ${SRC_FILE_LIST}
  ${SRC_NET_CONNECTION_LIST}
  ${SRC_NET_CONNECTION_MANAGER_LIST}
  ${SRC_NET_PACKET_LIST}
  ${SRC_NET_PROTOCOL_LIST}
  ${SRC_NET_SOCKET_LIST}
  ${SRC_NET_STREAM_LIST}
  ${SRC_SCRIPT_LIST}
  ${SRC_SYS_LIST}
  ${SRC_SYS_MEMORY_LIST}
  ${SRC_UTIL_LIST}
  ${SRC_UTIL_COMPRESSOR_LIST}
)

if (WIN32)
TARGET_LINK_LIBRARIES(pf_core ws2_32.lib )
else()
TARGET_LINK_LIBRARIES(pf_core dl pthread)
  if(USE_32BITS)
    SET(CMAKE_C_FLAGS "-Wall -ggdb -pipe -march=i386 -mtune=i686")
    SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall -ggdb -pipe -march=i386 -mtune=i686")
  else()
    SET(CMAKE_C_FLAGS "-Wall -ggdb -pipe -march=x86-64 -mtune=i686")
    SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall -ggdb -pipe -march=x86-64 -mtune=i686")
  endif(USE_32BITS)
endif(WIN32)
