# gmsv_steamworks

This module implements `steamworks.DownloadUGC` on the server. The implementation roughly mirrors the the [Garry's Mod API](https://wiki.facepunch.com/gmod/steamworks.DownloadUGC) with a few small exceptions, namely the fact that the second `file` argument is not available in the result callback. If the API call fails for any reason, the callback will be executed with a nil value for the path. 

# Example Code

```lua
require('steamworks')

steamworks.DownloadUGC('workshopid', function(path)
  if path then
    game.MountGMA(path)
  end
end)

```

# Builds

Builds are available for both windows and linux. Check the actions tab. 

# Important notice!

To get this to work **on linux**, you need a more recent version of the steam client. You will need to put this file in `$EXECUTABLE_ROOT/bin`, replacing the one that is already there. **If you fail to do this properly, your server will segfault.**
