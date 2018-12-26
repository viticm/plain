--[[
 - PAP Engine ( https://github.com/viticm/plainframework1 )
 - $Id config.lua
 - @link https://github.com/viticm/plainframework1 for the canonical source repository
 - @copyright Copyright (c) 2014- viticm( viticm@126.com/viticm.ti@gmail.com )
 - @license
 - @user viticm<viticm@126.com/viticm.ti@gmail.com>
 - @date 2015/04/23 15:10
 - @uses 配置文件加载
--]]

-- 配置文件加载方法
function loadconfig()
  
  -- Load protobuf files.
  net.pb_define_load("pb_define")
  net.pb_load(PUBLIC_PATH.."/pbfiles")
  
  kConfig = {}

end
