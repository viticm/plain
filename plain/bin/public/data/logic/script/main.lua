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
local op = require "pb_define"
local serpent = require "serpent"

-- 入口方法，服务器启动完成时执行一次
function main()
  loadconfig()
  g_system_timer = timer_t.new()
  -- Disable global value for new.
  disable_globalvalue()
  return true
end

-- 脚本心跳
function heartbeat()
  g_system_timer:run()
  return true
end

-- The test handler.
net.reg_pbhandler(op.test, function(data, conn, original)
  print("The test handler.....................................")
  print(serpent.block(data))
end)
