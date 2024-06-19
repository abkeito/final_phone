CC = gcc 
CFLAGS = -fdiagnostics-color=always -Wall -Iinclude
LDLIBS = -lm

SRCDIR = src
LIBDIR = lib
BINDIR = bin
INCDIR = include

COMMON_SOURCES = $(SRCDIR)/helper.c $(SRCDIR)/phone.c $(SRCDIR)/fft.c
COMMON_OBJECTS = $(COMMON_SOURCES:$(SRCDIR)/%.c=$(LIBDIR)/%.o)

ZOOM_SOURCES = $(SRCDIR)/client_zoom.c $(SRCDIR)/server_zoom.c
ZOOM_OBJECTS = $(ZOOM_SOURCES:$(SRCDIR)/%.c=$(LIBDIR)/%.o)
ZOOM_TARGET = $(BINDIR)/zoom.exe

zoom: $(ZOOM_TARGET)

$(ZOOM_TARGET): $(COMMON_OBJECTS) $(ZOOM_OBJECTS)
	$(CC) $^ -o $@ $(LDLIBS)

$(LIBDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDLIBS)

clean:
	rm -f $(LIBDIR)/*.o $(BINDIR)/*.exe

.PHONY: all clean tcp udp