#include "dproject.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 1048576

DProject *dproject_new(char *argv[], int argc)
{
    DProject *project = malloc(sizeof(*project));
    if (project == NULL) {
      fprintf(stderr, "[-] Failed to allocate memory for project.\n");
      return NULL;
    }

    project->argv = argv;
    project->argc = argc;
    return project;
}

void dproject_free(DProject *project)
{
    free(project);
}

int dproject_parse_args(DProject *project)
{
  size_t opt_index;

  for (opt_index = 1; opt_index < project->argc && project->argv[opt_index][0] == '-'; opt_index++) {
    switch (project->argv[opt_index][1]) {
      case HELP:
        dproject_print_help(project);
        break;
      case VERSION:
        dproject_print_version(project);
        break;
      case FULL_OPT:
        if (strcmp(project->argv[opt_index], "--help") == 0) {
          dproject_print_help(project);
        } else if (strcmp(project->argv[opt_index], "--version") == 0) {
          dproject_print_version(project);
        } else {
          fprintf(stderr, "Unknown option: %s\n", project->argv[opt_index]);
          return EXIT_FAILURE;
        }
        break;
      default:
        fprintf(stderr, "Unknown option: %s\n", project->argv[opt_index]);
        break;
    }
  }

  if (project->argc == 2) return EXIT_SUCCESS;

  // Set the project name.
  project->project_name = project->argv[1];

  int error;
  char *action = project->argv[2];

  if (strcmp(project->project_name, "dproject") == 0) {
    dproject_print_help(project);
    return EXIT_SUCCESS;
  } else if (strcmp(project->argv[2], DPROJECT_ACTIONS[DPROJECT_ACTION_START]) == 0) 
    error = dproject_start(project);
  else if (strcmp(project->argv[2], DPROJECT_ACTIONS[DPROJECT_ACTION_STOP]) == 0)
    error = dproject_stop(project);
  else if (strcmp(project->argv[2], DPROJECT_ACTIONS[DPROJECT_ACTION_RESTART]) == 0)
    error = dproject_restart(project);
  else if (strcmp(project->argv[2], DPROJECT_ACTIONS[DPROJECT_ACTION_STATUS]) == 0)
    error = dproject_status(project);
  else {
    return EXIT_SUCCESS;
  }

  if (error != EXIT_SUCCESS) {
    fprintf(stderr, "[-] Failed to %s project %s\n", action, project->project_name);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void dproject_print_help(DProject *project)
{
  printf("Usage: %s <project_name> [action]\n", project->argv[0]);
  printf("Usage: %s [options]\n", project->argv[0]);
  printf("Options:\n");
  printf("  -h\t\tShow this help message.\n");
  printf("  -v\t\tShow version information.\n");
  printf("Actions:\n");
  printf("  start\t\tStart the project.\n");
  printf("  stop\t\tStop the project.\n");
  printf("  restart\tRestart the project.\n");
  printf("  status\tShow the status of the project.\n");
}

void dproject_print_version(DProject *project)
{
  printf("dproject version %s\n", DPROJECT_VERSION);
}

int dproject_exec_with_output(DProject *project, char *command, char *buffer)
{
  FILE *fp;
  size_t bytes_read;

  fp = popen(command, "r");
  if (fp == NULL) {
    fprintf(stderr, "[-] Failed to run command: %s\n", command);
    free(buffer);
    return EXIT_FAILURE;
  }

  bytes_read = fread(buffer, 1, BUFFER_SIZE, fp); 

  if (bytes_read == 0) {
    fprintf(stderr, "[-] Failed to read command output: %s\n", command);
    free(buffer);
    return -1;
  } else if (bytes_read == BUFFER_SIZE) {
    fprintf(stderr, "[-] Command output too large: %s\n", command);
    free(buffer);
    return -2;
  } else {
    buffer[bytes_read] = '\0';
  }

  pclose(fp);
  return EXIT_SUCCESS;
}

int dproject_exec(DProject *project, char *command)
{
  FILE *fp;

  fp = popen(command, "r");
  if (fp == NULL) {
    fprintf(stderr, "[-] Failed to run command: %s\n", command);
    return EXIT_FAILURE;
  }

  pclose(fp);

  /*int error = system(command);
  if (error != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }*/

  return EXIT_SUCCESS;
}

int dproject_get_container_id(DProject *project, char *buffer)
{
  char command[256];
  sprintf(command, "sudo docker ps -q -f status=exited -f name=%s", project->project_name);
  return dproject_exec_with_output(project, command, buffer);
}

int dproject_start(DProject *project)
{
  printf("\033[2J\033[1;1H\n");
  printf("[+] Checking if project is already running...\n");
  
  char *buffer = (char *)malloc(BUFFER_SIZE);
  if (buffer == NULL) {
    fprintf(stderr, "[-] Failed to allocate memory for buffer.\n");
    return EXIT_FAILURE;
  }

  int error = dproject_get_container_id(project, buffer);
  if (error == EXIT_FAILURE) {
    fprintf(stderr, "[-] Failed to get container(s) for project: %s\n", project->project_name);
    free(buffer);
    return EXIT_FAILURE;
  } else if (error == -1) {
    fprintf(stderr, "[-] Container(s) already running.\n");
    free(buffer);
    return EXIT_SUCCESS;
  } else if (error == -2) {
    fprintf(stderr, "[-] Too many containers.\n");
    free(buffer);
    return EXIT_FAILURE;
  }

  printf("[+] Starting project...\n");
  char container_ids[100][100];
  int container_id_count = 0;

  char *line = strtok(buffer, "\n");
  while (line != NULL) {
    strcpy(container_ids[container_id_count++], line);
    line = strtok(NULL, "\n");
  }

  printf("[+] Discovered %d container(s) for project: %s\n", container_id_count, project->project_name);
  
  for (size_t i = 0; i < container_id_count; i++) {
    printf("[%zu/%d] Starting container: %s\n", i + 1, container_id_count, container_ids[i]);
    char command[256];
    sprintf(command, "sudo docker start %s", container_ids[i]);
    if (dproject_exec(project, command) == EXIT_FAILURE) {
      fprintf(stderr, "[-] Failed to start container: %s\n", container_ids[i]);
      free(buffer);
      return EXIT_FAILURE;
    } else {
      printf(" Done.\n");
    }
  }

  printf("[+] Started project %s\n", project->project_name);

  return EXIT_SUCCESS;
}

int dproject_stop(DProject *project)
{
  printf("\033[2J\033[1;1H\n");
  printf("[+] Checking if project is already exited...\n");
  
  char *buffer = (char *)malloc(BUFFER_SIZE);
  if (buffer == NULL) {
    fprintf(stderr, "[-] Failed to allocate memory for buffer.\n");
    return EXIT_FAILURE;
  }

  char command[256];
  sprintf(command, "sudo docker ps -q -f status=running -f name=%s", project->project_name);
  int error = dproject_exec_with_output(project, command, buffer);
  if (error == EXIT_FAILURE) {
    fprintf(stderr, "[-] Failed to get container(s) for project: %s\n", project->project_name);
    free(buffer);
    return EXIT_FAILURE;
  } else if (error == -1) {
    fprintf(stderr, "[-] Container(s) already exited.\n");
    free(buffer);
    return EXIT_SUCCESS;
  } else if (error == -2) {
    fprintf(stderr, "[-] Too many containers.\n");
    free(buffer);
    return EXIT_FAILURE;
  }

  printf("[+] Stoping project...\n");
  char container_ids[100][100];
  int container_id_count = 0;

  char *line = strtok(buffer, "\n");
  while (line != NULL) {
    strcpy(container_ids[container_id_count++], line);
    line = strtok(NULL, "\n");
  }

  printf("[+] Discovered %d container(s) for project: %s\n", container_id_count, project->project_name);
  
  for (size_t i = 0; i < container_id_count; i++) {
    printf("[%zu/%d] Stoping container: %s\n", i + 1, container_id_count, container_ids[i]);
    char command[256];
    sprintf(command, "sudo docker stop %s", container_ids[i]);
    if (dproject_exec(project, command) == EXIT_FAILURE) {
      fprintf(stderr, "[-] Failed to stop container: %s\n", container_ids[i]);
      free(buffer);
      return EXIT_FAILURE;
    } else {
      printf(" Done.\n");
    }
  }

  printf("[+] Stoped project %s\n", project->project_name);

  return EXIT_SUCCESS;
}

int dproject_restart(DProject *project)
{
  printf("\033[2J\033[1;1H\n");
  printf("[+] Checking if project is already running...\n");
  
  char *buffer = (char *)malloc(BUFFER_SIZE);
  if (buffer == NULL) {
    fprintf(stderr, "[-] Failed to allocate memory for buffer.\n");
    return EXIT_FAILURE;
  }

  char command[256];
  sprintf(command, "sudo docker ps -q -f name=%s", project->project_name);
  int error = dproject_exec_with_output(project, command, buffer);
  if (error == EXIT_FAILURE) {
    fprintf(stderr, "[-] Failed to get container(s) for project: %s\n", project->project_name);
    free(buffer);
    return EXIT_FAILURE;
  } else if (error == -1) {
    fprintf(stderr, "[-] Container(s) already exited.\n");
    free(buffer);
    return EXIT_SUCCESS;
  } else if (error == -2) {
    fprintf(stderr, "[-] Too many containers.\n");
    free(buffer);
    return EXIT_FAILURE;
  }

  printf("[+] Restarting project...\n");
  char container_ids[100][100];
  int container_id_count = 0;

  char *line = strtok(buffer, "\n");
  while (line != NULL) {
    strcpy(container_ids[container_id_count++], line);
    line = strtok(NULL, "\n");
  }

  printf("[+] Discovered %d container(s) for project: %s\n", container_id_count, project->project_name);
  
  for (size_t i = 0; i < container_id_count; i++) {
    printf("[%zu/%d] Restarting container: %s\n", i + 1, container_id_count, container_ids[i]);
    char command[256];
    sprintf(command, "sudo docker restart %s", container_ids[i]);
    if (dproject_exec(project, command) == EXIT_FAILURE) {
      fprintf(stderr, "[-] Failed to restart container: %s\n", container_ids[i]);
      free(buffer);
      return EXIT_FAILURE;
    } else {
      printf(" Done.\n");
    }
  }

  printf("[+] Restarted project %s\n", project->project_name);
  return EXIT_SUCCESS;
}

int dproject_status(DProject *project)
{
  printf("\033[2J\033[1;1H\n");
  printf("[+] Checking if project is already running...\n");
  
  char *buffer = (char *)malloc(BUFFER_SIZE);
  if (buffer == NULL) {
    fprintf(stderr, "[-] Failed to allocate memory for buffer.\n");
    return EXIT_FAILURE;
  }

  char command[1024];
  sprintf(command, "sudo docker ps -aq -f name=%s", project->project_name);
  int error = dproject_exec_with_output(project, command, buffer);
  if (error == EXIT_FAILURE) {
    fprintf(stderr, "[-] Failed to get container(s) for project: %s\n", project->project_name);
    free(buffer);
    return EXIT_FAILURE;
  } else if (error == -1) {
    fprintf(stderr, "[-] Container(s) already exited.\n");
    free(buffer);
    return EXIT_SUCCESS;
  } else if (error == -2) {
    fprintf(stderr, "[-] Too many containers.\n");
    free(buffer);
    return EXIT_FAILURE;
  }

  char container_ids[100][100];
  int container_id_count = 0;

  char *line = strtok(buffer, "\n");
  while (line != NULL) {
    strcpy(container_ids[container_id_count++], line);
    line = strtok(NULL, "\n");
  }

  printf("[+] Discovered %d container(s) for project: %s\n", container_id_count, project->project_name);
  
  /*char ids[] = "";
  for (size_t i = 0; i < container_id_count; i++) {
    sprintf(ids, "%s %s", ids, container_ids[i]);
  }

  sprintf(command, "sudo docker stats%s", ids);
  printf("Press Ctrl+C to exit.\n");
  printf("command: %s\n", command);
  while (dproject_exec_with_output(project, command, buffer) != EXIT_FAILURE)
  {
    printf("\033[2J\033[1;1H");
    printf(buffer);
  }*/

  return EXIT_SUCCESS;
}
