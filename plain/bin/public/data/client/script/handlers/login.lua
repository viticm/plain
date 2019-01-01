--[[
 - PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 - $Id login.lua
 - @link https://github.com/viticm/plain for the canonical source repository
 - @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 - @license
 - @user viticm<viticm.ti@gmail.com>
 - @date 2018/12/27 18:58
 - @uses The login module.
--]]

local op = require "pb_define"
local dumptable = require("dumptable")

-- Enter.
net.reg_pbhandler(op.l2c_enter, function(data, conn, original) 
  print(dumptable(data))
  local rand_func = function()
    math.randomseed(os.time())
    local result = math.random(0, 1)
    if 0 == result then
      local msg = {a = math.random(0, 100000), b = true, c = "teststring"}
      print("c2l_test >>>>>>>>0000000000")
      net.pb_send("connector", conn, op.c2l_test, msg)
    else
      local msg = {a = {1, 3, 5}}
      print("c2l_test1 >>>>>>>>>>>>>0000000000")
      net.pb_send("connector", conn, op.c2l_test1, msg)
    end
  end
  g_system_timer:register_onesecond_timer(rand_func)
end)

net.reg_pbhandler(op.l2c_test2, function(data, conn, original)
  print("l2c_test2---------------------------------")
  print(dumptable(data))
end)
