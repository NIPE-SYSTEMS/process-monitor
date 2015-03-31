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

struct process
{
	pid_t pid;
	pid_t ppid;
	char *command;
	struct process *next;
	struct process *child;
};

struct process *process_new(pid_t pid, pid_t ppid, char *command)
{
	struct process *proc = malloc(sizeof(struct process));
	if(proc == NULL)
	{
		return NULL;
	}
	
	proc->pid = pid;
	proc->ppid = ppid;
	proc->command = strdup(command);
	proc->next = NULL;
	proc->child = NULL;
	
	return proc;
}

void process_free(struct process *proc)
{
	if(proc != NULL)
	{
		free(proc->command);
		
		if(proc->next != NULL)
		{
			process_free(proc->next);
		}
		
		if(proc->child != NULL)
		{
			process_free(proc->child);
		}
	}
	
	free(proc);
}

void process_print(struct process *proc, int shift)
{
	int i = 0;
	
	if(proc == NULL)
	{
		return;
	}
	
	for(i = 0; i < shift; i++)
	{
		printf("  ");
	}
	
	printf("%i\n", proc->pid);
	
	if(proc->child != NULL)
	{
		process_print(proc->child, shift + 1);
	}
	
	if(proc->next != NULL)
	{
		process_print(proc->next, shift);
	}
}

void process_append(struct process *proc0, struct process *proc1)
{
	if(proc0 == NULL || proc0->next != NULL || proc1 == NULL)
	{
		return;
	}
	
	proc0->next = proc1;
}

void process_append_child(struct process *parent, struct process *child)
{
	if(parent == NULL || parent->child != NULL || child == NULL)
	{
		return;
	}
	
	parent->child = child;
}

struct process *process_last(struct process *first)
{
	struct process *pointer = first;
	
	if(first == NULL)
	{
		return NULL;
	}
	
	while(pointer->next != NULL)
	{
		pointer = pointer->next;
	}
	
	return pointer;
}

struct process *process_last_child(struct process *first_child)
{
	struct process *pointer = first_child;
	
	if(first_child == NULL)
	{
		return NULL;
	}
	
	while(pointer->child != NULL)
	{
		pointer = pointer->child;
	}
	
	return pointer;
}

int process_count(struct process *proc)
{
	int i = 0;
	struct process *pointer = proc;
	
	while(pointer->next != NULL)
	{
		i++;
		pointer = pointer->next;
	}
	
	return i;
}

int process_count_children(struct process *proc)
{
	int i = 0;
	struct process *pointer = proc;
	
	while(pointer->child != NULL)
	{
		i++;
		pointer = pointer->child;
	}
	
	return i;
}

int main(void)
{
	// glibtop_proclist proclist;
	// glibtop_proc_state procstate;
	// glibtop_proc_uid procuid;
	// pid_t *list = NULL;
	// unsigned long int i = 0;
	
	// glibtop_init();
	
	// list = glibtop_get_proclist(&proclist, 0, 0);
	
	// for(i = 0; i < proclist.number; i++)
	// {
	// 	glibtop_get_proc_state(&procstate, list[i]);
	// 	glibtop_get_proc_uid(&procuid, list[i]);
		
	// 	printf("%6i: %40s %i %6i\n", list[i], procstate.cmd, procstate.state, procuid.ppid);
	// }
	
	// g_free(list);
	
	struct process *root_proc = process_new(1, 0, "Test");
	struct process *next_proc0 = process_new(2, 0, "Test2");
	struct process *next_proc1 = process_new(3, 0, "Test3");
	struct process *next_proc2 = process_new(4, 0, "Test4");
	process_append(process_last(root_proc), next_proc0);
	process_append(process_last(root_proc), next_proc1);
	process_append_child(process_last_child(root_proc), next_proc2);
	
	process_print(root_proc, 0);
	
	printf("Last PID: %i, last child PID: %p, %i, %i\n", process_last(root_proc)->pid, process_last_child(root_proc), process_count(root_proc), process_count_children(root_proc));
	printf("Last PID: %i, last child PID: %p, %i, %i\n", process_last(root_proc)->pid, process_last_child(root_proc), process_count(root_proc), process_count_children(root_proc));
	
	process_free(root_proc);
	
	return 0;
}