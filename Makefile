SHELL=cmd.exe
USE_DEBUG = NO
USE_STATIC = NO
USE_WINMSGS = NO

BASE_PATH=d:/tdm32/bin/

ifeq ($(USE_DEBUG),YES)
CFLAGS=-Wall -O -ggdb -mwindows 
LFLAGS=
else
CFLAGS=-Wall -O2 -mwindows 
LFLAGS=-s
endif
CFLAGS += -Weffc++
CFLAGS += -Wno-write-strings
CFLAGS += -Wno-stringop-truncation
ifeq ($(USE_STATIC),YES)
LIBS=-lgdi32 -lcomctl32 -static
else
LIBS=-lgdi32 -lcomctl32
endif

CSRC+=lrender.cpp fontmgr.cpp \
wFontShow.cpp tooltips.cpp \
common_funcs.cpp statbar.cpp winmsgs.cpp

ifeq ($(USE_WINMSGS),YES)
CSRC+=winmsgs.cpp 
CFLAGS += -DUSE_WINMSGS
endif

OBJS = $(CSRC:.cpp=.o) rc.o

BIN=wFontShow

#************************************************************
%.o: %.cpp
	$(BASE_PATH)g++ $(CFLAGS) -c $< -o $@

all: $(BIN).exe

clean:
	rm -f $(BIN).exe $(OBJS) *.zip *.bak *~

dist:
	rm -f $(BIN).zip
	zip -r $(BIN).zip $(BIN).exe *.f* fntcol\* readme.txt

wc:
	wc -l *.cpp *.rc

source:
	rm -f $(BIN).src.zip
	zip $(BIN).src.zip *

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) lintdefs.cpp $(CSRC)"

depend:
	makedepend $(CFLAGS) $(CSRC)

#************************************************************
$(BIN).exe: $(OBJS)
	$(BASE_PATH)g++ $(CFLAGS) $(LFLAGS) $(OBJS) -o $@ $(LIBS)

rc.o: $(BIN).rc 
	windres $< -O COFF -o $@

# DO NOT DELETE

lrender.o: common.h fontmgr.h lrender.h
fontmgr.o: common.h fontmgr.h
wFontShow.o: targetver.h resource.h common.h statbar.h fontmgr.h lrender.h
tooltips.o: targetver.h resource.h common.h
common_funcs.o: common.h
statbar.o: common.h statbar.h
