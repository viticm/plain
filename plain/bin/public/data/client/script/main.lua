--[[
 - PAP Engine ( https://github.com/viticm/plainframework1 )
 - $Id main.lua
 - @link https://github.com/viticm/plainframework1 for the canonical source repository
 - @copyright Copyright (c) 2014- viticm( viticm@126.com/viticm.ti@gmail.com )
 - @license
 - @user viticm<viticm@126.com/viticm.ti@gmail.com>
 - @date 2015/04/23 15:14
 - @uses 中心服务器脚本入口，初始化脚本完成后调用此函数，类似C++main
 -       之后不能再创建新的全局变量
--]]

require("handlers/all")
local op = require("pb_define")
local connid

-- 入口方法，服务器启动完成时执行一次
function main()
  print("client main..................")


  loadconfig()

  g_system_timer = timer_t.new()

  -- Disable global value for new.
  disable_globalvalue()

  connid = net.connect("", "127.0.0.1", 2333, true)
  local msg = {
    a = 111,
    b = "test",
    c = {1, 3, 5},
  }
  local r1 = net.pb_send("connector", connid, op.test, msg)
  local r2 = net.pb_send("connector", connid, op.c2w_login, {username = "test"})
  local r3 = net.pb_send("connector", connid, op.c2w_enter, {username = "test", server = "logic1"})
  
  --log.slow("client main ........................ %d| %d.%d.%d", connid, r1 and 1 or 0, r2 and 1 or 0, r3 and 1 or 0)
  return true
end

-- 脚本心跳
function heartbeat()
  g_system_timer:run()
  return true
end
