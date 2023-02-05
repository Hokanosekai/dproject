# ifndef DP_PROJECT_H
# define DP_PROJECT_H

#define DPROJECT_VERSION "0.0.1"

static char *DPROJECT_ACTIONS[] = {
  "start",
  "stop",
  "restart",
  "status",
};

typedef enum {
  DPROJECT_ACTION_START,
  DPROJECT_ACTION_STOP,
  DPROJECT_ACTION_RESTART,
  DPROJECT_ACTION_STATUS,
} dproject_action_t;

typedef enum {
  HELP = 'h',
  VERSION = 'v',
  FULL_OPT = '-',
} dproject_opt_t;

typedef struct {
  int argc;
  char *project_name;
  char **argv;
  char **container_ids;
} DProject;

DProject *dproject_new(char *argv[], int argc);

void dproject_free(DProject *project);

int dproject_parse_args(DProject *project); 

void dproject_print_help(DProject *project);

void dproject_print_version(DProject *project);

int dproject_start(DProject *project);

int dproject_stop(DProject *project);

int dproject_restart(DProject *project);

int dproject_status(DProject *project);

int dproject_exec_with_output(DProject *project, char *command, char *buffer);

int dproject_exec(DProject *project, char *command);

int dproject_get_container_id(DProject *project, char *buffer);

# endif // DP_PROJECT_H
