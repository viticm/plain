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

```ini
;The env config will set in framework globals, just like: section.key=value 
;cn: 环境变量的配置会设置到框架的全局变量中，格式为：段名.字段名=值
```
