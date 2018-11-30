-- 重写日志记录方法
local old_log = {}
log = {}

old_log.slow = logger.slow
function log.slow(...)
  old_log.slow("[script] "..string.format(...))
end

old_log.slow_error = logger.slow_error
function log.slow_error(...)
  old_log.slow_error("[script] "..string.format(...))
end

old_log.slow_warning = logger.slow_warning
function log.slow_warning(...)
  old_log.slow_warning("[script] "..string.format(...))
end

old_log.slow_debug = logger.slow_debug
function log.slow_debug(...)
  old_log.slow_debug("[script] "..string.format(...))
end

old_log.slow_write = logger.slow_write
function log.slow_write(...)
  old_log.slow_write("[script] "..string.format(...))
end

old_log.fast = logger.fast
function log.fast(...)
  old_log.fast("[script] "..string.format(...))
end

old_log.fast_error = logger.fast_error
function log.fast_error(...)
  old_log.fast_error("[script] "..string.format(...))
end

old_log.fast_warning = logger.fast_warning
function log.fast_warning(...)
  old_log.fast_warning("[script] "..string.format(...))
end

old_log.fast_debug = logger.fast_debug
function log.fast_debug(...)
  old_log.fast_debug("[script] "..string.format(...))
end

old_log.fast_write = logger.fast_write
function log.fast_write(...)
  old_log.fast_write("[script] "..string.format(...))
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

-- 数据库缓存操作
-- 以后该部分数据考虑使用json
--[[
local old_cache = {}
old_cache.get = cache.get
function cache.get(tablename, key)
  local key = key or "null"
  return old_cache.get(tablename, key)
end

old_cache.put = cache.put
function cache.put(tablename, value, key, centerid)
  local key = key or "null"
  local value = value or {}
  local valuestr = ""
  local valuecount = table.getsize(value)
  local centerid = centerid or -1
  for i, val in ipairs(value) do
    local concatstr = "\t"
    if i == valuecount then concatstr = "" end
    valuestr = valuestr..val..concatstr
  end
  local result = old_cache.put(tablename, key, valuestr, centerid)
  return result
end

-- 转换成底层存储格式
function cache.value_tostring(value)
  local columnstr = ""
  local valuestr = ""
  local typestr = ""
  if type(value) ~= "table" then 
    return columnstr, typestr, valuestr 
  end
  local valuecount = table.getsize(value)
  local i = 1
  local tokey = true
  for index, val in pairs(value) do
    local concatstr = "\t"
    if i == valuecount then concatstr = "" end
    if type(index) ~= "string" then tokey = false end 
    if tokey then
      columnstr = columnstr..index..concatstr
    end
    if "string" == type(val) then
      typestr = typestr..tostring(kDBColumnTypeString)..concatstr
    else
      typestr = typestr..tostring(kDBColumnTypeNumber)..concatstr
    end
    valuestr = valuestr..tostring(val)..concatstr
    i = i + 1
  end
  if false == tokey then columnstr = "" end
  return columnstr, typestr, valuestr
end

-- 增、删、改、查
function cache.insert(tablename, value, key)
  local key = key or "null"
  local columnstr = ""
  local valuestr = ""
  local typestr = ""
  columnstr, typestr, valuestr = cache.value_tostring(value)
  return cache.query(tablename, key, kQueryInsert, columnstr, typestr, valuestr)
end

function cache.delete(tablename, condition, key)
  local key = key or "null"
  local condition = condition or ""
  return cache.query(tablename, key, kQueryDelete, condition)
end

function cache.update(tablename, value, condition, key)
  local key = key or "null"
  local column = column or {}
  local condition = condition or ""
  local columnstr = ""
  local valuestr = ""
  columnstr, _, valuestr = cache.value_tostring(value)
  local result = 
    cache.query(tablename, key, kQueryUpdate, columnstr, valuestr, condition)
  return result
end

function cache.select(tablename, column, condition, key)
  local key = key or "null"
  local column = column or {"*"}
  local condition = condition or ""
  local columnstr = ""
  if "table" == type(column) then
    local columncount = table.getsize(column)
    for i, key in ipairs(column) do
      local concatstr = "\t"
      if i == columncount then concatstr = "" end
      columnstr = columnstr..key..concatstr
    end
  end
  return cache.query(tablename, key, kQuerySelect, columnstr, condition)
end

-- 保存流程如下
-- 1 找寻当前缓存，如果没有则进行2，否则进行第4步
-- 2 如果新建不成功则返回，否则继续第3步
-- 3 进行查询数据，并继续
-- 4 成功则数据为空则进行插入，否则进行更新
-- 此方法需要定时调用，暂时无法及时保存，如需及时保存，直接调用query
function cache.save(tablename, value, condition, key)
  local condition = condition or ""
  local key = key or "null"
  local result, data = cache.get(tablename, key) -- Step 1.
  if kCacheInvalid == result then
    result = cache.put(tablename, nil, key) -- Step 2.
    if result ~= kCacheWaiting then return result end
    cache.select(tablename, "*", condition, key) -- Step 3.
    result, data = cache.get(tablename, key) -- Get.
  end
  if kCacheSuccess == result then -- Step 4.
    if not data or 0 == table.getsize(data) then
      result = cache.insert(tablename, value, key)
    else
      result = cache.update(tablename, value, condition, key)
    end
  end
  return result
end
]]--

-- 模块加载
function require_ex(module_name)
  local _module_name = get_scriptfile(module_name) -- To the true name.
  if package.loaded[_module_name] then
    log.fast_debug("require_ex module[%s] reload", module_name)
  else
    log.fast("require_ex(%s)", module_name)
  end
  package.loaded[_module_name] = nil
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

dump = require("functions.dumptable")
