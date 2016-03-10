#!/usr/bin/env python

from socket import socket, AF_UNIX, SOCK_STREAM, SOL_SOCKET

SO_PASSCRED = 16 # Pulled from /usr/include/asm-generic/socket.h

s = socket(AF_UNIX, SOCK_STREAM)

s.setsockopt(SOL_SOCKET, SO_PASSCRED, 1)

s.connect('/tmp/pass_cred')

s.close()