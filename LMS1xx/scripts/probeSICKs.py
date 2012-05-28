#!/usr/bin/env python
import struct
import socket
import threading
import SocketServer
from time import sleep

class ThreadedUDPRequestHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        data = self.request[0]
        if data[:2] == '\x90\x61':
            size = struct.unpack('!h', data[2:4])
            print 'XML size: %d' % size
            print 'MAC address: ' + ':'.join(['%02X' % struct.unpack('B', char) for char in data[4:10]])
            print 'Requester IP: %s' % socket.inet_ntoa(data[10:14])
            from xml.dom.minidom import parseString
            xml_response_dom = parseString(data[16:])
            print 'XML reply:\n%s' % xml_response_dom.toprettyxml(indent='  ')

class ThreadedUDPServer(SocketServer.ThreadingMixIn, SocketServer.UDPServer):
    pass

padding = '\x00\x00'
host = socket.gethostbyname(socket.gethostname())
data = struct.pack('<i', 16) + '\xff\xff\xff\xff\xff\xff' + socket.inet_aton(host) + padding

if __name__ == '__main__':
    # Port 0 means to select an arbitrary unused port
    HOST, PORT = '<broadcast>', 30718
    
    server = ThreadedUDPServer(('<broadcast>', PORT), ThreadedUDPRequestHandler)
    ip, port = server.server_address
    
    # Start a thread with the server -- that thread will then start one
    # more thread for each request
    server_thread = threading.Thread(target=server.serve_forever)
    # Exit the server thread when the main thread terminates
    server_thread.daemon = True
    server_thread.start()
    print 'Listening for SICK LMS[1|5]xx UDP probe responses.'
    
    while True:
        action = raw_input('Type P to probe the network for SICK LMS[1|5]xx devices,\nor X to exit: ')
        if action.lower() == 'p':
            udp_broadcast_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            udp_broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            udp_broadcast_socket.sendto(data, ('<broadcast>', 30718))
            print 'Probe sent, sleeping for 5 seconds...'
            sleep(5)
        elif action.lower() == 'x':
            break
    
    server.shutdown()
