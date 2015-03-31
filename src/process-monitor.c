/**
 * Copyright (C) 2015 NIPE-SYSTEMS
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <glibtop.h>
#include <glibtop/proclist.h>
#include <glibtop/procstate.h>
#include <glibtop/procuid.h>

int main(void)
{
	glibtop_proclist proclist;
	glibtop_proc_state procstate;
	glibtop_proc_uid procuid;
	pid_t *list = NULL;
	unsigned long int i = 0;
	
	glibtop_init();
	
	list = glibtop_get_proclist(&proclist, 0, 0);
	
	for(i = 0; i < proclist.number; i++)
	{
		glibtop_get_proc_state(&procstate, list[i]);
		glibtop_get_proc_uid(&procuid, list[i]);
		
		printf("%6i: %40s %i %6i\n", list[i], procstate.cmd, procstate.state, procuid.ppid);
	}
	
	g_free(list);
	
	return 0;
}