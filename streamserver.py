from twisted.internet.protocol import Protocol
from twisted.protocols.basic import IntNStringReceiver
from twisted.internet.protocol import Factory
from twisted.internet.endpoints import TCP4ServerEndpoint
from twisted.internet import reactor
import struct

class DorfClient(Protocol):
    def connectionMade(self):
        self.factory.clients += (self,)
        self.transport.write(self.factory.get_initial_message())

class DorfClientFactory(Factory):
    protocol = DorfClient

    def __init__(self):
        self.clients = ()
        self.canvaswidth = 0
        self.canvasheight = 0
        self.frame = None

    def forward(self, string):
        string = struct.pack(DorfServer.structFormat, len(string))+string
        for c in self.clients:
            c.transport.write(string)

    def get_initial_message(self):
        if self.frame is None:
            return ""

        w = str(self.canvaswidth)
        h = str(self.canvasheight)
        message = w+" "+h+" 0 0 "+w+" "+h+"\n"
        message += str(self.frame)
        message = struct.pack("!L", len(message)) + message
        return message

class EnsureError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class DorfServer(IntNStringReceiver):
    structFormat = "!L"
    prefixLength = struct.calcsize(structFormat)

    def __init__(self, clients):
        self.clients = clients

    def ensure(self, label, var, least, most):
        if var < least or var > most:
            raise EnsureError("Invalid input; "+label+" = "+str(var)+", expected range ["+str(least)+", "+str(most)+"]")

    def stringReceived(self, string):
        (geom, data) = string.split('\n', 1)
        (cwidth, cheight, outputcol, outputrow, outputwidth, outputheight) = map(int, geom.split(' '))
        try:
            self.ensure("canvaswidth", cwidth, 0, 4096)
            self.ensure("canvasheight", cheight, 0, 4096)
            self.ensure("outputcol", outputcol, 0, cwidth)
            self.ensure("outputrow", outputrow, 0, cheight)
            self.ensure("outputwidth", outputwidth, 0, cwidth-outputcol)
            self.ensure("outputheight", outputheight, 0, cheight-outputrow)
        except EnsureError as e:
            print e
            self.loseConnection()

        if self.clients.frame is None or self.clients.canvaswidth <> cwidth or self.clients.canvasheight <> cheight:
            self.clients.canvaswidth = cwidth
            self.clients.canvasheight = cheight
            self.clients.frame = bytearray(cwidth*cheight*2)

        y = outputrow
        inputidx = 0
        while y < outputrow + outputheight:
            x = outputcol
            outputidx = (y*cwidth + x)*2
            outputtill = outputidx + outputwidth*2
            inputtill = inputidx + outputwidth*2
            self.clients.frame[outputidx:outputtill] = data[inputidx:inputtill]
            inputidx = inputtill
            y = y + 1

        clients.forward(string)

class DorfServerFactory(Factory):
    def __init__(self, clients):
        self.clients = clients

    def buildProtocol(self, addr):
        return DorfServer(self.clients)

clients = DorfClientFactory()
reactor.listenTCP(8008, clients)
reactor.listenTCP(8009, DorfServerFactory(clients))
reactor.run()
