#Redis-pppd

Redis-pppd is a pretty simple pppd plugin using Redis as authentication database. It was designed to replace my vulnerable file-based PPTP user info database. Rather than a full-functionality plugin, you'd better take it as a example of how to write plugins for pppd.

##Compile & Install

Before compiling, you need to modify the following line:

	#define COMMAND "GET com.example.pppd."
	
It refers to the prefix of your pppd user information in Redis.

To compile it, you need to have hiredis and pppd installed.

	gcc -c -O redis-pppd.c -fPIC
	gcc -shared -o redis-pppd.so redis-pppd.o -lhiredis
	
Move it to /usr/lib/pppd/<Current pppd version>/ :

	mv redis-pppd.so /usr/lib/pppd/2.4.5/redis-pppd.so
	
Add the following line to your pppd config file:

	plugin redis-pppd.so
	redis_host 127.0.0.1
	redis_port 6379
	
Enjoy it!

##Author & Licence
Redis-pppd was written by Minsheng Liu (notcome at me dot com) and is released under the BSD license.
