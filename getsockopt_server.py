#!/usr/bin/env python

import struct
from socket import socket, AF_UNIX, SOCK_STREAM, SOL_SOCKET

SO_PEERCRED = 17 # Pulled from /usr/include/asm-generic/socket.h

s = socket(AF_UNIX, SOCK_STREAM)

s.bind('/tmp/pass_cred')
s.listen(1)

conn, addr = s.accept()

creds = conn.getsockopt(SOL_SOCKET, SO_PEERCRED, struct.calcsize('3i'))

pid, uid, gid = struct.unpack('3i',creds)

print 'pid: %d, uid: %d, gid %d' % (pid, uid, gid)