--[[
 - PLAIN FRAMEWORK ( https://github.com/viticm/plain )
 - $Id log.lua
 - @link https://github.com/viticm/plain for the canonical source repository
 - @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 - @license
 - @user viticm<viticm.ti@gmail.com>
 - @date 2018/12/22 11:03
 - @uses The plainframework log module.
--]]
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
