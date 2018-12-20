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
package.cpath = BASEPATH .."/public/luaclib/?.so" -- Framework build with cxx.
if g_debug then
  package.path = "./?.lua;" .. ROOTPATH .. "/?.lua;".. BASEPATH .."/public/lualib/?.lua"
else
  package.path = "./?.lc;" .. ROOTPATH .. "?.lc;".. BASEPATH .."/public/lualib/?.lc"
end

require("preload")
