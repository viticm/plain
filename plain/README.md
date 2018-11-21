1. app used gateway and logicserver and dbserver.

2. gateway（网关）的作用主要是账号验证和一些无效连接拦截
   logicserver 作为逻辑处理器，负责游戏世界的所有处理
   dbserver 作为数据库查询接入处理的服务器（暂放到逻辑服务器中处理）

   在logicserver中使用redis为玩家数据作为缓存和共享的方式，提供单个模块的缓存接口
   需要指定某个关键字作为更新（通用模块使用玩家ID），现在的缓存暂放在脚本层处理

   网络，统一使用protobuf的协议
   考虑动态加载协议进行注册的方式（包括C++层） -- 暂定
