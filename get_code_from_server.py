from pwn import *

program = './capstone'
port = '2530'

l = process(program)
r = remote('aup.zoolab.org', port)


for i in range(10):
    s = r.recvline_startswith('>>>')
    print(s)
    l.sendline(s)
    c = l.recvline()
    # print(c)
    r.send(c)

flag = r.recvall()
print(flag)
