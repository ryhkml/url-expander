#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "expand.h"

int main(int argc, char *argv[]) {
    init_curl();

    struct option some_options[] = {
        {"cookie",     required_argument, 0, 'c'},
        {"max-redirs", required_argument, 0, 'm'},
        {"user-agent", required_argument, 0, 'u'},
        {0,            0,                 0, 0  },
    };

    const char *short_url = NULL;
    const char *user_agent = DEFAULT_USER_AGENT;
    const char *cookie = NULL;
    long max_redirs = DEFAULT_MAX_REDIRS;

    int opt;
    while ((opt = getopt_long(argc, argv, "c:m:u:", some_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                cookie = optarg;
                break;
            case 'm':
                max_redirs = atoi(optarg);
                if (max_redirs <= 0) {
                    fprintf(stderr, "--max-redirs must be a positive\n");
                    cleanup_curl();
                    return EXIT_FAILURE;
                }
                break;
            case 'u':
                user_agent = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s <options?> <short_url>\n", argv[0]);
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

    expand(short_url, max_redirs, user_agent, cookie);
    cleanup_curl();

    return EXIT_SUCCESS;
}
