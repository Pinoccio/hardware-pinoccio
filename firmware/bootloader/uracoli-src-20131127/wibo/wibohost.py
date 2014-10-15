# $Id$
"""
wibo host utility

Usage:
    python wibo_host.py [OPTIONS]

    Options:
      -P PORT : Port string ("port" or "port:baudrate"), e.g. /dev/ttyS0, COM1:38400
                [default COM2:38400]
      -h      : show help and exit
      -V      : show version and exit
      -a ADDR : set current node addresses, e.g. -a 1, -a 1,3,5 or -a 1:10,15
                [default 1:8]
      -u FILE : selectively update nodes selected by ADDR with FILE
      -U FILE : broadcast update nodes with FILE
      -S      : scan for nodes in range min(ADDR):max(ADDR),
      -J      : send jump_to_bootloader frame to nodes selected by ADDR
      -E      : send exit_from_bootloader frame to nodes selected by ADDR
      -c CHANS: issue jump bootloader over the given channels, default: [11]
      -v      : increase verbose level

      Examples:

"""
import serial, string, re, time, sys, getopt
HISTORY = "wibohost.hist"
VERSION = 0.01


class WIBOHostBase(object):
    def __init__(self):
        self.VERBOSE = 0

    def ping(self, nodeid):
        """ Ping a dedicated node and evaluate reply """
        raise Exception("not implemented")

    def deaf(self, nodeid):
        """ Set node to deaf """
        raise Exception("not implemented")

    def finish(self, nodeid):
        raise Exception("not implemented")

    def feedhex(self, nodeid, ln):
        raise Exception("not implemented")

    def reset(self):
        raise Exception("not implemented")

    def exit(self, nodeid):
        raise Exception("not implemented")

    def crc(self):
        raise Exception("not implemented")

    def echo(self, dstr):
        raise Exception("not implemented")

    def info(self, nodeid):
        raise Exception("not implemented")

    def target(self, targ):
        """ Set memory target
            'E' : EEPROM
            'F' : Flash memory
            'X' : No memory, dry run
        """
        raise Exception("not implemented")

    def jbootl(self, nodeid):
        """ Jump to bootloader """
        raise Exception("not implemented")

    def bootlup(self, nodeid):
        """ Initiate Bootloader update """
        raise Exception("not implemented")

    def channel(self, channel):
        """ Set channel """
        raise Exception("not implemented")

class WIBOHost(WIBOHostBase, serial.Serial):
    """ Command to interface to WIBO Host device via serial line """

    def __init__(self, *args, **kwargs):
        """
            Initialize serial port
        """
        WIBOHostBase.__init__(self)
        serial.Serial.__init__(self, *args, **kwargs)
        self.flt = re.compile("(?P<code>[A-Z]+)([ ]?)(?P<data>.*)")
        self.cmdcnt = 0

    def _flush(self):
        """
            Internal function
            Flush serial buffer, used before issuing command
        """
        self.read(self.inWaiting())

    def _sendcommand(self, cmd, *args):
        """
            Internal function
            Send command to device, wait response and evaluate it
        """
        self.cmdcnt += 1
        self._flush()
        cmd = string.join(map(str,[cmd]+list(args)))
        if self.VERBOSE > 2:
            print "TX[%d]: %s" % (self.cmdcnt, cmd)
        self.write(cmd + '\n')
        # TODO evaluate returning line and parse for parameters
        s = self.readline().strip()
        if self.VERBOSE > 2:
            print "RX[%d]: %s" % (self.cmdcnt, s)
        m = self.flt.match(s)
        if m == None:
            return dict(code = "NO RESPONSE", data = cmd)
        else:
            return m.groupdict()

    def ping(self, nodeid):
        """ Ping a dedicated node and evaluate reply """
        ret = self._sendcommand('ping', hex(nodeid))
        if ret['code'] == 'OK': ret['data'] = eval(ret['data'])
        return ret

    def deaf(self, nodeid):
        """ Set node to deaf """
        return self._sendcommand('deaf', hex(nodeid))

    def finish(self, nodeid):
        return self._sendcommand('finish', hex(nodeid))

    def feedhex(self, nodeid, ln):
        return self._sendcommand('feedhex', hex(nodeid), ln)

    def reset(self):
        return self._sendcommand('reset')

    def exit(self, nodeid):
        return self._sendcommand('exit', hex(nodeid))

    def crc(self):
        return self._sendcommand('crc')

    def echo(self, dstr):
        return self._sendcommand('echo', dstr)

    def info(self, nodeid):
        return self._sendcommand('info', hex(nodeid))

    def target(self, targ):
        """ Set memory target
            'E' : EEPROM
            'F' : Flash memory
            'X' : No memory, dry run
        """
        return self._sendcommand('target', targ)

    def jbootl(self, nodeid):
        """ Jump to bootloader """
        return self._sendcommand('jbootl', hex(nodeid))

    def bootlup(self, nodeid):
        """ Initiate Bootloader update """
        return self._sendcommand('bootlup', hex(nodeid))

    def channel(self, channel):
        """ Set channel """
        return self._sendcommand('channel', hex(channel))

    def panid(self, pan_id):
        """ Set channel """
        return self._sendcommand('panid', hex(pan_id))

class NodeList(list):
    """ Little helper class to pretty print a list of nodes in ascii """
    def __init__(self, *args, **kwargs):
        list.__init__(self, *args, **kwargs)
    def __repr__(self):
        keys=['short_addr',
                'boardname',
                'appname',
                'status',
                'target',
                'version',
                ] # to have the dict sorted
        keymap={'short_addr':'ADDR',
                'boardname':'BOARD',
                'appname':'APP',
                'status':'STATUS',
                'target':'TARGET',
                'version':'VERSION',
            }
        if self.__len__() == 0:
            ret = '-empty-'
        else:
            colwidth = {}
            [colwidth.update({k:len(keymap[k])}) for k in keys]
            for k in keys:
                for d in self.__iter__():
                    l=len(str(d[k]))
                    if l > colwidth[k]: colwidth[k] = l
            [colwidth.update({k:v+2}) for k,v in colwidth.iteritems()]

            ret=""
            for k in keys:
                ret += keymap[k] + ' '*(colwidth[k]-len(keymap[k]))
            ret += '\n'
            for d in self.__iter__():
                for k in keys:
                    s=str(d[k])
                    ret +=  s + ' '*(colwidth[k]-len(s))
                ret += '\n'
        return ret

class WIBONetwork(WIBOHost):
    """ Class to represent a list of WIBO nodes """

    def __init__(self, *args, **kwargs):
        """ Constructor """
        WIBOHost.__init__(self, *args, **kwargs)
        self.nodes = NodeList()

    def ping(self, nodeid):
        defaults = {'status':None, 'target':'F'}
        ret = WIBOHost.ping(self, nodeid) # yes, use this one
        if ret['code'] == 'OK':
            data=ret['data']
            if data['short_addr'] not in [i['short_addr'] for i in self.nodes]:
                data.update(defaults)
                self.nodes.append(data)
            else:
                [n.update(data) for n in self.nodes if n['short_addr']==data['short_addr']]
        return ret

    def scan(self, scanrange=None):
        """ Scan a range of nodes by pinging them """
        if self.VERBOSE >= 1:
            print "scan nodes", scanrange,

        if scanrange != None:
            for a in scanrange:
                tmp = self.ping(a)
                if self.VERBOSE >= 2:
                    print "0x%04x" % tmp['data']['short_addr'],
                    if (len(self.nodes) % 4) == 0:
                        print "\n        ",
                else:
                    if self.VERBOSE >= 2: print "Already in list"
        else:
            lst=[]
            self.reset()
            self.nodes = NodeList()
            while True:
                tmp=self.ping(0xFFFF) # broadcast
                if tmp['code'] == 'OK':
                    lst.append(tmp['data'])

                    # TODO: command not supported in earlier versions
                    self.deaf(tmp['data']['short_addr'])
                else:
                    break
        if self.VERBOSE >= 1:
            print "\nFound %d devices\n" % len(self.nodes)

    def flashhex(self, nodeid=0xFFFF, fname=None):
        """ Flash hex-file to nodeid (single node or broadcast) """
        f=open(fname)
        self.reset()
        for i, ln in enumerate(f):
            ret=self.feedhex(nodeid, ln.strip())
            if ret['code'] == 'ERR':
                print 'ERR', ret['data']
                break
            elif ret['code'] == 'WARN':
                print 'WARN', ret['data']
            if self.VERBOSE >= 1:
                print "line %-4d crc: 0x%04x\r" % (i, int(self.crc()['data'], 16)),
                sys.stdout.flush()
            elif self.VERBOSE > 1:
                print i, ln.strip(), self.crc()['data']

        self.finish(nodeid)
        f.close()

    def checkcrc(self):
        """ Check CRC of node list and compare to the local CRC of host    """
        hostcrc = self.crc()['data']
        for n in self.nodes:
            p = self.ping(n['short_addr'])
            if p['code'] == 'OK':
                if (hostcrc == p['data']):
                    n['status'] = 'OK'
                else:
                    n['status'] = 'FAIL'
            else: # no reply
                n['status']='DISCONNECT'

    def filter_board(self, boardname):
        """ Set target to "Flash" for boards matching boardname, "X" (dry run) else """
        for n in self.nodes:
            if n['boardname'] == boardname:
                targ='F'
            else: targ='X'

            n['target']=targ # local list
            ret=self.target(n['short_addr'], 'F') # write to device
            if ret['code'] != 'OK': raise Exception("Could not set target")

def init_prompt():
    global HISTORY
    try:
        import readline, rlcompleter, atexit, sys, os
        sys.ps1 = "wibo>"
        readline.parse_and_bind("tab:complete")
        save_hist = lambda history : readline.write_history_file(history)
        atexit.register(readline.write_history_file, HISTORY)
        if os.path.exists(HISTORY):
            readline.read_history_file(HISTORY)
    except:
        print "No libreadline support"
        traceback.print_exc()

def param_evaluate_list(arg):
    lst = []
    for a in arg.split(","):
        a = a.split(":")
        if len(a)>1:
            lst += eval("range(%s,%s+1)" % (a[0],a[-1]))
        else:
            lst.append(eval(a[0]))
    lst.sort()
    return lst

def process_command_line():
    wnwk=WIBONetwork()
    ADDRESSES = None
    VERBOSE = 0
    CHANNELS = None
    PORT = None
    BAUDRATE = None
    ret = False

    try:
        opts,args = getopt.getopt(sys.argv[1:],"c:P:a:U:u:hVSJvEd:")
    except getopt.GetoptError,e:
        print "="*80
        print "Error:", e
        print "="*80
        opts = (("-h",""),)

    # options containing parameters that are required to work
    for o,v in opts:
        if o == "-h":
            print __doc__
            ret = True
            break
        elif o == "-V":
            print "wibohost v.", VERSION
            ret = True
            break
        elif o == "-v":
            VERBOSE+=1
        elif o == "-P":
            try:
                p,b = v.split(":")
                PORT = p
                BAUDRATE = int(b)
            except ValueError:
                print "Invalid baudrate"
                raise ValueError
        elif o == "-a":
            ADDRESSES = param_evaluate_list(v)
        elif o == "-c":
            CHANNELS = param_evaluate_list(v)

    if ret == False:
        wnwk.VERBOSE = VERBOSE
        wnwk.close()
        wnwk.setTimeout(1.0)
        wnwk.setPort(PORT)
        wnwk.setBaudrate(BAUDRATE)
        wnwk.open()
        time.sleep(0.1) # some time for boot printout
        for i in range(10):
            x = wnwk.echo("HalloWibo")
            if x["code"] == "OK":
                break
            else:
                time.sleep(.1)
        if i == 9:
            raise Exception("FATAL", "unable to connect wibo host %s" % x)

        for o,v in opts:
            if o == "-u":
                print "selective flashing nodes", ADDRESSES
                for n in ADDRESSES:
                    print "flash node", n
                    tmp = wnwk.ping(n)
                    if tmp['code'] == 'OK' and tmp['data']['appname'] == "wibo":
                            wnwk.flashhex(n,v)
                            print "                      \r"\
                                "file: %s, node: 0x%04x, crc: 0x%04x" % \
                                (v, n, int(wnwk.crc()['data'], 16))
                            print "CRC", wnwk.checkcrc() # TODO: rework
                            print "EXIT", wnwk.exit(n)
                    else:
                        print "node %d is not responding" % n

            elif o == "-U":
                if CHANNELS == None:
                    wnwk.scan(ADDRESSES)
                else:
                    for c in CHANNELS:
                        if VERBOSE:
                            print "\nchan %d:" % c,
                        wnwk.channel(c)
                        wnwk.scan(ADDRESSES)
                ADDRESSES = [n['short_addr'] for n in wnwk.nodes]
                listeners = []
                for n in ADDRESSES:
                    ret = wnwk.ping(n)
                    if ret['data']['appname'] == "wibo":
                        listeners.append(n)
                if len(listeners) == 0:
                    print "skip flashing, no listeners"
                else:
                    print "broadcast flashing nodes:", listeners
                    wnwk.flashhex(0xffff,v)
                    print "                      \r"\
                        "file: %s, node: 0x%04x, crc: 0x%04x" % \
                        (v, n, int(wnwk.crc()['data'], 16))
                    for n in listeners:
                        print "CRC:",n, wnwk.checkcrc() # TODO: rework
                        print "EXIT:",n, wnwk.exit(n)
            elif o == "-S":
                if CHANNELS == None:
                    wnwk.scan(ADDRESSES)
                else:
                    for c in CHANNELS:
                        if VERBOSE:
                            print "\nchan %d:" % c,
                        wnwk.channel(c)
                        wnwk.scan(ADDRESSES)
                ADDRESSES = [n['short_addr'] for n in wnwk.nodes]
                if VERBOSE and len(wnwk.nodes):
                    pkeys = sorted(wnwk.nodes[0].keys())
                    print ", ".join(pkeys)
                    for n in wnwk.nodes:
                        print ", ".join([str(n[k]) for k in pkeys])
                print "SCAN:", ADDRESSES
            elif o == "-J":
                if VERBOSE:
                    print "Jump to bootloader",
                if CHANNELS == None:
                    for n in ADDRESSES:
                        if VERBOSE>1:
                            print "\n node: ", n,
                        wnwk.jbootl(n)
                else:
                    for c in CHANNELS:
                        if VERBOSE>1:
                            print "\nchannel: %d, node: " % c,
                        wnwk.channel(c)
                        for n in ADDRESSES:
                            if VERBOSE>1:
                                print n,
                            wnwk.jbootl(n)
                if VERBOSE:
                    print
                listeners = []
                for n in ADDRESSES:
                    res = wnwk.ping(n)
                    if res['code'] == 'OK':
                        if res.get("appname") == "wibo":
                            listeners.append(n)
                print "Wibo listeners:", listeners
            elif o == "-E":
                for n in ADDRESSES:
                    wnwk.exit(n)
                    print "EXIT:",n, wnwk.exit(n)
    wnwk.close()
    return ret

if __name__ == "__main__":
    do_exit = process_command_line()
    if do_exit:
        sys.exit(0)
    # init_prompt()
