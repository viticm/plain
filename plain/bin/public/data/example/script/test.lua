function mytest()
  log.fast("mytest function is done: %d", os.time())
end

mytest = {}
mytest.mytest = function()
  log.fast_debug("test function...");
end

g_testtimes_max = 1
g_testtimes = g_testtimes or 0
function testall()
  if g_testtimes_max <= g_testtimes then return end
  print("testall >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
  dcache_tests()
  g_testtimes = g_testtimes + 1
end
