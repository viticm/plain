--[[
 - PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 - $Id login.lua
 - @link https://github.com/viticm/plain for the canonical source repository
 - @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 - @license
 - @user viticm<viticm.ti@gmail.com>
 - @date 2018/12/27 19:00
 - @uses The login module.
--]]

local op = require "pb_define"
local dumptable = require("dumptable")

local userlist = userlist or {}

-- The routing reached.
net.reg_routing("logic1", function(aim_name, service) 
  print("routing reached", aim_name, service)
  local info = userlist[aim_name]
  if info then
    info.routing = {aim_name = aim_name, service = service}
    if 1 == info.login then
      local msg = {rolelist = {"aaa", "bbb", "ccc"}}
      print(net.pb_send("connector", "logic1", op.l2c_enter, msg, nil, info.routing))
    elseif 2 == info.login then -- Relogin.

    end
  end
end)

-- Enter.
net.reg_pbhandler(op.w2l_enter, function(data, conn, original) 
  print(dumptable(data))
  local username = data.username
  local connid = data.connid
  local servicename = data.servicename
  math.randomseed(os.time())
  local result = 0 -- math.random(0, 1)
  local rmsg = {result = result, connid = connid, servicename = servicename}
  net.pb_send("connector", conn, op.l2w_enter, rmsg)
  if 0 == result then
    userlist[username] = {login = 1} -- First login.
    net.routing_request("connector", conn, nil, servicename, username, connid)
  end
end)

net.reg_pbhandler(op.c2l_test, function(data, conn, original) 
  print(dumptable(data))
  print("info: ", conn, original)
end)

net.reg_pbhandler(op.c2l_test1, function(data, conn, original) 
  print(dumptable(data))
  print("info1: ", conn, original)
  local info = original and userlist[original]
  if info and info.routing then
    local msg = {a = 2333, b = 3.1415926, c = "l2c_test1"}
    print(net.pb_send("connector", "logic1", op.l2c_test2, msg, nil, info.routing))
  end
end)
