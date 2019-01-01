-- The script packet id must begin 20001.
return {

  test = {id = 0x4e21, proto = "test.pb_test"},

  c2w_login = {id = 0x4e30, proto = "login.c2w_login"}, -- 客户端请求登录网关
  c2w_enter = {id = 0x4e31, proto = "login.c2w_enter"}, -- 客户端请求进入游戏
  w2l_enter = {id = 0x4e32, proto = "login.w2l_enter"}, -- 网关请求进入游戏服
  l2w_enter = {id = 0x4e33, proto = "login.l2w_enter"}, -- 逻辑服返回的进入结果
  l2c_enter = {id = 0x4e34, proto = "login.l2c_enter"}, -- 逻辑服返回客户端的进入游戏信息
  c2l_test = {id = 0x4e35, proto = "login.c2l_test"},
  c2l_test1 = {id = 0x4e36, proto = "login.c2l_test1"},
  l2c_test2 = {id = 0x4e37, proto = "login.l2c_test2"},

}
