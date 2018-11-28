--[[
 - PAP Engine ( https://github.com/viticm/plainframework1 )
 - $Id bootstrap.lua
 - @link https://github.com/viticm/plainframework1 for the canonical source repository
 - @copyright Copyright (c) 2014- viticm( viticm@126.com/viticm.ti@gmail.com )
 - @license
 - @user viticm<viticm@126.com/viticm.ti@gmail.com>
 - @date 2017/01/17 17:13
 - @uses 全局定义，这里的方法和变量不能被重载，重载的变量和函数禁止放在此处
--]]
g_debug = true

if g_debug then
  package.path = "./?.lua"
else
  package.path = "./?.lc"
end
SETTING_PATH = ROOTPATH.."/../setting"

-- 获得脚本文件
function get_scriptfile(filename)
  return ROOTPATH.."/"..filename
end

-- 获得配置文件
function get_settingfile(filename)
  return SETTING_PATH.."/"..filename
end

-- 系统方法重写
local require_sys = require
function require_s(modulename)
  return require_sys(get_scriptfile(modulename))
end

local loadfile_sys = loadfile
function loadfile_s(filename)
  return loadfile_sys(get_scriptfile(filename))
end

require = require_s
loadfile = loadfile_s

function table.getsize(_table)
  local count = 0
  for _, val in pairs(_table) do
    count = count + 1
  end
  return count
end

require("preload")
