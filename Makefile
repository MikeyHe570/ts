CC=g++
LIBS = -lpthread 
CFLAGS = -Wall
TARGET = ts_http_server
RM =rm -f
OBJS =main.o para_init.o workers_manage.o server.o timer.o client_handle.o epoll_event_handle.o
$(TARGET) :$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS)
clean :
	$(RM) $(TARGET) $(OBJS)
