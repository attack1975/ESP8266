
all:
	gcc -g -O0 main.c -o out
	gcc -g -O0  -std=c99 -D_POSIX_C_SOURCE gzip.c -o gzip -lz
	gcc -g -O0 -std=c99 -D_POSIX_C_SOURCE -I/home/wouters/GIT/willemwouters/ESP8266/projects/apps/websocket_example/giftest/libwebsockets/lib -L/home/wouters/GIT/willemwouters/ESP8266/projects/apps/websocket_example/giftest/libwebsockets/build/lib md5.c send.c -o send -lpthread -lwebsockets
	gcc -g -O0 -std=c99 -D_POSIX_C_SOURCE -I/home/wouters/GIT/willemwouters/ESP8266/projects/apps/websocket_example/giftest/libwebsockets/lib -L/home/wouters/GIT/willemwouters/ESP8266/projects/apps/websocket_example/giftest/libwebsockets/build/lib md5.c send_gif.c -o send_gif -lpthread -lwebsockets
