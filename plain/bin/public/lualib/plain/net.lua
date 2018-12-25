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
-- @param mixed original The packet from.
function plain_nethandler(npacket, original)
  local id = net.read_id(npacket)
  if not id then return end
  if pb_packethandlers[id] then
    local proto = pb_define[id]
    if not proto then
      error("plain_nethandler can't find protoc config from: "..id)
      return
    end
    local bytes = net.read_string(npacket)
    local data = assert(pb.decode(proto, bytes)) 
    pb_packethandlers[id](data, original)
  else
    local func = packethandlers[id]
    if func then func(npacket, original) end
  end
end

-- Register the net handler.
-- @param number id The packet id.
-- @param function func [(npacket, original)]
function net.reg_handler(id, func)
  packethandlers[id] = func
end

-- Register the net protobuf handler.
-- @param number id The packet id.
-- @param function func [(data, original)]
function net.reg_pbhandler(id, func)
  pb_packethandlers[id] = func
end

-- Send a protobuf packet.
-- @param string name The connection name.
-- @param table oper The operator info {id = packet id, proto = protobuf name}
-- @param table data The protobuf data.
function net.pb_send(name, oper, data)
  local id = oper.id
  local proto = oper.proto
  local bytes = assert(pb.encode(proto, data))
  local npacket = net.alloc(id)
  if not npacket then
    return
  end
  net.write_string(npacket, bytes)
  net.send(name, npacket)
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
