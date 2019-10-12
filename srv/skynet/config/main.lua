local skynet = require "skynet"

skynet.start(function()
  skynet.newservice("bovosrv")
  skynet.exit()
end)
