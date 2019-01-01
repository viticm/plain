--[[
 - PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 - $Id net.lua
 - @link https://github.com/viticm/plain for the canonical source repository
 - @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 - @license
 - @user viticm<viticm.ti@gmail.com>
 - @date 2018/12/24 16:05
 - @uses The plainframework net module.
--]]

local pb = require "pb"
local protoc = require "protoc" 
local lfs = require "lfs"

-- The lost event functions.
local lostfuncs = lostfuncs or {}

-- The net lost global function.
-- @param string name The lost connection name.
-- @param number connid The lost connection id.
function plain_netlost(name, connid)
  local func = lostfuncs[name]
  if func then func(name) end
end

-- The routing reached event functions.
local routingfuncs = routingfuncs or {}

-- The net routing reached global function.
-- @param string name The connection name.
-- @param string aim_name The routing aim name.
function plain_netrouting(name, aim_name, service)
  local func = routingfuncs[name]
  if func then func(aim_name, service) end
end

-- Register the net routing reached callback.
-- @param string name The connection name.
-- @param function func [(aim_name, service)]
function net.reg_routing(name, func)
  routingfuncs[name] = func
end

-- Register the net lost callback.
-- @param string name The lost connection name.
-- @param function func [(name)]
function net.reg_lost(name, func)
  lostfuncs[name] = func
end

-- The net handler functions.
local packethandlers = packethandlers or {}
local pb_packethandlers = pb_packethandlers or {}
local pb_define = pb_define or {}

-- The net packet handle global function.
-- @param number npacket The packet memory address.
-- @param mixed conn The connection id or name.
-- @param mixed original The packet from.
function plain_nethandler(npacket, conn, original)
  --print("plain_nethandler", npacket, original)
  local id = net.read_id(npacket)
  if not id then return end
  if pb_packethandlers[id] then
    local proto = pb_define[id]
    if not proto then
      error("plain_nethandler can't find protobuf config from: "..id)
      return
    end
    local bytes = net.read_bytes(npacket)
    --print("plain_nethandler bytes", pb.tohex(bytes))
    local data = assert(pb.decode(proto, bytes)) 
    pb_packethandlers[id](data, conn, original)
  else
    local func = packethandlers[id]
    if func then func(npacket, conn, original) end
  end
end

-- Register the net handler.
-- @param number id The packet id.
-- @param function func [(data, connection(name or id), original)]
function net.reg_handler(id, func)
  packethandlers[id] = func
end

-- Register the net protobuf handler.
-- @param mixed oper The operator table or packet id.
-- @param function func [(data, connection(name or id), original)]
function net.reg_pbhandler(oper, func)
  local id = oper
  if "table" == type(oper) then
    id = oper.id
  end
  pb_packethandlers[id] = func
end

-- Send a protobuf packet.
-- @param string manager_name The connection manager name.
-- @param string name_or_id The connection name/id.
-- @param mixed oper The operator table or packet id.
-- @param table data The protobuf data.
-- @param mixed listener_name If the manager name is listener this can't nil.
-- @param mixed routing_info It this is table then use routing send.
function net.pb_send(manager_name, name_or_id, oper, data, listener_name, routing_info)
  local id = oper
  local proto = nil
  if "table" == type(oper) then
    id = oper.id
  end
  local proto = pb_define[id]
  if not proto then
    error("plain net.pb_send can't find protobuf config from: "..id)
    return
  end
  local bytes = assert(pb.encode(proto, data))
  local npacket = net.packet_alloc(id)
  if not npacket then
    assert(false, "net.pb_send packet_alloc failed")
    return
  end
  --print("pb_send bytes", pb.tohex(bytes), string.len(bytes))
  net.write_bytes(npacket, bytes)
  if not routing_info then
    return net.send(manager_name, name_or_id, npacket, listener_name)
  else
    local aim_name = routing_info.aim_name
    local service = routing_info.service
    return net.routing(manager_name, name_or_id, listener_name, aim_name, npacket, service)
  end
end

-- Set the protobuf define.
-- @param number id The packet id.
-- @param string proto The proto name.
function net.set_pb_define(id, proto)
  pb_define[id] = proto
end

-- Load the protobuf define from table(the value will be {id = 1, proto = "name"}).
-- @param mixed name The lua name or define table.
function net.pb_define_load(name)
  local tb = name
  if "string" == type(name) then
    tb = require(name)
  end
  if not tb or type(tb) ~= "table" then
    error("plain net pb_define_load error")
    return
  end
  for k, v in pairs(tb) do
    local id, proto = v.id, v.proto
    pb_define[id] = proto
  end
end

-- Load protobuf files from a directory.
-- @param string path The load directory.
-- @param mixed only_pb If just load *.pb files.
function net.pb_load(path, only_pb)
  if not path then return end
  for file in lfs.dir(path) do
    local rfile = file
    -- Check file.
    if only_pb then
      rfile = nil
      if string.len(file) > 3 and string.sub(file, -3, -1) == ".pb" then
        rfile = file
      end
    end
    if "." == rfile or ".." == rfile then
      rfile = nil
    end
    -- Load it.
    local fp = rfile and io.open(path.."/"..rfile, "r")
    if fp then
      if g_debug then
        log.fast_debug("load protobuf file: %s", rfile)
      end
      local buffer = fp:read("*a")
      if buffer then
        protoc:load(buffer)
      end
      fp:close()
    end
  end
end
