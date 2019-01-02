# Plain #

## The plain framework, a framework based on c++ for net applictions. ##

The core is `SFE`, that is the safe and fast and easy.

**What's i can do:**

1. Safe

You can use it for safe soket connection, more safe interface protect your appliction.

2. Fast

The very many useful interfaces let develop start become soon, just use default
environment file run your appliction, also can get the high performance with multithreading.

3. Easy

All interfaces use very easy and coupling light, You can start with a environment
file. If you want some diffrent then can write owner plugins.


----------

## How to start with plain framework? ##

On Linux, use the command build and run, make sure installed g++ and cmake:

```shell
git clone https://github.com/viticm/plain && cd plain
git submodule init && git submodule update
cd cmake && cmake ./ && sudo make install
cd ../plain/bin && ./app --env=config/.env.example
```

On windows, will come soon.


## Customize your application by environment file. ##

The environment example file is: ``plain/bin/config/.env.example``
The env config will set in framework globals, just like: `section.key=value`, you
can get it like: `GLOBALS["app.name"]`

### The sections ###

**app** 

1. `name` - Set the appliction name(*by default `""`*).
2. `pidfile` - Set the pid file name(*by default `""`*).

**log**

1. `active` - If Enable the log module(*by default `1`*).
2. `directory` - The log directory(*by default `Execute file path with "/log".`*).
3. `clear` - If remove all log on start(*by default `0`*).

**default**

1. `engine.frame` - The engine main thread frame(*by default `100`*).
2. `net.open` - The default net module is enable(*by default `0`*).
3. `net.service` - The default net module if is with server service(*by default `0`*).
4. `net.service_ip` - The default service ip(*by default `""`, bind any*).
5. `net.service_port` - The default service port(*by default `0`, rand port*).
6. `net.connmax` - The default net connection max count(*by default `NET_CONNECTION_MAX`*).
7. `net.reconnect_time` - The default net client reconnect second(*by default `3`*).
8. `script.open` - The default script if enable(*by default `0`*).
9. `script.rootpath` - The default script root path(*by default `SCRIPT_ROOT_PATH`*).
10. `script.workpath` - The default script workpath(*by default `SCRIPT_WORK_PATH`*).
11. `script.bootstrap` - The default script bootstrap file(*by default `bootstrap.lua`*).
12. `script.reload` - The default script reload file(*by default `reload.lua`*).
13. `script.type` - The default script type(*by default `-1`*).
14. `script.heartbeat` - The default script heartbeat function(*by default `""`*).
15. `script.enter` - The default script enter function(*by default `"main"`*).
16. `db.open` - The default database if enable(*by default `0`*).
17. `db.type` - The default database type(*by default `-1`*).
18. `db.name` - The default database name(*by default `""`*).
19. `db.user` - The default database user(*by default `""`*).
20. `db.password` - The default database password(*by default `""`*).
21. `db.encrypt` - The default database password if encrypted(*by default `0`*)

**plugins**
