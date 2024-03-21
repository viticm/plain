# Plain #

## Plain framework 一个基于C++的网络应用框架 ##

- **作者:** Viticm
- **网站:** [http://www.cnblogs.com/lianyue/](http://www.cnblogs.com/lianyue/)
- **版本:** 2.0.0(C++23 没用使用模块(modules))

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
```

Windows(vs 2022 以上版本可以直接在工程中右键cmake/CMakeList.txt生成).

更新子模块.

```shell
git submodule update --remote --recursive
```


## 定制你自己的网络. ##


## 简单的项目. ##

很多开发者如果初次接触会对框架感到疑惑，因此可以参考该项目快速上手.

项目地址:

[https://github.com/viticm/plain-simple](https://github.com/viticm/plain-simple)
