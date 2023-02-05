#include <stddef.h>
#include <stdlib.h>

#include "dproject.h"

int main(int argc, char *argv[])
{
  DProject *project = dproject_new(argv, argc);
  if (project == NULL) {
    return EXIT_FAILURE;
  }

  // If there are no arguments, or too many arguments, print help.
  if (argc > 3 || argc < 2) {
    dproject_print_help(project);
    return EXIT_FAILURE;
  }

  if (dproject_parse_args(project) == EXIT_FAILURE) {
    dproject_free(project);
    return EXIT_FAILURE;
  }

  dproject_free(project);
  return EXIT_SUCCESS;
} 
