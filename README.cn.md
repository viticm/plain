# Plain #

## Plain framework 一个基于C++的网络应用框架 ##

- **作者:** Viticm
- **网站:** [http://www.cnblogs.com/lianyue/](http://www.cnblogs.com/lianyue/)
- **版本:** 1.1.0rc

[![Build Status](https://travis-ci.org/viticm/plain.svg)](https://travis-ci.org/viticm/plain)

![img](https://github.com/viticm/plain-simple/blob/master/docs/pf-simple.gif)

设计的核心为 `SFE`，即安全、快速、简单.

**我可以做什么:**

1. 安全

你可以加密你的网络连接，用以保护你的应用.

2. 快速

丰富有用的接口让开发来的很快，只需要配置默认的环境配置就可以开启你的应用，同时可
以体验由多线程带来的高性能。

3. 简单

所有接口具有高耦合性，容易使用。你只需要编写自己所需的插件，即可定制丰富的内容。

----------

## 怎么开始使用 plain framework? ##

在UNIX相关的系统中你需要使用以下命令安装g++和cmake:

```shell
git clone --recursive https://github.com/viticm/plain && cd plain
cd cmake && cmake ./ && sudo make install
cd ../plain/bin && ./app --env=config/.env.example
```

Windows(vs 2019+ 以上版本可以直接在工程中右键cmake/CMakeList.txt生成).

更新子模块.

```shell
git submodule update --remote --recursive
```


## 定制你自己的应用环境配置. ##

环境配置示例文件为: ``plain/bin/config/.env.example``
环境配置文件的值会被设置到框架的全局变量中, 就像: `section.key=value`, 
你可以在代码中这样获得它们的值: `GLOBALS["app.name"]`

### 配置字段 ###

**app（应用）** 

1. `name` - 设置应用名称(*默认 `""`*).
2. `pidfile` - 设置应用的pid文件路径(*默认 `""`*).
3. `console` - 是否开启控制台.(*默认 0*)
4. `console.name` - 控制台名称.(*默认 `"console"`*)
5. `console.ip` - 控制台监听IP.(*默认 `"127.0.0.1"`*)
6. `console.port` - 控制台监听端口.(*默认 -1*)
7. `console.connmax` - 控制台支持的网络最大连接数.(*默认 32*)


**log（日志）**

1. `active` - 是否开启日志模块(*默认 `1`*).
2. `directory` - 日志的目录(*默认 `应用所在目录加上 "/log".`*).
3. `clear` - 是否在启动应用的时候清除所有日志(*默认 `0`*).

**default（默认）**

1. `engine.frame` - 主线程的帧率(*默认 `100`*).
2. `net.open` - 是否开启默认网络(*默认 `0`*).
3. `net.service` - 默认网络管理器是否为服务(*默认 `0`*).
4. `net.ip` - 默认网络服务的IP(*默认 `""`, bind any*).
5. `net.port` - 默认网络服务的端口(*默认 `0`, rand port*).
6. `net.connmax` - 默认网络服务器连接最大数(*默认 `NET_CONNECTION_MAX`*).
7. `net.reconnect_time` - 默认网络自动重连的时间（秒）(*默认 `3`*).
8. `script.open` - 默认脚本是否开启(*默认 `0`*).
9. `script.rootpath` - 默认脚本文件的根目录(*默认 `SCRIPT_ROOT_PATH`*).
10. `script.workpath` - 默认脚本文件的工作目录(*默认 `SCRIPT_WORK_PATH`*).
11. `script.bootstrap` - 默认脚本文件的启动文件(*默认 `bootstrap.lua`*).
12. `script.reload` - 默认脚本文件重载的文件(*默认 `reload.lua`*).
13. `script.type` - 默认脚本文件的类型（和插件注册类型对应）(*默认 `-1`*).
14. `script.heartbeat` - 默认脚本文件心跳函数(*默认 `""`*).
15. `script.enter` - 默认脚本文件的启动函数(*默认 `"main"`*).
16. `db.open` - 默认数据库是否开启(*默认 `0`*).
17. `db.type` - 默认数据库类型（和插件类型对应）(*默认 `-1`*).
18. `db.name` - 默认数据库名称(*默认 `""`*).
19. `db.user` - 默认数据库用户(*默认 `""`*).
20. `db.password` - 默认数据库密码(*默认 `""`*).
21. `db.encrypt` - 默认数据库的密码是否加密(*默认 `0`*)

**plugins（插件）**

1. `count` - 需要载入的插件数量.
2. `(0-n)` - 按照序号填写插件的载入配置(*列如 `pf_plugin_odbc:local:...`*).

**database（数据库）**

1. `count` - 数量.
2. `type(0-n)` - 类型（0-n）(*dbenv_t*)
3. `name(0-n)` - 服务名称(0-n).
4. `dbname(0-n)` - 数据库名称(0-n).
5. `dbuser(0-n)` - 数据库用户名(0-n).
6. `dbpassword(0-n)` - 数据库密码(0-n).
7. `encrypt(0-n)` - 数据库密码是否加密(0-n).

**server（服务器）**

1. `count` - 扩展的服务数量.
2. `name(0-n)` - 服务名称(0-n).
3. `ip(0-n)` - 服务的IP(0-n).
4. `port(0-n)` - 服务的监听端口(0-n).
5. `connmax(0-n)` - 服务的最大连接数(0-n).
6. `encrypt(0-n)`- 服务是否加密(0-n) 加密则需要连接使用加密握手包.
7. `scriptfunc(0-n)` - 服务的脚本查询函数名(0-n).

**client（客户端）**

1. `count` - 扩展的客户端连接数量.
2. `usercount` - 用户自定义的连接数量.
3. `name(0-n)` - 连接名(0-n).
4. `ip(0-n)` - 连接IP(0-n).
5. `port(0-n)` - 连接端口(0-n).
6. `encrypt(0-n)` - 连接加密串，如果该值不为空则连接时自动进行握手验证.
7. `startup(0-n)` - 是否在应用启动时就开始连接.
8. `scriptfunc(0-n)` - 连接脚本查询方法名(0-n).

## 更多. ##

### 扩展框架 ###

你可以扩展或者重写这些模块:

- engine（引擎）: 扩展pf_engine::Kernel类.
- net（网络）: 重写或扩展网络连接、协议、数据流.
- ...

| 模块                    | 描述                                          |
| ----------------------- | -----------------------------------           |
| `connection`            | 定义你自己的连接                              |
| `packet`                | 定义你自己的网络包                            |
| `protocol`              | 定义你自己的网络协议                          |
| `stream`                | 定义你自己的网络数据流                        |

### 编写插件 ###

Plain framework插件使得开发更加灵活，你可以不用关心它们如何实现而直接使用，当然
你可以自己编写属于自己心仪的插件.

你可以这样设置你的插件参数 `0=pf_plugin_lua:global:0`.

**关于`pf_plugin_lua:global:0` 描述:**

- `pf_plugin_lua`: 插件名称.
- `global`: 是否载入为全局符号，另一个参数为`local`（本地符号）.
- `0`: 插件的参数，lua插件这个参数为脚本的类型（当然后面你可以扩展你的更多参数）.

你可以参考下面的项目来进行编写自己的插件:

[https://github.com/viticm/plain-plugins](https://github.com/viticm/plain-plugins)


## 简单的项目. ##

很多开发者如果初次接触会对框架感到疑惑，因此可以参考该项目快速上手.

项目地址:

[https://github.com/viticm/plain-simple](https://github.com/viticm/plain-simple)
