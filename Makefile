OBJS=server.o simple_http.o content.o main.o util.o
CFLAGS=-g -I. -Wall -Wextra -pthread
#DEFINES=-DTHINK_TIME
BIN=server
CC=gcc
PORT=8080

%.o:%.c
	$(CC) $(CFLAGS) $(DEFINES) -o $@ -c $<

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -o $(BIN) $^

clean:
	rm $(BIN) $(OBJS)

# Use 'make testX PORT=XXXX' when the address is used.
test0:
	./server $(PORT) 0 &
	httperf --port=$(PORT) --server=localhost --num-conns=1
	killall server

test1:
	./server $(PORT) 1 &
	httperf --port=$(PORT) --server=localhost --num-conns=1000 --burst-len=100
	killall server

test2:
	./server $(PORT) 2 &
	httperf --port=$(PORT) --server=localhost --num-conns=1000 --burst-len=100
	killall server
