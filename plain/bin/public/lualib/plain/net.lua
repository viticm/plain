--[[
 - PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 - $Id net.lua
 - @link https://github.com/viticm/plain for the canonical source repository
 - @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 - @license
 - @user viticm<viticm.ti@gmail.com>
 - @date 2018/12/22 14:44
 - @uses The plainframework net module.
--]]

-- The lost event functions.
local lostfuncs = lostfuncs or {}

-- The net lost global function.
-- @param string name The lost connection name.
-- @param number connid The lost connection id.
function plain_net_lost(name, connid)
  local func = lostfuncs[name]
  if func then func(name) end
end

-- Register the net lost callback.
-- @param string name The lost connection name.
-- @param function func [(name)]
function net.reg_lost(name, func)
  lostfuncs[name] = func
end
