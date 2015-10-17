# Copyright (C) 2015 NIPE-SYSTEMS
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CC = gcc

CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += `pkg-config --cflags glib-2.0`
CFLAGS += `pkg-config --cflags gio-2.0`

CFLAGS_WAIT += -Wall
CFLAGS_WAIT += -Wextra

LIBS += `pkg-config --libs glib-2.0`
LIBS += `pkg-config --libs gio-2.0`

.PHONY: all process-monitor init clean

all: process-monitor wait

# Compile main program

process-monitor: init bin/obj/process-monitor.o bin/obj/process_retrieve.o
	$(CC) bin/obj/process-monitor.o bin/obj/process_retrieve.o $(CFLAGS) -o bin/process-monitor $(LIBS)

wait: init src/wait.c
	$(CC) src/wait.c $(CFLAGS_WAIT) -o bin/wait

# Initializes directories

init:
	mkdir -p bin
	mkdir -p bin/obj

# C-objects

bin/obj/process-monitor.o: src/process-monitor.c
	$(CC) $(CFLAGS) -c -o bin/obj/process-monitor.o src/process-monitor.c $(LIBS)

bin/obj/process_retrieve.o: src/process_retrieve.c
	$(CC) $(CFLAGS) -c -o bin/obj/process_retrieve.o src/process_retrieve.c $(LIBS)

# Clean

clean:
	rm -Rf bin