require("plain") -- 框架的一些方法重写

-- 开始打印脚本日志，基本环境已经准备完毕
log.fast_debug("begin load script files...")

-- 载入游戏模块
local data = file.opentab("scripttable.txt")
local basemodule = { modulename_ = "__ModuleBase" };
local moduleindex = { __index = basemodule }
local moduleset = {}
for _, line in pairs(data or {}) do
  local modulename = line.TableName
  moduleset[modulename] = 1
  _G[modulename] = setmetatable({ modulename_ = modulename }, moduleindex);
end
Env.moduleset_ = moduleset

function mytest(a, b, c ,d)
  print(a)
  print(b)
  print(c)
  print(d)
end

myfun = {}
myfun.mytest = function(a)
  print(a)
end

--[[
net.globalexcute({"mytest", 1.1444444444444, "abc", 3, 4})
net.globalexcute({"log.fast_error", "this is test function"})
net.globalexcute({"log.fast_debug", "this is test function"})
]]--

-- 载入脚本变量
local scriptvalue_table = file.opentab("scriptvalue/filelist.txt")
for _, line in pairs(scriptvalue_table) do
  local tablename = line.TableName
  local _table = _G[tablename]
  if not _table then
    _table = {}
    _G[tablename] = _table
  end
  local filepath = line.FilePath
  local tablevalue = file.opentab(filepath)
  if not tablevalue then
    log.fast_error("script value file: %s open error", filepath)
    tablevalue = {}
  end
  for _, line in pairs(tablevalue) do
    local name = line.Name
    local value = tonumber(line.Value) or line.Value
    _table[name] = value
  end
end

-- 模块加载
require_ex("config")
require_ex("timer")
require_ex("main")
require_ex("test")
