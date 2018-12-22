--[[
 - PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 - $Id sys.lua
 - @link https://github.com/viticm/plain for the canonical source repository
 - @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 - @license
 - @user viticm<viticm.ti@gmail.com>
 - @date 2018/12/22 11:09
 - @uses your description
--]]
-- 模块加载
function require_ex(module_name)
  if package.loaded[module_name] then
    log.fast_debug("require_ex module[%s] reload", module_name)
  else
    log.fast("require_ex(%s)", module_name)
  end
  package.loaded[module_name] = nil
  require(module_name)
end

-- 文件加载
function dofile_ex(filename)
  local func = loadfile(filename..".lua")
  if nil == func then
    func = loadfile(filename..".lc")
    if nil == func then
      log.fast_error("dofile_ex(%s) failed", filename)
      os.exit()
      return
    end
  end
  setfenv(func, getfenv(2))
  func()
end
