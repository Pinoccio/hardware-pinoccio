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
        [0x30] = "P2P_XMPL_LED",
        [0x40] = "P2P_WUART_DATA",
        [0x50] = "P2P_CHSWEEP_SWEEP_REQ",
        [0x51] = "P2P_CHSWEEP_RESULT_REQ",
        [0x52] = "P2P_CHSWEEP_RESULT_CNF",
        [0x53] = "P2P_CHSWEEP_EXEC_REQ"
    }

    local f_pan = ProtoField.uint16("p2p.pan", "pan", base.HEX)
    local f_to = ProtoField.uint16("p2p.to", "to", base.HEX)
    local f_from = ProtoField.uint16("p2p.from", "from", base.HEX)
    local f_cmd = ProtoField.uint8("p2p.cmd", "cmd", nil, cmd_table)
    local f_rawdata = ProtoField.bytes("p2p.rawdata", "rawdata", base.HEX)

    -- PING_CNF
    local f_status = ProtoField.uint8("p2p.status","status")
    local f_errno = ProtoField.uint8("p2p.errno", "errno")
    local f_version = ProtoField.uint8("p2p.version", "version")
    local f_crc = ProtoField.uint16("p2p.crc", "crc", base.HEX)
    local f_pagesz = ProtoField.uint16("p2p.pgsz", "pgsz", base.HEX)
    local f_appname = ProtoField.string("p2p.appname", "appname")
    local f_boardname = ProtoField.string("p2p.boardname", "boardname")
    -- P2P_WIBO_DATA
    local f_targmem = ProtoField.string("p2p.targmem", "targmem")
    local f_dsize = ProtoField.uint8("p2p.dsize", "dsize")
    local f_data = ProtoField.bytes("p2p.data", "data")
    -- P2P_XMPL_LED
    local f_led = ProtoField.uint8("p2p.led", "led")
    local f_state = ProtoField.uint8("p2p.state", "state")
    -- P2P_WUART_DATA
    local f_mode = ProtoField.uint8("p2p.mode", "mode")
    local f_sdata = ProtoField.string("p2p.sdata", "sdata")
    -- P2P_CHSWEEP_SWEEP_REQ
    local f_rx_addr = ProtoField.uint16("p2p.rxaddr", "rxaddr", base.HEX)
    -- P2P_CHSWEEP_RESULT_REQ / P2P_CHSWEEP_RESULT_CNF
    local f_res_mode = ProtoField.uint8("p2p.res_mode","res_mode")
    -- P2P_CHSWEEP_EXEC_REQ
    local f_clear_stat = ProtoField.uint8("p2p.clear_stat", "clear_stat")
    local f_next_channel = ProtoField.uint8("p2p.next_channel", "next_channel")


    -- Init function, called before any packet is dissected
    function p2p.init()
        -- print("p2p.init")
    end

    function dissect_ping_cnf (offs, buffer, pinfo, tree)
        local st = tree --tree:add(p2p, buffer())
        st:add(f_status, buffer(offs, 1))
        st:add(f_errno, buffer(offs+1, 1))
        st:add(f_version, buffer(offs+2, 1))
        st:add_le(f_crc, buffer(offs+3, 2))
        st:add_le(f_pagesz, buffer(offs+5, 2))
        st:add(f_appname, buffer(offs+7, 8))
        st:add(f_boardname, buffer(offs+15, buffer:len() - offs - 15 -2))
    end

    function dissect_wibo_data(offs, buffer, pinfo, tree)
        tree:add(f_targmem, buffer(offs, 1))
        tree:add_le(f_dsize, buffer(offs+1, 1))
        tree:add(f_data, buffer(offs+2, buffer:len() - offs - 2 - 2))
    end

    function dissect_wuart_data(offs, buffer, pinfo, tree)
        tree:add(f_mode, buffer(offs, 1))
        tree:add(f_sdata, buffer(offs+1, buffer:len() - offs - 1 - 2))
    end

    function dissect_chsweep_result_cnf(offs, buffer, pinfo, subtree)
        tree:add(f_res_mode, buffer(offs, 1))
        tree:add(f_data, buffer(offs+1, buffer:len() - offs - 1 - 2))
        -- once more then one result mode is defined, we will dissect
        -- it in this function.
    end

    -- The main dissector function
    function p2p.dissector (buffer, pinfo, tree)
        local fcf = buffer(0,2):uint()
        if (fcf == 0x6188 or fcf == 0x4188) then
            pinfo.cols.protocol = "P2P"
            local subtree = tree:add(p2p,buffer(),"P2P Protocol Data")
            subtree:add(buffer(0,2),"FCF: " .. buffer(0,2))
            offs = 3
            subtree:add_le(f_pan, buffer(offs, 2))
            subtree:add_le(f_to, buffer(offs+2, 2))
            subtree:add_le(f_from, buffer(offs+4, 2))
            local cmd = buffer(offs+6,1)
            subtree:add(f_cmd, cmd)
            
            local rlen = buffer:len() - offs - 7 - 2
            if (rlen > 0) then
                subtree:add(f_rawdata, buffer(offs+7, buffer:len() - offs - 7 - 2))
            end
            -- decode frame internals
            offs = 10
            cmdname = cmd_table[cmd:uint()]
            if (cmdname == "P2P_PING_CNF") then
                dissect_ping_cnf(offs, buffer, pinfo, subtree)
            elseif (cmdname == "P2P_WIBO_DATA") then
                dissect_wibo_data(offs, buffer, pinfo, subtree)
            elseif (cmdname == "P2P_XMPL_LED") then
                tree:add(f_led, buffer(offs, 1))
                tree:add(f_state, buffer(offs+1, 1))
            elseif (cmdname == "P2P_WUART_DATA") then
                dissect_wuart_data(offs, buffer, pinfo, subtree)
            elseif (cmdname == "P2P_CHSWEEP_SWEEP_REQ") then
                subtree:add_le(f_rx_addr, buffer(offs, 2))
                -- debug code for old p2p.pcap example trace, that had mismatch
                -- between SWEEP_REQ and EXEC_REQ.
                -- subtree:add(f_clear_stat, buffer(offs, 1))
                -- subtree:add(f_next_channel, buffer(offs+1, 1))
            elseif (cmdname == "P2P_CHSWEEP_RESULT_REQ") then
                subtree:add(f_res_mode, buffer(offs, 1))
            elseif (cmdname == "P2P_CHSWEEP_RESULT_CNF") then
                dissect_chsweep_result_cnf(offs, buffer, pinfo, subtree)
            elseif (cmdname == "P2P_CHSWEEP_EXEC_REQ") then
                subtree:add(f_clear_stat, buffer(offs, 1))
                subtree:add(f_next_channel, buffer(offs+1, 1))
            end
        end
    end

    -- Create the protocol fields
    p2p.fields = { -- P2P frame header and payload
                   f_pan, f_to, f_from, f_cmd, f_rawdata,
                   -- P2P_PING_CNF
                   f_status, f_errno, f_version, f_crc,
                   f_pagesz, f_appname, f_boardname,
                   -- P2P_WIBO_DATA
                   f_targmem, f_dsize, f_data,
                   -- P2P_XMPL_LED
                   f_led, f_state,
                   -- P2P_WUART_DATA
                   f_mode, f_sdata,
                   -- P2P_CHSWEEP_SWEEP_REQ
                   f_rx_addr,
                   -- P2P_CHSWEEP_RESULT_REQ/CNF
                   f_res_mode,
                   -- P2P_CHSWEEP_EXEC_REQ
                   f_clear_stat, f_next_channel
                 }

     -- Register dissector
    register_postdissector (p2p)

-- end
