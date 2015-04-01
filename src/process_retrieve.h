typedef struct
{
	pid_t pid;
	pid_t ppid;
	char *cmdline;
	char *cmd;
} process_t;

int ret_check_exists(pid_t pid);
process_t *ret_retrieve(pid_t pid);
void ret_free(process_t *process);
GArray *ret_walk(void);
gboolean ret_check_directory(gchar *directory);