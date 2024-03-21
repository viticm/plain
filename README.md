# Plain #

## The plain framework, a framework based on c++ for net applictions. ##

- **Author:** Viticm
- **Website:** [http://www.cnblogs.com/lianyue/](http://www.cnblogs.com/lianyue/)
- **Version:** 2.0.0(cpp23 without modules)

[![Build Status](https://travis-ci.org/viticm/plain.svg)](https://travis-ci.org/viticm/plain)

![img](https://github.com/viticm/plain-simple/blob/master/docs/pf-simple.gif)

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
git clone --recursive https://github.com/viticm/plain && cd plain
cd cmake && cmake ./ && sudo make install
```

On windows(vs 2022+ can build with cmake directly).

Update all submodule.

```shell
git submodule update --remote --recursive
```

## Customize your network. ##


## Simple project. ##

Many developers will confused use this framework, this project will tech you quick start.

The project url:

[https://github.com/viticm/plain-simple](https://github.com/viticm/plain-simple)
