ifeq (${CROSS},1)
CC=i686-pc-mingw32-gcc -I/usr/local/cross-tools/i386-mingw32/include
LDFLAGS += -L/usr/local/cross-tools/i386-mingw32/lib
EXE=.exe
endif

SRC1=  pbxpm.cc
SRC=pbres.c $(SRC1)

PROG=xpbres$(EXE) xpb2pb$(EXE)

all: $(PROG)

xpbres$(EXE): $(SRC)
	$(CC) -g -o $@ $^ $(LDFLAGS) -lz -lstdc++ 


xpb2pb$(EXE): $(SRC1)
	$(CC) -DCONVERT -g -o $@ $^ $(LDFLAGS) -lz -lstdc++ 
	