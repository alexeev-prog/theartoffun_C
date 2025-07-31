/**
 * @file cmdparser.h
 * @author Alexeev Bronislav
 * @brief Advanced command line arguments parser
 * @version 1.0
 * @date 2025-07-19
 *
 * @copyright Copyright Alexeev Bronislav (c) 2025
 *
 * ██████ ███    ███ ██████      ██████   █████  ██████  ███████ ███████ ██████
 *██████ ██      ████  ████ ██   ██     ██   ██ ██   ██ ██   ██ ██      ██ ██ ██
 *██ ██      ██ ████ ██ ██   ██     ██████  ███████ ██████  ███████ █████ ██████
 *██ ██      ██  ██  ██ ██   ██     ██      ██   ██ ██   ██      ██ ██      ██
 *██     ██ ██████ ██      ██ ██████      ██      ██   ██ ██   ██ ███████
 *███████ ██   ██      ██████
 *
 */

#ifndef CMDPARSER_H
#define CMDPARSER_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Command Option configuration structure
 *
 * @param help Help description text
 * @param long_name Long option name (without "--")
 * @param short_name Short option character (without '-')
 * @param has_arg Flag indicating if option requires argument
 * @param default_value Default argument value (NULL if none)
 * @param handler Pointer to variable storing option result
 */
struct CommandOption {
  const char *help;
  const char *long_name;
  char short_name;
  int has_arg;
  const char *default_value;
  void *handler;
};

/**
 * @brief CLI Program Metadata container
 *
 * @param prog_name Program executable name
 * @param description Program description text
 * @param usage_args Usage arguments pattern
 * @param options Array of command options
 * @param options_count Number of command options
 */
struct CLIMetadata {
  const char *prog_name;
  const char *description;
  const char *usage_args;
  struct CommandOption *options;
  size_t options_count;
};

/**
 * @brief Prints formatted help message
 *
 * @param meta Pointer to program metadata
 */
void print_help(struct CLIMetadata *meta) {
  printf("%s\n", meta->description);
  printf("Usage: %s [OPTIONS] %s\n\n", meta->prog_name,
         meta->usage_args ? meta->usage_args : "");
  printf("Options:\n");

  for (size_t i = 0; i < meta->options_count; i++) {
    struct CommandOption *opt = &meta->options[i];
    char left_col[128] = {0};

    if (opt->short_name && opt->long_name) {
      snprintf(left_col, sizeof(left_col), "-%c, --%s", opt->short_name,
               opt->long_name);
    } else if (opt->short_name) {
      snprintf(left_col, sizeof(left_col), "-%c", opt->short_name);
    } else if (opt->long_name) {
      snprintf(left_col, sizeof(left_col), "--%s", opt->long_name);
    }

    if (opt->has_arg) {
      if (opt->long_name) {
        strncat(left_col, "=ARG", sizeof(left_col) - strlen(left_col) - 1);
      } else {
        strncat(left_col, " ARG", sizeof(left_col) - strlen(left_col) - 1);
      }
    }

    if (opt->default_value) {
      printf("  %-30s %s (default: %s)\n", left_col, opt->help ? opt->help : "",
             opt->default_value);
    } else {
      printf("  %-30s %s\n", left_col, opt->help ? opt->help : "");
    }
  }
  printf("\n");
}

/**
 * @brief Finds command option by short/long name
 *
 * @param options Options array
 * @param options_count Number of options
 * @param short_name Short name to search
 * @param long_name Long name to search
 * @return struct CommandOption* Found option or NULL
 */
struct CommandOption *find_option(struct CommandOption *options,
                                  size_t options_count, char short_name,
                                  const char *long_name) {
  for (size_t i = 0; i < options_count; ++i) {
    if (short_name && options[i].short_name == short_name) {
      return &options[i];
    }
    if (long_name && options[i].long_name &&
        strcmp(options[i].long_name, long_name) == 0) {
      return &options[i];
    }
  }
  return NULL;
}

/**
 * @brief Parses command line options
 *
 * @param argc Arguments count
 * @param argv Arguments array
 * @param options Command options configuration
 * @param options_count Number of options
 * @return int Index of first non-option argument or -1 on error
 */
int parse_options(int argc, char **argv, struct CommandOption *options,
                  size_t options_count) {
  int i = 1;

  while (i < argc) {
    const char *arg = argv[i];

    // End of options marker
    if (strcmp(arg, "--") == 0) {
      i++;
      break;
    }

    // Long option handling
    if (strncmp(arg, "--", 2) == 0) {
      const char *name = arg + 2;
      const char *value = strchr(name, '=');
      size_t name_len = value ? (size_t)(value - name) : strlen(name);

      // Validate long name length
      if (name_len == 0 || name_len > 63) {
        fprintf(stderr, "Invalid option: %s\n", arg);
        return -1;
      }

      char long_name[64];
      strncpy(long_name, name, name_len);
      long_name[name_len] = '\0';

      struct CommandOption *opt =
          find_option(options, options_count, '\0', long_name);
      if (!opt) {
        fprintf(stderr, "Unknown option: --%s\n", long_name);
        return -1;
      }

      if (opt->has_arg) {
        if (value) {
          *(const char **)opt->handler = value + 1;
        } else {
          if (i + 1 < argc) {
            *(const char **)opt->handler = argv[++i];
          } else if (opt->default_value) {
            *(const char **)opt->handler = opt->default_value;
          } else {
            fprintf(stderr, "Missing argument for: --%s\n", long_name);
            return -1;
          }
        }
      } else {
        if (value) {
          fprintf(stderr, "Unexpected argument for: --%s\n", long_name);
          return -1;
        }
        *(int *)opt->handler = 1;
      }
      i++;
      continue;
    }

    // Short option handling
    if (arg[0] == '-' && arg[1] != '\0') {
      const char *chars = arg + 1;

      while (*chars) {
        char c = *chars++;
        struct CommandOption *opt =
            find_option(options, options_count, c, NULL);

        if (!opt) {
          fprintf(stderr, "Unknown option: -%c\n", c);
          return -1;
        }

        if (opt->has_arg) {
          // Prevent '=' in short options
          if (*chars == '=') {
            fprintf(stderr, "Invalid '=' in short option -%c\n", c);
            return -1;
          }

          if (*chars != '\0') {
            *(const char **)opt->handler = chars;
            break; // Process next argument
          } else {
            if (i + 1 < argc) {
              *(const char **)opt->handler = argv[++i];
            } else if (opt->default_value) {
              *(const char **)opt->handler = opt->default_value;
            } else {
              fprintf(stderr, "Missing argument for: -%c\n", c);
              return -1;
            }
            break; // Process next argument
          }
        } else {
          *(int *)opt->handler = 1;
        }
      }
      i++;
      continue;
    }

    // Non-option argument
    break;
  }

  return i;
}

#endif // CMDPARSER_H
