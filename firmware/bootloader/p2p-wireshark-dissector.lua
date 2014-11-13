-- Peer-to-peer protocol dissector for Wireshark.
--
-- This file was taken from the uracoli project, 802.15.4 sniffer.
--
-- To install, just put this file into your "Personal Plugins" Folder
-- (see Help->About->Folders inside wireshark).
--
-- This file is licensed under the terms of the Modified BSD license:
--
--  Redistribution and use in source and binary forms, with or without
--  modification, are permitted provided that the following conditions
--  are met:
--
--  * Redistributions of source code must retain the above copyright
--    notice, this list of conditions and the following disclaimer.
--  * Redistributions in binary form must reproduce the above copyright
--    notice, this list of conditions and the following disclaimer in the
--    documentation and/or other materials provided with the distribution.
--  * Neither the name of the authors nor the names of its contributors
--    may be used to endorse or promote products derived from this software
--    without specific prior written permission.
--
--  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
--  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
--  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
--  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
--  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
--  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
--  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
--  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
--  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
--  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
--  POSSIBILITY OF SUCH DAMAGE.
-- do
    --
    -- uracoli P2P dissector for Wireshark
    --

    local p2p = Proto ("p2p", "uracoli peer to peer protocoll")

    local cmd_table = {
        [0x01] = "P2P_PING_REQ",
        [0x02] = "P2P_PING_CNF",
        [0x03] = "P2P_JUMP_BOOTL",
        [0x20] = "P2P_WIBO_DATA",
        [0x21] = "P2P_WIBO_FINISH",
        [0x22] = "P2P_WIBO_RESET",
        [0x23] = "P2P_WIBO_EXIT",
        [0x24] = "P2P_WIBO_TARGET",
        [0x25] = "P2P_WIBO_DEAF",
        [0x26] = "P2P_WIBO_ADDR",
        [0x30] = "P2P_XMPL_LED",
        [0x40] = "P2P_WUART_DATA",
    }

    local targmem_table = {
        [string.byte("F")] = "F: Flash",
        [string.byte("E")] = "E: EEPROM",
        [string.byte("X")] = "X: Dry run",
    }

    p2p.fields.f_cmd = ProtoField.uint8("p2p.cmd", "cmd", nil, cmd_table)
    p2p.fields.f_rawdata = ProtoField.bytes("p2p.rawdata", "rawdata", base.HEX)

    -- PING_CNF
    p2p.fields.f_status = ProtoField.uint8("p2p.status","status")
    p2p.fields.f_errno = ProtoField.uint8("p2p.errno", "errno")
    p2p.fields.f_version = ProtoField.uint8("p2p.version", "version")
    p2p.fields.f_crc = ProtoField.uint16("p2p.crc", "crc", base.HEX)
    p2p.fields.f_appname = ProtoField.string("p2p.appname", "appname")
    p2p.fields.f_boardname = ProtoField.string("p2p.boardname", "boardname")
    -- P2P_WIBO_DATA
    p2p.fields.f_dsize = ProtoField.uint8("p2p.dsize", "dsize")
    p2p.fields.f_data = ProtoField.bytes("p2p.data", "data")
    -- P2P_WIBO_TARGET
    p2p.fields.f_targmem = ProtoField.uint8("p2p.targmem", "targmem", nil, targmem_table)
    -- P2P_WIBO_ADDR
    p2p.fields.f_addr = ProtoField.uint32("p2p.addr", "addr")
    -- P2P_XMPL_LED
    p2p.fields.f_led = ProtoField.uint8("p2p.led", "led")
    p2p.fields.f_state = ProtoField.uint8("p2p.state", "state")
    -- P2P_WUART_DATA
    p2p.fields.f_mode = ProtoField.uint8("p2p.mode", "mode")
    p2p.fields.f_sdata = ProtoField.string("p2p.sdata", "sdata")

    p2p.experts.too_short = ProtoExpert.new("short", "Packet too short", expert.group.MALFORMED, expert.severity.ERROR)
    p2p.experts.unknown_cmd = ProtoExpert.new("unknown_cmd", "Unknown command", expert.group.MALFORMED, expert.severity.ERROR)

    -- Init function, called before any packet is dissected
    function p2p.init()
        -- print("p2p.init")
    end

    function dissect_ping_cnf (buffer, pinfo, tree)
        local st = tree --tree:add(p2p, buffer())
        st:add(p2p.fields.f_status, buffer(0, 1))
        st:add(p2p.fields.f_errno, buffer(1, 1))
        st:add(p2p.fields.f_version, buffer(2, 1))
        st:add_le(p2p.fields.f_crc, buffer(3, 2))
        st:add(p2p.fields.f_appname, buffer(5, 16))
        st:add(p2p.fields.f_boardname, buffer(21, 16))
    end

    function dissect_wibo_data(buffer, pinfo, tree)
        tree:add_le(p2p.fields.f_dsize, buffer(0, 1))
        tree:add(p2p.fields.f_data, buffer(1, buffer(0, 1):uint()))
    end

    function dissect_wibo_target(buffer, pinfo, tree)
        tree:add(p2p.fields.f_targmem, buffer(1, 1))
    end

    function dissect_wibo_addr(buffer, pinfo, tree)
        tree:add(p2p.fields.f_addr, buffer(0, 4))
    end

    function dissect_xmpl_led(buffer, pinfo, subtree)
        tree:add(p2p.fields.f_led, buffer(0, 1))
        tree:add(p2p.fields.f_state, buffer(1, 1))
    end

    function dissect_wuart_data(buffer, pinfo, tree)
        tree:add(p2p.fields.f_mode, buffer(0, 1))
        tree:add(p2p.fields.f_sdata, buffer(1))
    end

    -- The main dissector function
    function real_dissector (buffer, pinfo, subtree)
        local cmd = buffer(0,1)
        subtree:add(p2p.fields.f_cmd, cmd)

        cmdname = cmd_table[cmd:uint()]
        if (not cmdname) then
            subtree:add_tvb_expert_info(p2p.experts.unknown_cmd, cmd)
            return false
        end

        -- Skip 1 byte of p2p command
        subbuf = buffer(1):tvb()

        if (subbuf:len() > 0) then
            subtree:add(p2p.fields.f_rawdata, subbuf())
        end

        -- decode frame internals
        if (cmdname == "P2P_PING_CNF") then
            dissect_ping_cnf(subbuf, pinfo, subtree)
        elseif (cmdname == "P2P_WIBO_DATA") then
            dissect_wibo_data(subbuf, pinfo, subtree)
        elseif (cmdname == "P2P_WIBO_TARGET") then
            dissect_wibo_target(subbuf, pinfo, subtree)
        elseif (cmdname == "P2P_WIBO_ADDR") then
            dissect_wibo_addr(subbuf, pinfo, subtree)
        elseif (cmdname == "P2P_XMPL_LED") then
            dissect_xmpl_led(subbuf, pinfo, subtree)
        elseif (cmdname == "P2P_WUART_DATA") then
            dissect_wuart_data(subbuf, pinfo, subtree)
        end

        -- Only set the protocol name last, so we only set it for
        -- packets that look valid.
        pinfo.cols.protocol = "P2P"

        return true
    end

    -- The main dissector function
    function p2p.dissector (buffer, pinfo, tree)
        local subtree = tree:add(p2p, "P2P Protocol Data")

        ok, result = pcall(real_dissector, buffer, pinfo, subtree)
        if not ok and result:match("Range is out of bounds$") then
            subtree:add_proto_expert_info(p2p.experts.too_short)
            return false
        elseif not ok then
            error(result)
        else
            return result
        end
    end

    -- Register as a heuristic dissector, that gets called for all wpan
    -- packets. The labmda is a workaround, see
    -- https://bugs.wireshark.org/bugzilla/show_bug.cgi?id=10695
    p2p:register_heuristic("wpan", function(...) p2p.dissector(...) end)

    -- Register as a normal dissector. We have to specify a panid here,
    -- so we just pass -1 as we don't really care (we cannot leave it
    -- out, see https://bugs.wireshark.org/bugzilla/show_bug.cgi?id=10696).
    table = DissectorTable.get("wpan.panid")
    table:add(-1, p2p)
-- end
