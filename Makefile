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
CFLAGS += `pkg-config --cflags libgtop-2.0`

LIBS += `pkg-config --libs libgtop-2.0`

.PHONY: all process-monitor init clean

all: process-monitor

# Compile main program

process-monitor: init bin/obj/process-monitor.o
	$(CC) bin/obj/process-monitor.o $(CFLAGS) -o bin/process-monitor $(LIBS)

# Initializes directories

init:
	mkdir -p bin
	mkdir -p bin/obj

# C-objects

bin/obj/process-monitor.o: src/process-monitor.c
	$(CC) $(CFLAGS) -c -o bin/obj/process-monitor.o src/process-monitor.c $(LIBS)

bin/obj/walk.o: src/walk.c
	$(CC) $(CFLAGS) -c -o bin/obj/walk.o src/walk.c $(LIBS)

bin/obj/processor.o: src/processor.c
	$(CC) $(CFLAGS) -c -o bin/obj/processor.o src/processor.c $(LIBS)

bin/obj/path_helper.o: src/path_helper.c
	$(CC) $(CFLAGS) -c -o bin/obj/path_helper.o src/path_helper.c $(LIBS)

bin/obj/archive.o: src/archive.c
	$(CC) $(CFLAGS) -c -o bin/obj/archive.o src/archive.c $(LIBS)

bin/obj/arguments.o: src/arguments.c
	$(CC) $(CFLAGS) -c -o bin/obj/arguments.o src/arguments.c $(LIBS)

# Clean

clean:
	rm -Rf bin