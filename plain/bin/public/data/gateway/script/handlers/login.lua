--[[
 - PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 - $Id login.lua
 - @link https://github.com/viticm/plain for the canonical source repository
 - @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 - @license
 - @user viticm<viticm.ti@gmail.com>
 - @date 2018/12/27 16:07
 - @uses The login module.
--]]

local op = require "pb_define"
local dumptable = require("dumptable")

local loginlist = loginlist or {}

-- User login.
net.reg_pbhandler(op.c2w_login, function(data, conn, original)
  print(dumptable(data))
  loginlist[data.username] = 1
end)

-- User enter.
net.reg_pbhandler(op.c2w_enter, function(data, conn, original) 
  print(dumptable(data))
  local username = data.username
  local server = data.server
  local connid = conn
  if not loginlist[username] then
    assert(false, "c2w_enter user not login: "..username)
    return
  end
  if type(connid) ~= "number" then
    assert(false, "c2w_enter connection error")
    return
  end
  local msg = {username = username, connid = connid}
  print("w2l_enter...........")
  print(dumptable(msg))
  local r = net.pb_send("listener", server, op.w2l_enter, msg, "server_service")
  if not r then
    assert(false, "c2w_enter can't reach the server: "..server)
  end
end)

-- Enter result.
net.reg_pbhandler(op.l2w_enter, function(data, conn, original) 
  print(dumptable(data))
  if data.result ~= 0 then
    net.disconnect(data.servicename, data.connid)
  end
end)
