CC=gcc

CC=gcc
CFLAGS=-I.
#CFLAGS=
DEPS= 
OBJ11 = daytimetcpcliv6.o
OBJ12 = daytimetcpsrvv6.o 

OBJ2 = checkopts.o

OBJ31 = daytimetcpcliv6_02.o 
OBJ32 = daytimetcpsrvv6_02.o

OBJ41 = daytimetcpcliv4_02.o
OBJ42 = daytimetcpsrvv4_02.o

OBJ51 = daytimetcpcliv4_03.o
OBJ52 = daytimetcpsrvv4_03.o
OBJ53 = daytimetcpcliv4_04.o

OBJ61 = daytimetcpcliv6_03.o
OBJ62 = daytimetcpsrvv6_03.o  

OBJ71 = daytimetcpcliv6_04.o
OBJ72 = daytimetcpsrvv6_04.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: dttcv6 dttsv6 checkopts  dttcv6_02 dttsv6_02  dttcv4_02 dttsv4_02 dttcv4_03 dttsv4_03 dttcv4_04 dttcv6_03 dttsv6_03 dttcv6_04 dttsv6_04


dttcv6: $(OBJ11)
	gcc -o $@ $^ $(CFLAGS)
dttsv6: $(OBJ12)
	gcc -o $@ $^ $(CFLAGS)

checkopts: $(OBJ2)
	gcc -o $@ $^ $(CFLAGS)

dttcv6_02: $(OBJ31)
	gcc -o $@ $^ $(CFLAGS)	
dttsv6_02: $(OBJ32)
	gcc -o $@ $^ $(CFLAGS)
	
dttcv4_02: $(OBJ41)
	gcc -o $@ $^ $(CFLAGS)
dttsv4_02: $(OBJ42)
	gcc -o $@ $^ $(CFLAGS)	

dttcv4_03: $(OBJ51)
	gcc -o $@ $^ $(CFLAGS)	
dttsv4_03: $(OBJ52)
	gcc -o $@ $^ $(CFLAGS)	
dttcv4_04: $(OBJ53)
	gcc -o $@ $^ $(CFLAGS)	

dttcv6_03: $(OBJ61)
	gcc -o $@ $^ $(CFLAGS)
dttsv6_03: $(OBJ62)
	gcc -o $@ $^ $(CFLAGS)

dttcv6_04: $(OBJ71)
	gcc -o $@ $^ $(CFLAGS)
dttsv6_04: $(OBJ72)
	gcc -o $@ $^ $(CFLAGS)
	
clean:
	rm *.o dttcv6 dttsv6 checkopts  dttcv6_02 dttsv6_02  dttcv4_02 dttsv4_02 dttcv4_03 dttsv4_03 dttcv4_04 dttcv6_03 dttsv6_03 dttcv6_04 dttsv6_04
