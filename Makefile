# Simple Makefile for building FocusApp on Windows with MinGW or MSVC

CC = g++
RC = windres
TARGET = FocusApp.exe
OBJS = main.o TimerWidget.o AppMonitor.o Config.o
RES = resource.res

CXXFLAGS = -std=c++17 -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -Wl,--subsystem,windows -Wl,-entry,wWinMainCRTStartup
LDFLAGS = -luser32 -lgdi32 -lshell32 -lcomctl32 -lpsapi

all: $(TARGET)

$(TARGET): $(OBJS) $(RES)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(RES) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

$(RES): resource.rc
	$(RC) -O coff -i resource.rc -o $@

clean:
	rm -f $(TARGET) $(OBJS) $(RES)

.PHONY: all clean
