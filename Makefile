#/*********************************************
#license:	GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007
#author:		arvik
#email:		1216601195@qq.com
#blog:		http://blog.csdn.net/u012819339
#github:		https://github.com/arviklinux/impush
#date:		2017.03.07
#*********************************************/


#################################################
#you may change some env below
#可填写交叉编译器
CC=gcc

#软件运行的目标机器的位数，如果是32位则定义该MACHINE_32BIT，64位机器则定义为空
#macro32bit=-DMACHINE_32BIT

##################################################



LIBSRC:=$(wildcard aklib/ak*.c)
GOALLIB:=libak.so

#COMMONSRC:=im_prase.c im_epoll.c im_rbtimer.c im_ss.c
COMMONSRC:=$(wildcard imsrc/im_*.c)

LDFLAGS+=-lak
CFLAGS+=-L./ -Iaklib/ -Iimsrc/

CC=gcc

all:clean server client
	@echo "complete compile!"

server:aklibc
	$(CC) -o impushserver $(COMMONSRC) imsrc/impush_server.c $(CFLAGS) $(LDFLAGS)

aklibc:
	$(CC)  $(LIBSRC) -fPIC -shared -o $(GOALLIB)

client:
	$(CC) -o imclient imclib/imclib.c imclib/client_main.c

install:
	cp $(GOALLIB) /lib/$(GOALLIB)

.PHONY:clean
clean:
	-rm -f $(EXEC) *.elf *.gdb *.o *.so impushserver imclient impub libak.so
