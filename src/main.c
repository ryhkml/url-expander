#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "expand.h"

static void print_help() {
    printf("\n");
    printf("%sUnshorten URLs into their original links%s\n", GREEN, RESET);
    printf("\n");
    printf("Usage   : expand-url <options?> <short_url>\n");
    printf("Options :\n");
    printf("  -c, --cookie <string>      Set cookie path from file\n");
    printf("  -M, --max-redirs <uint>    Set maximum redirects\n");
    printf("  -U, --user-agent <string>  Set user-agent\n");
    printf("\n");
    printf("  -h, --help                 Display help message\n");
    printf("      --verbose              Enable verbose mode\n");
    printf("\n");
}

int main(int argc, char *argv[]) {
    init_curl();

    struct option some_options[] = {
        {"cookie",     required_argument, NULL, 'c'},
        {"max-redirs", required_argument, NULL, 'M'},
        {"user-agent", required_argument, NULL, 'U'},
        {"help",       no_argument,       NULL, 'h'},
        {"verbose",    no_argument,       NULL, 0  },
        {0,            0,                 0,    0  }
    };

    const char *short_url = NULL;
    struct url_expander_opt opt_v = {
        .cookie = NULL,
        .max_redirs = DEFAULT_MAX_REDIRS,
        .user_agent = DEFAULT_USER_AGENT,
        .verbose = false,
    };

    int opt;
    int opt_index = 0;
    while ((opt = getopt_long(argc, argv, "c:M:U:h", some_options, &opt_index)) != -1) {
        switch (opt) {
            case 'c':
                opt_v.cookie = optarg;
                break;
            case 'M':
                opt_v.max_redirs = (long)atoi(optarg);
                if (opt_v.max_redirs <= 0) {
                    fprintf(stderr, "--max-redirs must be a positive\n");
                    cleanup_curl();
                    return EXIT_FAILURE;
                }
                break;
            case 'U':
                opt_v.user_agent = optarg;
                break;
            case 'h':
                print_help();
                cleanup_curl();
                return EXIT_SUCCESS;
            case 0:
                if (opt_index == 4) opt_v.verbose = true;
                break;
            default:
                printf("Invalid option. Use -h or --help to display help message\n");
                cleanup_curl();
                return EXIT_FAILURE;
        }
    }

    if (optind < argc) short_url = argv[optind];
    if (!short_url) {
        fprintf(stderr, "Usage: %s <options?> <short_url>\n", argv[0]);
        cleanup_curl();
        return EXIT_FAILURE;
    }

    expand(short_url, &opt_v);
    cleanup_curl();

    return EXIT_SUCCESS;
}
