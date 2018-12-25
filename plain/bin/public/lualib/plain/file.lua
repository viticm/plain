--[[
 - PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 - $Id file.lua
 - @link https://github.com/viticm/plain for the canonical source repository
 - @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 - @license
 - @user viticm<viticm.ti@gmail.com>
 - @date 2018/12/22 11:02
 - @uses The plainframework file module.
--]]
SETTING_PATH = ROOTPATH.."/../setting"
PUBLIC_PATH = BASEPATH.."/public"

-- 获得配置文件
function get_settingfile(filename)
  return SETTING_PATH.."/"..filename
end

-- 重写文件操作
local old_file = {}

old_file.opentab = file.opentab
function file.opentab(filename)
  local result = old_file.opentab(get_settingfile(filename))
  return result
end

old_file.openini = file.openini
function file.openini(filename)
  local result = old_file.openini(get_settingfile(filename))
  return result
end
