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
		printf("      ");
	}
	
	printf("%6i\n", proc->pid);
	
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
	if(first_child == NULL || first_child->child == NULL)
	{
		return first_child;
	}
	
	return process_last(first_child->child);
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
	if(proc == NULL || proc->child == NULL)
	{
		return 0;
	}
	
	return process_count(proc->child) + 1;
}

struct process *process_search_pid(struct process *proc, pid_t pid)
{
	struct process *found_proc = NULL;
	
	if(proc == NULL)
	{
		return NULL;
	}
	
	if(proc->pid == pid)
	{
		return proc;
	}
	
	if(proc->next != NULL)
	{
		found_proc = process_search_pid(proc->next, pid);
		
		if(found_proc != NULL)
		{
			return found_proc;
		}
	}
	
	if(proc->child != NULL)
	{
		found_proc = process_search_pid(proc->child, pid);
		
		if(found_proc != NULL)
		{
			return found_proc;
		}
	}
	
	return NULL;
}

int main(void)
{
	glibtop_proclist proclist;
	glibtop_proc_state procstate;
	glibtop_proc_uid procuid;
	pid_t *list = NULL;
	unsigned long int i = 0;
	
	struct process *root_proc = NULL;
	struct process *temp_proc = NULL;
	struct process *found_proc = NULL;
	struct process *last_proc = NULL;
	
	glibtop_init();
	
	list = glibtop_get_proclist(&proclist, 0, 0);
	
	if(proclist.number > 0)
	{
		glibtop_get_proc_state(&procstate, list[0]);
		glibtop_get_proc_uid(&procuid, list[0]);
		
		root_proc = process_new(list[0], procuid.ppid, procstate.cmd);
	}
	
	if(proclist.number > 1)
	{
		for(i = 1; i < proclist.number; i++)
		{
			glibtop_get_proc_state(&procstate, list[i]);
			glibtop_get_proc_uid(&procuid, list[i]);
			
			temp_proc = process_new(list[i], procuid.ppid, procstate.cmd);
			if(procuid.ppid == 0)
			{
				process_append(process_last(root_proc), temp_proc);
			}
			else
			{
				found_proc = process_search_pid(root_proc, procuid.ppid);
				if(found_proc != NULL)
				{
					if(process_count_children(found_proc) == 0)
					{
						process_append_child(found_proc, temp_proc);
					}
					else
					{
						process_append(process_last_child(found_proc), temp_proc);
					}
				}
			}
		}
	}
	
	g_free(list);
	
	process_print(root_proc, 0);
	
	process_free(root_proc);
	
	return 0;
}