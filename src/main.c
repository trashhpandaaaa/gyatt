// Gyatt - Because reinventing the wheel is fun
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gyatt.h"

void print_usage(const char *prog_name) {
    printf("Gyatt - Like Git, but with personality\n\n");
    printf("Usage: %s <command> [options]\n\n", prog_name);
    printf("Commands:\n");
    printf("  init        Initialize a new Gyatt repository\n");
    printf("  add         Add files to staging area\n");
    printf("  commit      Record changes to the repository\n");
    printf("  status      Show working tree status\n");
    printf("  log         Show commit history\n");
    printf("  branch      List, create, or delete branches\n");
    printf("  checkout    Switch branches or restore files\n");
    printf("  push        Push changes to remote server\n");
    printf("  pull        Pull changes from remote server\n");
    printf("  server      Start Gyatt server mode\n");
    printf("  help        Show this help message\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *command = argv[1];

    // The world's longest if-else chain (TODO: use a hash map when we're feeling fancy)
    if (strcmp(command, "init") == 0) {
        return cmd_init(argc - 1, argv + 1);
    } else if (strcmp(command, "add") == 0) {
        return cmd_add(argc - 1, argv + 1);
    } else if (strcmp(command, "commit") == 0) {
        return cmd_commit(argc - 1, argv + 1);
    } else if (strcmp(command, "status") == 0) {
        return cmd_status(argc - 1, argv + 1);
    } else if (strcmp(command, "log") == 0) {
        return cmd_log(argc - 1, argv + 1);
    } else if (strcmp(command, "branch") == 0) {
        return cmd_branch(argc - 1, argv + 1);
    } else if (strcmp(command, "checkout") == 0) {
        return cmd_checkout(argc - 1, argv + 1);
    } else if (strcmp(command, "push") == 0) {
        return cmd_push(argc - 1, argv + 1);
    } else if (strcmp(command, "pull") == 0) {
        return cmd_pull(argc - 1, argv + 1);
    } else if (strcmp(command, "server") == 0) {
        return cmd_server(argc - 1, argv + 1);
    } else if (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n", command);
        fprintf(stderr, "Try 'gyatt help' if you're lost\n");
        return 1;
    }

    return 0; // We'll never get here but the compiler gets anxious without it
}
