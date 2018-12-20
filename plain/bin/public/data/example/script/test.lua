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

function testpb()
  local pb = require "pb"
  local protoc = require "protoc"

  assert(protoc:load [[
     message Phone {
        optional string name        = 1;
        optional int64  phonenumber = 2;
     }
     message Person {
        optional string name     = 1;
        optional int32  age      = 2;
        optional string address  = 3;
        repeated Phone  contacts = 4;
     } ]])

  local data = {
     name = "ilse",
     age  = 18,
     contacts = {
        { name = "alice", phonenumber = 12312341234 },
        { name = "bob",   phonenumber = 45645674567 }
     }
  }

  local bytes = assert(pb.encode("Person", data))
  print(pb.tohex(bytes))

  local data2 = assert(pb.decode("Person", bytes))
  print(require "serpent".block(data2))

end
testpb()
