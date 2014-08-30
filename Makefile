OBJS=server.o simple_http.o content.o main.o util.o
CFLAGS=-g -I. -Wall -Wextra -pthread
#DEFINES=-DTHINK_TIME
BIN=server
CC=gcc
PORT=8080
NUM=10000

%.o:%.c
	$(CC) $(CFLAGS) $(DEFINES) -o $@ -c $<

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -o $(BIN) $^

clean:
	rm $(BIN) $(OBJS)

# Use 'make testX PORT=XXXX' when the address is used.
# USE 'NUM = XX' change default times. 08/30/2014 
test0:
	./server $(PORT) 0 &
	httperf --port=$(PORT) --server=localhost --num-conns=1
	killall server

test1:
	./server $(PORT) 1 &
	httperf --port=$(PORT) --server=localhost --num-conns=$(NUM) --burst-len=100
	killall server

test2:
	./server $(PORT) 2 &
	httperf --port=$(PORT) --server=localhost --num-conns=$(NUM) --burst-len=100
	killall server

