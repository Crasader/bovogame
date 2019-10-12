local skynet = require "skynet"
local socket = require "skynet.socket"
local pb = require "libpb"
local xck = require "libxchk"

local mode = ...

--[[
user:
  { name, self-cid, oppo-cid, mark, ready, valid }
--]]

if mode == "agent" then

local front = { name = nil, cid = nil, mark = 0, ready = 0, valid = 0 }
local back = { name = nil, cid = nil, mark = 0, ready = 0, valid = 0 }
local board = xck.init()

local cmd_NONE = 0
local cmd_JOIN = 1
local cmd_STYLE = 2
local cmd_PUT = 3
local cmd_QUIT = 4
local cmd_TOKEN = 5

local status_OKAY = 0
local status_ERROR = 1
local status_WON = 2
local status_LOST = 3

local token_to_play = 0

function cleanup()
  front.valid = 0
  back.valid = 0
end

function alloc_item(nam, cid)
  if front.valid > 0 and back.valid > 0 then
    return nil
  end
  local item = nil
  if front.valid == 0 then
    front.name = nam -- name
    front.cid = cid -- cid
    front.mark = 1 --mark style
    front.ready = 0 -- ready
    front.valid = 1 -- valid
    item = front
    token_play = 1
  elseif back.valid == 0 and nam ~= front.name then
    back.name = nam
    back.cid = cid
    back.mark = 2
    back.ready = 0
    back.valid = 1
    item = back
  end
  if front.valid > 0 and back.valid > 0 then
    front.ready = 1 -- both ready
    back.ready = 1
  end
  return item
end

function match_is_ready()
  return front.ready > 0 and back.ready > 0
end

function match_quit(cid, status)
  local ud = pb.init()
  if ud then
    pb.set(ud, "cmd", cmd_QUIT)
    pb.set(ud, "status", status)
    local dat, len = pb.serialize(ud)
    if dat and len > 0 then
      socket.write(cid, dat, len)
    end
  end
end

function play(self, oppo, x, y)
  --
  xck.set(board, x, y, self.mark)
  --
  -- notify opponent to update board
  --
  local ud = pb.init()
  if ud then
    pb.set(ud, "cmd", cmd_PUT) -- PUT
    pb.set(ud, "x", y) -- x
    pb.set(ud, "y", x) -- y
    pb.set(ud, "markstyle", self.mark) -- markstyle
    local dat, len = pb.serialize(ud)
    if dat and len > 0 then
      socket.write(oppo.cid, dat, len)
    end
    pb.free(ud)
  end
  --
  -- check if it already won or not
  --
  if xck.check(board, x, y, self.mark) then
    -- already won match
    --
    skynet.error(string.format("%s has won the match", self.name))
    --
    local ud1 = pb.init()
    local ud2 = pb.init()
    if ud1 and ud2 then
      pb.set(ud1, "cmd", cmd_QUIT) -- quit
      pb.set(ud2, "cmd", cmd_QUIT)
      pb.set(ud1, "status", status_WON) -- WON
      pb.set(ud2, "status", status_LOST) -- LOST for opponent
      pb.set(ud1, "markstyle", self.mark)
      pb.set(ud2, "markstyle", oppo.mark)
      pb.set(ud1, "x", y)
      pb.set(ud1, "y", x)
      pb.set(ud2, "x", y)
      pb.set(ud2, "y", x)
      local dat = nil
      local len = 0
      dat, len = pb.serialize(ud1)
      if dat and len > 0 then
        socket.write(self.cid, dat, len)
      end
      dat, len = pb.serialize(ud2)
      if dat and len > 0 then
        socket.write(oppo.cid, dat, len)
      end
    end
    if ud1 then
      pb.free(ud1)
    end
    if ud2 then
      pb.free(ud2)
    end

    -- cleanup
    cleanup()
    return true

  else
    local ud = pb.init()
    if ud then
      pb.set(ud, "cmd", cmd_NONE) -- none
      pb.set(ud, "status", status_OKAY) -- okay
      pb.set(ud, "x", y)
      pb.set(ud, "y", x)
      local dat, len = pb.serialize(ud)
      if dat and len > 0 then
        socket.write(self.cid, dat, len)
      end
    end
  end

  return true
end

function dispatch_join(cid, ud)
  local nam = pb.get(ud, "name")
  if nam == nil then
    return false
  end
  local item = alloc_item(nam, cid)
  if item == nil then
    return false
  end
  if item == back and match_is_ready() then
    -- both front and back are initialized
    -- notify client to update its info
    local ud1 = pb.init()
    local ud2 = pb.init()
    if ud1 and ud2 then
      pb.set(ud1, "cmd", cmd_STYLE) -- STYLE
      pb.set(ud2, "cmd", cmd_STYLE)
      pb.set(ud1, "status", status_OKAY) -- OKAY
      pb.set(ud2, "status", status_OKAY)
      pb.set(ud1, "markstyle", front.mark) -- mark style
      pb.set(ud2, "markstyle", back.mark)
      pb.set(ud1, "name", back.name) -- opponent's name
      pb.set(ud2, "name", front.name)
      local dat, len
      dat, len = pb.serialize(ud1)
      if dat and len > 0 then
        if socket.write(front.cid, dat, len) then
          skynet.error(front.name .. " joined server")
        end
      end
      dat, len = pb.serialize(ud2)
      if dat and len > 0 then
        if socket.write(back.cid, dat, len) then
          skynet.error(back.name .. " joined server")
        end
      end
    end
    if ud1 then
      pb.free(ud1)
    end
    if ud2 then
      pb.free(ud2)
    end
    local ud3 = pb.init()
    if ud3 then
      token_to_play = 1
      pb.set(ud3, "cmd", cmd_TOKEN) -- TOKEN
      pb.set(ud3, "status", status_OKAY) -- OKAY
      local dat, len = pb.serialize(ud3)
      if dat and len > 0 then
        socket.write(front.cid, dat, len) -- pass token to front
      end
      pb.free(ud3)
    end
  end
  return true
end

function dispatch_put(cid, ud)
  if not match_is_ready() then
    return false
  end
  local nam = pb.get(ud, "name") -- username
  if nam == nil then
    return false
  end
  if ( nam == front.name and token_to_play ~= 1 ) or ( nam == back.name and token_to_play ~= 2 ) then
    local ud = pb.init()
    pb.set(ud, "cmd", cmd_NONE)
    pb.set(ud, "status", status_ERROR)
    local dat, len = pb.serialize(ud)
    if dat and len > 0 then
      socket.write(cid, dat, len)
    end
    return true
  end
  local x = pb.get(ud, "x")
  local y = pb.get(ud, "y")
  local markstyle = 0
  local self = nil
  local oppo = nil
  if nam == front.name then
    markstyle = front.mark
    token_to_play = 2 -- pass token to back
    self = front
    oppo = back
  elseif nam == back.name then
    markstyle = back.mark
    token_to_play = 1 -- pass token to front
    self = back
    oppo = front
  else
    return false
  end
  skynet.error(self.name .. " go (" .. tostring(x) .. "," .. tostring(y) .. ")")
  if play(self, oppo, x, y) then
    return true
  end
  return false
end

skynet.start(function()
  skynet.error("Service started")
  skynet.dispatch("lua", function(_, _, cid)
    socket.start(cid)
    while true do
      local msg = socket.read(cid)
      local quit = false
      local ud = pb.init()
      if msg and ud and pb.parse(ud, msg, string.len(msg)) then
        -- `ud' has message
        local cmd = pb.get(ud, "cmd")
        if cmd == cmd_JOIN then -- *** JOIN ***
          quit = not dispatch_join(cid, ud)
          if quit then
            skynet.error("Disconnected from server after JOIN")
            match_quit(cid, status_ERROR)
          end
        elseif cmd == cmd_PUT then -- *** PUT ***
          quit = not dispatch_put(cid, ud)
          if quit then
            skynet.error("Disconnected from server after PUT")
            match_quit(cid, status_OKAY)
          end
        elseif cmd == cmd_QUIT then -- *** QUIT ***
          quit = true
        else
          skynet.error("Malformed message")
          match_quit(cid, status_ERROR)
          quit = true
        end
      else
        skynet.error("Unknown message " .. tostring(pb.get(ud, "cmd")))
        match_quit(cid, status_ERROR)
        quit = true
      end
      --print(msg)
      --socket.write(cid, "message from server")
      if ud then
        pb.free(ud)
      end
      if quit == true then
        skynet.error("Client submitted quit message")
        front.name = nil
        back.name = nil
        front.valid = 0
        back.valid = 0
        break
      end
    end -- while
    skynet.error("Server close connection")
    socket.close(cid)
  end)
end)

else

skynet.start(function()
  local agent = {}
  for i = 1, 8 do
    agent[i] = skynet.newservice(SERVICE_NAME, "agent")
  end
  local balance = 1
  local inc = 1
  local id = socket.listen("0.0.0.0", 8000)
  skynet.error("Listen game port on 8000")
  socket.start(id, function(id, addr)
    skynet.error(string.format("%s connected, pass it to agent[%d]", addr, balance))
    skynet.send(agent[balance], "lua", id)
    inc = inc + 1
    if inc > 2 then -- every service only contains 2 users
      balance = (balance % (#agent)) + 1
      inc = 1
    end
  end)
end)

end
