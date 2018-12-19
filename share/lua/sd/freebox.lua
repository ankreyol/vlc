--[[
 $Id$

 Copyright © 2010 VideoLAN and AUTHORS

 Authors: Fabio Ritrovato <sephiroth87 at videolan dot org>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
--]]

function descriptor()
    return { title="Freebox TV" }
end

function main()
    -- fetch the playlist
    fd, msg = vlc.stream( "http://mafreebox.freebox.fr/freeboxtv/playlist.m3u" )
    if not fd then
        vlc.msg.warn(msg)
        return nil
    end
    local items =  fd:readdir()
    local duration, artist, name, arturl
    local options={"deinterlace=1"}
    for _, item in ipairs(items) do
        vlc.sd.add_item({ path = item:uri(),
                          duration = item:duration(),
                          title = item:name(),
                          uiddata = item:uri(),
                          meta = {["Listing Type"]="tv"},
                          options = options })
    end
end
