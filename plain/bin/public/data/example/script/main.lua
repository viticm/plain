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

-- 入口方法，服务器启动完成时执行一次
function main()
  loadconfig()
  register_global_logic_systems()
  g_system_timer = timer_t.new()
  -- Disable global value for new.
  disable_globalvalue()
  return 2
end

-- 禁止使用新的全局变量
function disable_globalvalue()
  local metatable = {
    __newindex = function(_table, key, value)
      if (key == "it" or key == "him" or key == "me") then
        rawset(_G, key, value);
      else
        error("Attempt create global value :"..tostring(key), 2);
      end
    end
  }
  setmetatable(_G, metatable);
end

-- 脚本心跳
function heartbeat()
  testall()
  if g_logic_system then g_logic_system:heartbeat() end
  return 1
end

-- 脚本消息总入口
function process_message(connectionid, handle, type)
  local data = {} -- 考虑使用无索引的数据，可以节省内存
  data.connectionid = connectionid
  data.handle = handle
  data.type = type
  local systemid = net.pget_systemid(handle)
  local logic_system = g_logic_system:get(systemid)
  if not logic_system then
    log.fast_error("process_message can't find logic system by id: %d",
                   systemid)
    return false
  end
  logic_system:handle_net(data)
  return true
end

-- 一秒定时器
function on_onesecond_timer()
  if kLogicStatusReady == g_logic_system:getstatus() then
    g_system_timer:on_onesecond_timer()
  end
end

-- 一分钟定时器
function on_oneminute_timer()
  if kLogicStatusReady == g_logic_system:getstatus() then
    g_system_timer:on_oneminute_timer()
  end
end

-- 五分钟定时器
function on_fiveminute_timer()
  if kLogicStatusReady == g_logic_system:getstatus() then
    g_system_timer:on_onesecond_timer()
  end
end
