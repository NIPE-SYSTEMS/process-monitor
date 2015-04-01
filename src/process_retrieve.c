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
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>

#include "process_retrieve.h"

/**
 * Returns if a process with given pid exists.
 * @param pid the pid of the process which should be checked
 * @return TRUE if process exists, FALSE if not
 */
int ret_check_exists(pid_t pid)
{
	gboolean state = FALSE;
	gchar *path = g_strdup_printf("/proc/%i", pid);
	
	state = g_file_test(path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR);
	
	g_free(path);
	
	return state;
}

/**
 * Retrieves all informations for a given process.
 * @param pid
 * @return a structure which contains all informations
 */
process_t *ret_retrieve(pid_t pid)
{
	process_t *process = NULL;
	
	gchar *path_stat = NULL;
	gchar *contents_stat = NULL;
	gsize contents_stat_size = 0;
	GError *error = NULL;
	gchar *path_cmdline = NULL;
	gchar *contents_cmdline = NULL;
	gsize contents_cmdline_size = 0;
	gchar *working_ptr0 = NULL;
	gchar *working_ptr1 = NULL;
	gchar *working_ptr2 = NULL;
	gchar *working_ptr3 = NULL;
	
	if(!ret_check_exists(pid))
	{
		return NULL;
	}
	
	// /proc/$pid/stat
	path_stat = g_strdup_printf("/proc/%i/stat", pid);
	g_file_get_contents(path_stat, &contents_stat, &contents_stat_size, &error);
	
	if(error != NULL)
	{
		fprintf(stderr, "Failed to read file: %s\n", error->message);
		g_error_free(error);
	}
	
	// printf("%s\n", contents_stat);
	
	process = malloc(sizeof(process_t));
	
	if(process != NULL && contents_stat != NULL)
	{
		working_ptr0 = strchr(contents_stat, ' ');
		*working_ptr0 = '\0';
		working_ptr1 = strchr(working_ptr0 + 2, ')');
		*working_ptr1 = '\0';
		working_ptr2 = strchr(working_ptr1 + 2, ' ');
		working_ptr3 = strchr(working_ptr2 + 1, ' ');
		*working_ptr3 = '\0';
		
		process->pid = (pid_t)strtol(contents_stat, NULL, 10);
		process->ppid = (pid_t)strtol(working_ptr2 + 1, NULL, 10);
		process->cmd = g_strdup(working_ptr0 + 2);
	}
	
	g_free(contents_stat);
	g_free(path_stat);
	
	// /proc/$pid/cmdline
	path_cmdline = g_strdup_printf("/proc/%i/cmdline", pid);
	g_file_get_contents(path_cmdline, &contents_cmdline, &contents_cmdline_size, &error);
	
	if(error != NULL)
	{
		fprintf(stderr, "Failed to read file: %s\n", error->message);
		g_error_free(error);
	}
	
	// printf("%s\n", contents_cmdline);
	
	process->cmdline = contents_cmdline;
	
	g_free(path_cmdline);
	
	return process;
}

/**
 * Cleans up memory used by process structure.
 * @param process the process to be cleaned up
 */
void ret_free(process_t *process)
{
	g_free(process->cmd);
	g_free(process->cmdline);
	g_free(process);
}

/**
 * Loops through all directories
 */
GArray *ret_walk(void)
{
	GFileEnumerator *enumerator = NULL;
	GError *error = NULL;
	GFileInfo *info = NULL;
	GFile *path = g_file_new_for_path("/proc");
	GArray *processes = g_array_new(FALSE, TRUE, sizeof(pid_t));
	pid_t generated_pid = 0;
	
	enumerator = g_file_enumerate_children(path, "*", G_FILE_QUERY_INFO_NONE, NULL, &error);
	
	if(error != NULL)
	{
		fprintf(stderr, "Failed to initialize enumerator: %s\n", error->message);
		g_error_free(error);
	}
	else
	{
		while((info = g_file_enumerator_next_file(enumerator, NULL, &error)) != NULL)
		{
			if(error != NULL)
			{
				fprintf(stderr, "Failed to get next file: %s\n", error->message);
				g_error_free(error);
			}
			
			if(g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY && ret_check_directory((gchar *)g_file_info_get_name(info)))
			{
				generated_pid = (pid_t)strtol((gchar *)g_file_info_get_name(info), NULL, 10);
				processes = g_array_append_val(processes, generated_pid);
			}
			
			g_object_unref(info);
		}
		
		g_file_enumerator_close(enumerator, NULL, &error);
	}
	
	g_object_unref(path);
	
	return processes;
}

/**
 * Returns if a directory name is numerical.
 * @param directory the directory name
 * @return TRUE if directory contains only numbers, FALSE if not
 */
gboolean ret_check_directory(gchar *directory)
{
	size_t i = 0;
	
	if(directory == NULL)
	{
		return FALSE;
	}
	
	for(i = 0; i < strlen(directory); i++)
	{
		if(!(directory[i] == '0' || directory[i] == '1' || directory[i] == '2' || directory[i] == '3' || directory[i] == '4' || directory[i] == '5' || directory[i] == '6' || directory[i] == '7' || directory[i] == '8' || directory[i] == '9'))
		{
			return FALSE;
		}
	}
	
	return TRUE;
}