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
#include <string.h>
#include <sys/types.h>
#include <glibtop.h>
#include <glibtop/proclist.h>
#include <glibtop/procstate.h>
#include <glibtop/procuid.h>
#include <glibtop/procargs.h>

struct process
{
	pid_t pid;
	pid_t ppid;
	char *command;
	struct process *next;
	struct process *child;
};
typedef struct process process_t;

typedef enum { TERMINATE } process_monitor_t;
typedef enum { DO_NOTHING, RESTART } process_monitor_action_t;

typedef struct
{
	pid_t pid;
	char *command;
	process_monitor_t monitor;
	process_monitor_action_t action;
	char *args;
} process_monitor_job_t;

static process_t *process_root = NULL;
static process_monitor_job_t *process_monitor_job_list = NULL;
static int process_monitor_job_list_amount = 0;

process_t *process_new(pid_t pid, pid_t ppid, char *command)
{
	process_t *proc = malloc(sizeof(process_t));
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

void process_free(process_t *proc)
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
	
	proc = NULL;
}

void process_print(process_t *proc, int shift)
{
	int i = 0;
	
	if(proc == NULL)
	{
		return;
	}
	
	for(i = 0; i < shift; i++)
	{
		printf(". ");
	}
	
	printf("%s (%i)\n", proc->command, proc->pid);
	
	if(proc->child != NULL)
	{
		process_print(proc->child, shift + 1);
	}
	
	if(proc->next != NULL)
	{
		process_print(proc->next, shift);
	}
}

void process_append(process_t *proc0, process_t *proc1)
{
	if(proc0 == NULL || proc0->next != NULL || proc1 == NULL)
	{
		return;
	}
	
	proc0->next = proc1;
}

void process_append_child(process_t *parent, process_t *child)
{
	if(parent == NULL || parent->child != NULL || child == NULL)
	{
		return;
	}
	
	parent->child = child;
}

process_t *process_last(process_t *first)
{
	process_t *pointer = first;
	
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

process_t *process_last_child(process_t *first_child)
{
	if(first_child == NULL || first_child->child == NULL)
	{
		return first_child;
	}
	
	return process_last(first_child->child);
}

int process_count(process_t *proc)
{
	int i = 0;
	process_t *pointer = proc;
	
	while(pointer->next != NULL)
	{
		i++;
		pointer = pointer->next;
	}
	
	return i;
}

int process_count_children(process_t *proc)
{
	if(proc == NULL || proc->child == NULL)
	{
		return 0;
	}
	
	return process_count(proc->child) + 1;
}

process_t *process_search_pid(process_t *proc, pid_t pid)
{
	process_t *found_proc = NULL;
	
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

void process_retrieve(void)
{
	glibtop_proclist proclist;
	glibtop_proc_state procstate;
	glibtop_proc_uid procuid;
	pid_t *list = NULL;
	unsigned long int i = 0;
	
	process_t *temp_proc = NULL;
	process_t *found_proc = NULL;
	
	glibtop_init();
	
	list = glibtop_get_proclist(&proclist, 0, 0);
	
	if(proclist.number > 0)
	{
		glibtop_get_proc_state(&procstate, list[0]);
		glibtop_get_proc_uid(&procuid, list[0]);
		
		process_root = process_new(list[0], procuid.ppid, procstate.cmd);
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
				process_append(process_last(process_root), temp_proc);
			}
			else
			{
				found_proc = process_search_pid(process_root, procuid.ppid);
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
	
	process_print(process_root, 0);
}

void process_monitor_job_add(pid_t pid, process_monitor_t monitor, process_monitor_action_t action)
{
	glibtop_proc_state procstate;
	glibtop_proc_uid procuid;
	glibtop_proc_args procargs;
	process_monitor_job_t *job_list_backup = process_monitor_job_list;
	char *args = NULL;
	unsigned long int i = 0;
	
	if(process_monitor_job_list == NULL)
	{
		process_monitor_job_list = malloc(sizeof(process_monitor_job_t));
		
		if(process_monitor_job_list == NULL)
		{
			return;
		}
		
		process_monitor_job_list_amount++;
	}
	else
	{
		process_monitor_job_list = realloc(process_monitor_job_list, ++process_monitor_job_list_amount * sizeof(process_monitor_job_t));
		
		if(process_monitor_job_list == NULL)
		{
			process_monitor_job_list_amount--;
			process_monitor_job_list = job_list_backup;
			
			return;
		}
	}
	
	glibtop_get_proc_state(&procstate, pid);
	glibtop_get_proc_uid(&procuid, pid);
	args = glibtop_get_proc_args(&procargs, pid, 0);
	
	// concatenate args parts together
	if(args != NULL && procargs.size > 0)
	{
		for(i = 0; i < procargs.size - 1; i++)
		{
			if(args[i] == '\0')
			{
				args[i] = ' ';
			}
		}
	}
	
	process_monitor_job_list[process_monitor_job_list_amount - 1].pid = pid;
	
	if(procstate.cmd == NULL)
	{
		process_monitor_job_list[process_monitor_job_list_amount - 1].command = NULL;
	}
	else
	{
		process_monitor_job_list[process_monitor_job_list_amount - 1].command = strdup(procstate.cmd);
	}
	
	process_monitor_job_list[process_monitor_job_list_amount - 1].monitor = monitor;
	process_monitor_job_list[process_monitor_job_list_amount - 1].action = action;
	process_monitor_job_list[process_monitor_job_list_amount - 1].args = args;
}

void process_monitor_job_print(void)
{
	int i = 0;
	
	for(i = 0; i < process_monitor_job_list_amount; i++)
	{
		printf("%s\n", process_monitor_job_list[i].command);
	}
}

void process_monitor_job_cleanup(void)
{
	int i = 0;
	
	for(i = 0; i < process_monitor_job_list_amount; i++)
	{
		free(process_monitor_job_list[i].command);
		free(process_monitor_job_list[i].args);
	}
	
	free(process_monitor_job_list);
}

void process_monitor_job_handle(process_monitor_job_t *job)
{
	printf("Unexpected event: %s\n", job->command);
}

void process_monitor_job_compare(void)
{
	process_t *found_proc = NULL;
	int i = 0;
	
	for(i = 0; i < process_monitor_job_list_amount; i++)
	{
		switch(process_monitor_job_list[i].monitor)
		{
			case TERMINATE:
			{
				printf("TERMINATE: %s (%i)\n", process_monitor_job_list[i].command, process_monitor_job_list[i].pid);
				
				found_proc = process_search_pid(process_root, process_monitor_job_list[i].pid);
				if(found_proc == NULL)
				{
					process_monitor_job_handle(&(process_monitor_job_list[i]));
					
					break;
				}
				
				if(strcmp(found_proc->command, process_monitor_job_list[i].command) != 0)
				{
					process_monitor_job_handle(&(process_monitor_job_list[i]));
					
					break;
				}
				
				printf("Process exists. Everything is fine!\n");
				
				break;
			}
			default:
			{
				printf("Unknown compare method.\n");
				break;
			}
		}
	}
}

int main(void)
{
	process_retrieve();
	
	process_monitor_job_add(1, TERMINATE, DO_NOTHING);
	process_monitor_job_add(1180, TERMINATE, DO_NOTHING);
	process_monitor_job_add(9999, TERMINATE, RESTART);
	
	process_monitor_job_print();
	process_monitor_job_compare();
	
	process_monitor_job_cleanup();
	
	process_free(process_root);
	
	return 0;
}