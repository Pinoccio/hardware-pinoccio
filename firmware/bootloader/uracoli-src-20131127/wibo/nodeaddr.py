# $Id$
# This file is generated automatically from wibo_gen_py.tpl
#
"""
Dumps a intel hex file for a node config structure.

    python nodeaddr.py [OPTIONS]

    Options:
     -a SHORTADDR
        Fixed node short source address (16 bit value).
     -A LONGADDR
        Fixed node short source address (64 bit value).
     -p PANID
        Default pan id (16 bit value).
     -c CHANNEL
        A Channel hint for applications.

     -C CFGFILE
        The configuration file that describes the network settings.
        Using such a config file reduces the average command line length by
        several meters :-) Use -G option to generate a commented template file.
        Note that command line parameters overwrite the settings in the config
        file (default: nodeaddr.cfg).
     -G
        Generate a initial config file with comments (see option -C).

     -B BOARDNAME
        create the record at flash-end for the MMCU
        of the given BOARDNAME (see also -O option)
     -M MMCU
        give the name of the MMCU (instead of -B)
     -O OFFSET
        per default the address of the config record is flashend.
        With -O an explicit offset can be specified.

     -f HEXFILE
        Name of IHEX file to be used as firmware, e.g. wuart_<board>.hex
        where <board> is replaced by the current value.
     -b BOOTLOADER
        Name of IHEX file to be used as bootloader, e.g. wibo_<board>.hex
        where <board> is replaced by the current value.
     -o NEWHEXFILE
        Name of the outputfile, if '-' stdout is used.
        e.g. "foo_<board>_<saddr>.hex", where <board> and <saddr> are replaced
        by the current values.

     -h
        Display help and exit.
     -v
        Show version and exit.
     -l
        List board names and exit
     -L
        List MMCU names and exit

Example:
  - Create the config file nodeaddr.cfg:
     python nodeaddr.py -G

  - Create the config file my_nodeaddr.cfg:
     python nodeaddr.py -C my_nodeaddr.cfg -G

  - After editing the file nodeaddr.cfg use:
     python nodeaddr.py -a1
    in order to create a file for node 1.

  - Add an address record to a hexfile and flash it to the target.
     python nodeaddr.py -b rdk230 -f wibo_rdk230.hex -a 42 -p 1 > a42.hex
     avrdude -P usb -p m1281 -c jtag2 -U a42.hex

  - Pipe the generated hexfile directly into avrdude.
     python nodeaddr.py -b rdk230 -f xmpl_leds_rdk230.hex -a 42 -p 1 |\\
            avrdude -P usb -p m1281 -c jtag2 -U fl:w:-:i
  - Flash the record w/o erasing
    python nodeaddr.py -a 1 | avrdude -P usb -p m1281 -c jtag2 -V -D -U fl:w:-:i


   Writes source address 1 into the device via a pipe to avrdude.

"""

# === import ==================================================================
import struct, getopt, sys, ConfigParser, os
try:
    import Tkinter
except:
    pass

# === globals =================================================================
VERSION = "20131127"


# I/O file parameters
INFILE = None
BOOTLOADER = None
OUTFILE = None

# contents of the config records
PANID = None
SADDR = None
LADDR = None
CHANNEL  = None
CFGFILE = "nodeaddr.cfg"

# memory location of config record
BOARD = None
OFFSET = None

# Default parameters
DEFAULTS = {
    "short_addr": 0xffff,
    "pan_id":     0xffff,
    "ieee_addr":  0xffffffffffffffffL,
    "channel":    0xff,
    "board": None,
    "offset": None
}

# Tables
MMCU_TABLE = {'any2400': 'atmega1281',
 'any2400st': 'atmega1281',
 'any900': 'atmega1281',
 'any900st': 'atmega1281',
 'bat': 'atmega128rfa1',
 'bitbean': 'atmega1281',
 'cbb212': 'atxmega256a3',
 'cbb230': 'atxmega256a3',
 'cbb230b': 'atxmega256a3',
 'cbb231': 'atxmega256a3',
 'cbb232': 'atxmega256a3',
 'cbb233': 'atxmega256a3',
 'derfa1': 'atmega128rfa1',
 'derfn128': 'atmega128rfa1',
 'derfn128u0': 'atmega128rfa1',
 'derfn256u0': 'atmega256rfr2',
 'derfn256u0pa': 'atmega256rfr2',
 'derftorcbrfa1': 'atmega128rfa1',
 'dracula': 'atmega128rfa1',
 'ibdt212': 'atmega644',
 'ibdt231': 'atmega644',
 'ibdt232': 'atmega644',
 'icm230_11': 'atmega1281',
 'icm230_12a': 'atmega1281',
 'icm230_12b': 'atmega1281',
 'icm230_12c': 'atmega128',
 'ics230_11': 'atmega1281',
 'ics230_12': 'atmega128',
 'ict230': 'atmega1281',
 'im240a': 'atmega328',
 'im240a_eval': 'atmega328',
 'lgee231': 'atmega88',
 'lgee231_v2': 'atmega88',
 'midgee': 'atmega88p',
 'mnb900': 'atmega1281',
 'muse231': 'atmega88pa',
 'museII232': 'atmega328p',
 'museIIrfa': 'atmega128rfa1',
 'pinoccio': 'atmega256rfr2',
 'psk212': 'atmega1281',
 'psk230': 'atmega1281',
 'psk230b': 'atmega1281',
 'psk231': 'atmega1281',
 'psk232': 'atmega1281',
 'psk233': 'atmega1281',
 'radiofaro': 'atmega128rfa1',
 'radiofaro_v1': 'atmega128rfa1',
 'raspbee': 'atmega256rfr2',
 'ravrf230a': 'atmega1284p',
 'ravrf230b': 'atmega1284p',
 'rbb128rfa1': 'atmega128rfa1',
 'rbb212': 'atmega1281',
 'rbb230': 'atmega1281',
 'rbb230b': 'atmega1281',
 'rbb231': 'atmega1281',
 'rbb232': 'atmega1281',
 'rbb233': 'atmega1281',
 'rdk212': 'atmega1281',
 'rdk230': 'atmega1281',
 'rdk230b': 'atmega1281',
 'rdk231': 'atmega1281',
 'rdk232': 'atmega1281',
 'rdk233': 'atmega1281',
 'rose231': 'atmega328p',
 'rzusb': 'at90usb1287',
 'stb128rfa1': 'atmega128rfa1',
 'stb212': 'atmega1281',
 'stb230': 'atmega1281',
 'stb230b': 'atmega1281',
 'stb231': 'atmega1281',
 'stb232': 'atmega1281',
 'stb233': 'atmega1281',
 'stb256rfr2': 'atmega256rfr2',
 'stkm16': 'atmega16',
 'stkm8': 'atmega8',
 'tiny230': 'attiny84',
 'tiny231': 'attiny84',
 'wdba1281': 'atmega1281',
 'wprog': 'atmega128rfa1',
 'xxo': 'atmega128rfa1',
 'zgbh212': 'atmega1281',
 'zgbh230': 'atmega1281',
 'zgbh231': 'atmega1281',
 'zgbl212': 'atmega1281',
 'zgbl230': 'atmega1281',
 'zgbl231': 'atmega1281',
 'zgbt1281a2nouart': 'atmega1281',
 'zgbt1281a2uart0': 'atmega1281',
 'zgbt1281a2uart1': 'atmega1281',
 'zigduino': 'atmega128rfa1'}
MMCU = MMCU_TABLE.get(BOARD,"?")

# data structure for config file handling
CFGP = ConfigParser.ConfigParser()

# This a workaround for obtaining the flashends for the current used MCU's.
# avr-gcc is used to extract the address.
# for i in $(awk -F'=' '/^cpu/{print $2}' Src/Lib/Inc/boards/board.cfg|sort|uniq);do x=$(echo "#include <avr/io.h>"|avr-gcc -mmcu=$i -E -dM -|awk '/FLASHEND/{print  $NF}');echo "\"$i\" : $x,"; done
#
FLASHEND = {
    "at90usb1287" : 0x1FFFF,
    "atmega128" : 0x1FFFF,
    "atmega1281" : 0x1FFFF,
    "atmega1284p" : 0x1FFFF,
    "atmega128rfa1" : 0x1ffff,
    "atmega256rfr2" : 0x3FFFF,
    "atmega16" : 0x3FFF,
    "atmega328" : 0x7FFF,
    "atmega328p" : 0x7FFF,
    "atmega644" : 0xFFFF,
    "atmega8" : 0x1FFF,
    "atmega88" : 0x1FFF,
    "atmega88pa" : (0x1FFF),
    "attiny84" : 0x1FFF,
    "atxmega256a3" : 0x41FFF,
}

CFGTEMPLATE = """
# In the "groups" section several nodes can be configured to be in one group,
# e.g. 1,3,4,5 are ravenboards.
[group]
#ravengang=1,3:5

# In the "board" section the hardware targets are assigned to the short
# addresses.
[board]
default = rdk230
#ravengang = ravrf230a
#0 = stb230

# In the "channel" section the default radio channel for the board is assigned.
[channel]
default=17

[pan_id]
default=0xdeaf

[ieee_addr]
#1=12:34:56:78:9a:bc:de:f0

[firmware]
default = install/bin/wuart_<board>.hex
#0 = install/bin/wibohost_<board>.hex
outfile = /tmp/node_<saddr>.hex

[bootloader]
default = install/bin/wibo_<board>.hex

#[offset]
#ravengang=0x00
"""

# payload of record w/o crc
NODE_CONFIG_FMT ="<HHQB2x"


# === functions ===============================================================
##
# Format an intel hex record
# For a description of the intel hex format see:
#  http://en.wikipedia.org/wiki/Intel_HEX
#
# @param rtype
#        record type
#          00     Data Record
#          01     End of File Record
#          02     Extended Segment Address Record
#          03     Start Segment Address Record
#          04     Extended Linear Address Record
#          05     Start Linear Address Record
# @param addr
#           16 bit address value
# @param data
#           list with 8 bit values.
# @return string of the formated record.
#
def ihex_record(rtype, addr,data = []):

    dlen = len(data) & 0xff
    darr  = [ dlen,
             (addr >> 8) & 0xff,
             (addr & 0xff) ,
              rtype & 0xff]
    darr.extend(data)
    crc = 0
    for d in darr:
        crc += d
    crc = ((crc &0xff)^0xff) + 1
    darr.append(crc & 0xff)
    return ":"+"".join(["%02X" %d  for d in darr])

##
# Dallas ibutton crc8.
#
# This implementation is based on avr-libc
# _crc_ibutton_update(), see http://www.nongnu.org/avr-libc/
#
# @param data
#           array of numbers or raw binary string.
# @return The computed crc8 value.
#
# The Dallas iButton test vector must return 0
# ibutton_crc( [ 0x02, 0x1c, 0xb8, 0x01, 0, 0, 0, 0xa2 ] )
#
def ibutton_crc(data):
    if isinstance(data,str):
        idata = map(ord, data)
    else:
        idata = data
    crc =  0
    for d in idata:
        crc = crc ^ d
        for i in range(8):
            if crc & 0x01:
                crc = (crc >> 1) ^ 0x8C
            else:
                crc >>= 1
    return crc

def resolve_value(cfg, sect, addr_key, group_key = None):
    # cascade for retriving values
    curr_sect = cfg.get(sect, {})
    rv = curr_sect.get(addr_key,
            curr_sect.get(group_key,
                curr_sect.get("default",
                    DEFAULTS.get(sect))))
    try:
        rv = eval(str(rv))
    except:
        pass
    return rv

def resolve_groups(cfg):
    rv = {}
    for k, v in cfg.get('group', {}).items():
        for a in v.split(","):
            tmp = map(eval, a.split(":"))
            if len(tmp) == 2:
                tmp2 = range(tmp[0],tmp[1]+1)
                tmp = tmp2
            for xx in tmp:
                rv[str(xx)] = k
    return rv

def resolve_parameters():
    global SADDR, LADDR, PANID, CHANNEL, BOARD, OFFSET
    global INFILE, BOOTLOADER, OUTFILE
    cfg = dict([(s,dict(CFGP.items(s))) for s in CFGP.sections()])
    group_keys =  resolve_groups(cfg)

    addr_key = "%d" % SADDR
    group_key = group_keys.get(addr_key)
    if PANID == None:
        PANID = resolve_value(cfg, "pan_id", addr_key, group_key)
    if LADDR == None:
        LADDR = resolve_value(cfg, "ieee_addr", addr_key, group_key)
    if SADDR == None:
        SADDR = resolve_value(cfg, "short_addr", addr_key, group_key)
    if CHANNEL == None:
        CHANNEL = resolve_value(cfg, "channel", addr_key, group_key)
    if BOARD == None:
        BOARD = resolve_value(cfg, "board", addr_key, group_key)
    if OFFSET == None:
        OFFSET = resolve_value(cfg, "offset", addr_key, group_key)
    if OFFSET == None:
        mmcu = MMCU_TABLE.get(BOARD,None)
        if mmcu:
            OFFSET = get_flashend_offset_for_node_config(mmcu)
        else:
            print "Failure: can not determine offset for"\
                  " mmcu=%s, board = %s" % (mmcu, BOARD)
            sys.exit(2)

    if INFILE == None:
        INFILE = resolve_value(cfg, "firmware", addr_key, group_key)
    if INFILE != None:
        INFILE = INFILE.replace("<board>", BOARD)
        if not os.path.exists(INFILE):
            print "Failure: firmware file '%s' not found" % INFILE
            sys.exit(3)
    if BOOTLOADER == None:
        BOOTLOADER = resolve_value(cfg, "bootloader", addr_key, group_key)
    if BOOTLOADER != None:
        BOOTLOADER = BOOTLOADER.replace("<board>", BOARD)
        if not os.path.exists(BOOTLOADER):
            print "Failure: bootloader file '%s' not found" % BOOTLOADER
            sys.exit(3)

    if OUTFILE == None:
        OUTFILE = resolve_value(cfg, "firmware", "outfile")
    if OUTFILE == None or OUTFILE == "-":
        OUTFILE = sys.stdout
    else:
        ofn = OUTFILE.replace("<saddr>", "0x%04X" % SADDR)
        ofn = ofn.replace("<board>", BOARD)
        OUTFILE = open(ofn ,"w")

    sys.stderr.write(\
        "Use Parameters:\n"\
        " board:      %s\n"\
        " addr_key:   %s\n"\
        " group_key:  %s\n"\
        " short_addr: 0x%04x\n"\
        " pan_id:     0x%04x\n"\
        " ieee_addr:  0x%016x\n"\
        " channel:    %d\n"\
        " offset:     0x%08x\n"\
        " infile:     %s\n"\
        " bootloader: %s\n"\
        " outfile:    %s\n\n"\
        % (BOARD, addr_key, group_key, SADDR, PANID, LADDR, CHANNEL, OFFSET,
            INFILE, BOOTLOADER, OUTFILE.name))

def get_flashend_offset_for_node_config(mmcu):
    return (FLASHEND[mmcu] - struct.calcsize(NODE_CONFIG_FMT + "B") + 1)

##
# Generating the node config structure.
#
# The format of the structure node_config_t is defined in board.h.
# @param memaddr
#           address, where to locate the record
#           Per default memaddr=None and the location FLASHEND is
#           used.
def generate_nodecfg_record(memaddr):
    ret = []
    # payload of record w/o crc
    extaddr = (memaddr >> 16)
    if (extaddr > 0):
        # is extended addr record needed ?
        #  a  b    c  d    e
        # :02 0000 02 1000 EC
        data = map(ord, struct.pack("<H",extaddr<<4))
        ret.append(ihex_record(2, 0, data))
    data = map(ord,struct.pack(NODE_CONFIG_FMT, SADDR, PANID, LADDR, CHANNEL))
    crc8 = ibutton_crc( data )
    data.append(crc8)
    ret.append(ihex_record(0, memaddr, data))
    return ret

##
# add a node cofg structure at the end of the flash.
#
def patch_hexfile(appfiles, fo, offset):
    END_RECORD = ":00000001FF"
    for ifile in appfiles:
        if ifile != None:
            fi = open(ifile,"r")
            for l in fi.xreadlines():
                if l.find(END_RECORD) == 0:
                    # end record found
                    break
                # write regular record
                fo.write(l)
            fi.close()
    nodecfg = generate_nodecfg_record(offset)
    fo.write("\n".join(nodecfg)+"\n")
    fo.write(END_RECORD)
    if fo != sys.stdout:
        fo.close()

def list_boards():
    bl = MMCU_TABLE.keys()
    bl.sort()
    print "BOARD            MMCU                 FLASH-END"
    for b in bl:
        mmcu = MMCU_TABLE[b]
        print "%-16s %-20s 0x%x" % (b, mmcu, FLASHEND.get(mmcu,0))

def list_mmcus():
    mmcus = FLASHEND.keys()
    mmcus.sort()
    print "BOARD               FLASH-END"
    for mmcu in mmcus:
        print "%-20s 0x%x" % (mmcu, FLASHEND.get(mmcu,0))

try:
    class EntryField(Tkinter.Frame):
        def __init__(self, master, text, textvariable=None):
            Tkinter.Frame.__init__(self, master=master)
            Tkinter.Label(master=self, text=text).pack(side=Tkinter.LEFT)
            Tkinter.Entry(master=self, textvariable=textvariable).pack(side=Tkinter.RIGHT)

    class Scrollbox(Tkinter.Frame):
        def __init__(self, master, *args, **kwargs):
            Tkinter.Frame.__init__(self, master, *args, **kwargs)
            self.bar=Tkinter.Scrollbar(master=self)
            self.bar.pack(side=Tkinter.RIGHT, fill=Tkinter.Y)
            self.box=Tkinter.Listbox(master=self, yscrollcommand=self.bar.set)
            self.box.pack(side=Tkinter.LEFT)
            self.bar.config(command=self.box.yview)


    class GUI(object):
        def __init__(self, parent):
            self.frame = Tkinter.Frame(parent)
            self.frame.pack()
            Tkinter.Label(master=self.frame, text=BOARD).pack()
            self.shortaddr = Tkinter.StringVar(value=str(hex(SADDR))) # start with default
            self.shortaddr.trace("w", self.cb_changed)
            self.longaddr = Tkinter.StringVar(value=str(hex(LADDR)))
            self.longaddr.trace("w", self.cb_changed)
            self.panid = Tkinter.StringVar(value=str(hex(PANID)))
            self.panid.trace("w", self.cb_changed)
            self.channel = Tkinter.StringVar(value=str(hex(CHANNEL)))
            self.channel.trace("w", self.cb_changed)
            sb=Scrollbox(master=self.frame)
            sb.pack(side=Tkinter.LEFT)
            [sb.box.insert(Tkinter.END, i) for i in MMCU_TABLE.keys()]
            EntryField(master=self.frame, text="Short Addr", textvariable=self.shortaddr).pack()
            EntryField(master=self.frame, text="Long Addr", textvariable=self.longaddr).pack()
            EntryField(master=self.frame, text="PAN Id", textvariable=self.panid).pack()
            EntryField(master=self.frame, text="Channel", textvariable=self.channel).pack()
            Tkinter.Button(master=self.frame, text="Done", command=self.frame.quit).pack()

        def cb_changed(self, *args):
            global SADDR, LADDR, PANID, CHANNEL
            SADDR = int(self.shortaddr.get(), 16)
            LADDR = int(self.longaddr.get(), 16)
            PANID = int(self.panid.get(), 16)
            CHANNEL = int(self.channel.get(), 16)

    def call_gui():
        """ Pop up Tkinter GUI to enter parameters """
        root = Tkinter.Tk()
        app=GUI(root)
        root.mainloop()
except:
    def call_gui():
        msg = "=" * 80 + "\n"\
              "Sorry, there seems to be no Tkinter within your Python "\
              "installation.\n"\
              "Please install python-tk package for your OS or simply use the "\
              "command line.\n"\
              "Use python %s --help to see the available options.\n" +\
              "=" * 80 + "\n"
        print msg % sys.argv[0]
        sys.exit(1)


# === classes =================================================================

if __name__ == "__main__":
    try:
        opts,args = getopt.getopt(sys.argv[1:],"a:p:A:f:b:hvo:c:O:B:lLM:C:G")
    except:
        msg = "=" * 80 +\
              "\nInvalid arguments. Please try python %s -h.\n" +\
              "=" * 80
        print msg % sys.argv[0]
        opts = [('-v',''),]
        args = []

    if opts == []:
        call_gui()

    #test vectors
    #x = ":1001600060E070E000E010E020E030E00E94D705A1"
    #addr = 0x0160
    #data = [ 0x60, 0xE0, 0x70, 0xE0, 0x00, 0xE0, 0x10, 0xE0,
    #         0x20, 0xE0, 0x30, 0xE0, 0x0E, 0x94, 0xD7, 0x05 ]
    #y = ihex_record(0,addr, data)
    doexit = False
    for o,v in opts:
        if o == "-a":
            SADDR = eval(v)
        elif o == "-A":
            LADDR = eval(v)
        elif o == "-p":
            PANID = eval(v)
        elif o == "-c":
            CHANNEL = eval(v)
        elif o == "-f":
            INFILE = v
        elif o == "-b":
            BOOTLOADER = v
        elif o == "-B":
            if MMCU_TABLE.has_key(v):
                BOARD = v
                MMCU = MMCU_TABLE.get(BOARD,"?")
        elif o == "-M":
            if FLASHEND.has_key(v):
                BOARD = "??"
                MMCU = v
        elif o == "-C":
            CFGFILE = v
        elif o == "-G":
            if not os.path.isfile(CFGFILE):
                f = open(CFGFILE, "w")
                f.write(CFGTEMPLATE)
                f.close()
                print "generated file %s" % f.name
            else:
                print "file %s does already exist" % CFGFILE
            doexit = True

        elif o == "-h":
            print __doc__
            doexit = True
        elif o == "-v":
            print "Version",VERSION
            doexit = True
        elif o == "-l":
            list_boards()
            doexit = True
        elif o == "-L":
            list_mmcus()
            doexit = True
        elif o == "-o":
            OUTFILE = v
        elif o == "-O":
            OFFSET = eval(v)

    if doexit:
        sys.exit(0)

    if os.path.isfile(CFGFILE):
        CFGP.read(CFGFILE)

    resolve_parameters()
    patch_hexfile([INFILE, BOOTLOADER], OUTFILE, OFFSET)

    try:
        import readline, rlcompleter
        readline.parse_and_bind("tab:complete")
    except:
        pass
