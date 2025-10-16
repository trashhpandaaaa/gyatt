#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../gyatt.h"
#include "../utils.h"
#include "../buffer.h"

static int create_directory_structure(const char *base_path) {
    // Create main .gyatt directory
    if (mkdir_recursive(base_path) != 0) {
        fprintf(stderr, "Error: Failed to create .gyatt directory\n");
        return -1;
    }

    // Create objects directory
    char *objects_dir = path_join(base_path, "objects");
    if (mkdir_recursive(objects_dir) != 0) {
        fprintf(stderr, "Error: Failed to create objects directory\n");
        free(objects_dir);
        return -1;
    }
    free(objects_dir);

    // Create refs directory structure
    char *refs_dir = path_join(base_path, "refs");
    mkdir_recursive(refs_dir);
    free(refs_dir);

    char *refs_heads_dir = path_join(base_path, "refs/heads");
    mkdir_recursive(refs_heads_dir);
    free(refs_heads_dir);

    char *refs_remotes_dir = path_join(base_path, "refs/remotes");
    mkdir_recursive(refs_remotes_dir);
    free(refs_remotes_dir);

    return 0;
}

static int create_head_file(const char *base_path) {
    char *head_path = path_join(base_path, "HEAD");
    
    // Default HEAD points to main branch
    const char *head_content = "ref: refs/heads/main\n";
    
    int result = write_file(head_path, head_content, strlen(head_content));
    free(head_path);
    
    if (result != 0) {
        fprintf(stderr, "Error: Failed to create HEAD file\n");
        return -1;
    }
    
    return 0;
}

static int create_config_file(const char *base_path) {
    char *config_path = path_join(base_path, "config");
    
    // Default configuration
    buffer_t *buf = buffer_create(512);
    buffer_append_str(buf, "[core]\n");
    buffer_append_str(buf, "\tcompression = 6\n");
    buffer_append_str(buf, "\n");
    buffer_append_str(buf, "[user]\n");
    buffer_append_str(buf, "\tname = Your Name\n");
    buffer_append_str(buf, "\temail = you@example.com\n");
    
    int result = write_file(config_path, buffer_cstr(buf), buf->len);
    
    free(config_path);
    buffer_free(buf);
    
    if (result != 0) {
        fprintf(stderr, "Error: Failed to create config file\n");
        return -1;
    }
    
    return 0;
}

static int create_description_file(const char *base_path) {
    char *desc_path = path_join(base_path, "description");
    
    const char *description = "Gyatt repository\n";
    
    int result = write_file(desc_path, description, strlen(description));
    free(desc_path);
    
    if (result != 0) {
        fprintf(stderr, "Error: Failed to create description file\n");
        return -1;
    }
    
    return 0;
}

static int create_gyattignore(const char *repo_root) {
    char *ignore_path = path_join(repo_root, ".gyattignore");
    
    // Check if .gyattignore already exists
    if (file_exists(ignore_path)) {
        free(ignore_path);
        return 0;
    }
    
    // Default ignore patterns
    const char *ignore_content = 
        "# Gyatt internal files\n"
        ".gyatt/\n"
        "\n"
        "# Build artifacts\n"
        "*.o\n"
        "*.a\n"
        "*.so\n"
        "*.exe\n"
        "*.out\n"
        "\n"
        "# OS files\n"
        ".DS_Store\n"
        "Thumbs.db\n"
        "\n"
        "# Editor files\n"
        "*~\n"
        "*.swp\n"
        ".vscode/\n"
        ".idea/\n";
    
    int result = write_file(ignore_path, ignore_content, strlen(ignore_content));
    free(ignore_path);
    
    return result;
}

int cmd_init(int argc, char *argv[]) {
    (void)argc;  // Mark as intentionally unused
    (void)argv;  // Mark as intentionally unused
    
    char *cwd = get_current_dir();
    if (!cwd) {
        fprintf(stderr, "Error: Failed to get current directory\n");
        return 1;
    }
    
    // Check if already in a Gyatt repository
    if (is_gyatt_repo()) {
        fprintf(stderr, "Error: Already in a Gyatt repository\n");
        fprintf(stderr, "Repository location: %s\n", find_repo_root());
        free(cwd);
        return 1;
    }
    
    char *gyatt_dir = path_join(cwd, GYATT_DIR);
    
    printf("Initializing Gyatt repository in %s\n", cwd);
    
    // Create directory structure
    if (create_directory_structure(gyatt_dir) != 0) {
        free(gyatt_dir);
        free(cwd);
        return 1;
    }
    
    // Create HEAD file
    if (create_head_file(gyatt_dir) != 0) {
        free(gyatt_dir);
        free(cwd);
        return 1;
    }
    
    // Create config file
    if (create_config_file(gyatt_dir) != 0) {
        free(gyatt_dir);
        free(cwd);
        return 1;
    }
    
    // Create description file
    if (create_description_file(gyatt_dir) != 0) {
        free(gyatt_dir);
        free(cwd);
        return 1;
    }
    
    // Create .gyattignore
    create_gyattignore(cwd);
    
    free(gyatt_dir);
    
    printf("\n✓ Initialized empty Gyatt repository in %s/%s\n", cwd, GYATT_DIR);
    printf("\nNext steps:\n");
    printf("  1. Configure your identity:\n");
    printf("     Edit .gyatt/config to set your name and email\n");
    printf("  2. Add files:\n");
    printf("     gyatt add <files>\n");
    printf("  3. Create your first commit:\n");
    printf("     gyatt commit -m \"Initial commit\"\n");
    
    free(cwd);
    return 0;
}
