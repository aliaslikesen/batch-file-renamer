#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#define INPUT_SIZE 1024

typedef struct {
    char *original_name;
    char *temp_name;
    char *final_name;
} FileItem;

static char *copy_string(const char *text) {
    char *result = malloc(strlen(text) + 1);
    if (!result) {
        return NULL;
    }

    strcpy(result, text);
    return result;
}

static void remove_newline(char *text) {
    size_t len = strlen(text);

    if (len > 0 && text[len - 1] == '\n') {
        text[len - 1] = '\0';
    }

    len = strlen(text);

    if (len > 0 && text[len - 1] == '\r') {
        text[len - 1] = '\0';
    }
}

static void read_line(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    fflush(stdout);

    if (!fgets(buffer, (int)size, stdin)) {
        fprintf(stderr, "Blad odczytu danych.\n");
        exit(1);
    }

    remove_newline(buffer);
}

static int is_base_name_invalid(const char *name) {
    if (name[0] == '\0') {
        return 1;
    }

    for (size_t i = 0; name[i] != '\0'; i++) {
        char c = name[i];

        if (c == '/' || c == '\\') {
            return 1;
        }

#ifdef _WIN32
        if (c == '<' || c == '>' || c == ':' || c == '"' ||
            c == '|' || c == '?' || c == '*') {
            return 1;
        }
#endif
    }

    return 0;
}

static char path_separator(void) {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

static char *join_path(const char *folder, const char *file_name) {
    size_t folder_len = strlen(folder);
    size_t file_len = strlen(file_name);

    int needs_separator = 1;

    if (folder_len > 0) {
        char last = folder[folder_len - 1];

        if (last == '/' || last == '\\') {
            needs_separator = 0;
        }
    }

    char *result = malloc(folder_len + file_len + needs_separator + 1);
    if (!result) {
        return NULL;
    }

    strcpy(result, folder);

    if (needs_separator) {
        result[folder_len] = path_separator();
        result[folder_len + 1] = '\0';
    }

    strcat(result, file_name);

    return result;
}

static int path_exists(const char *path) {
#ifdef _WIN32
    DWORD attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES;
#else
    return access(path, F_OK) == 0;
#endif
}

static int add_file(FileItem **items, size_t *count, const char *file_name) {
    FileItem *new_items = realloc(*items, (*count + 1) * sizeof(FileItem));
    if (!new_items) {
        return 0;
    }

    *items = new_items;

    (*items)[*count].original_name = copy_string(file_name);
    (*items)[*count].temp_name = NULL;
    (*items)[*count].final_name = NULL;

    if (!(*items)[*count].original_name) {
        return 0;
    }

    (*count)++;

    return 1;
}

static int compare_files(const void *a, const void *b) {
    const FileItem *file_a = (const FileItem *)a;
    const FileItem *file_b = (const FileItem *)b;

    return strcmp(file_a->original_name, file_b->original_name);
}

static int collect_files(const char *folder, FileItem **items, size_t *count) {
#ifdef _WIN32
    WIN32_FIND_DATAA data;
    char *pattern = join_path(folder, "*");

    if (!pattern) {
        return 0;
    }

    HANDLE handle = FindFirstFileA(pattern, &data);
    free(pattern);

    if (handle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0) {
            continue;
        }

        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        if (!add_file(items, count, data.cFileName)) {
            FindClose(handle);
            return 0;
        }

    } while (FindNextFileA(handle, &data));

    FindClose(handle);
    return 1;
#else
    DIR *dir = opendir(folder);

    if (!dir) {
        return 0;
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char *full_path = join_path(folder, entry->d_name);
        if (!full_path) {
            closedir(dir);
            return 0;
        }

        struct stat info;
        int is_regular = 0;

        if (stat(full_path, &info) == 0 && S_ISREG(info.st_mode)) {
            is_regular = 1;
        }

        free(full_path);

        if (!is_regular) {
            continue;
        }

        if (!add_file(items, count, entry->d_name)) {
            closedir(dir);
            return 0;
        }
    }

    closedir(dir);
    return 1;
#endif
}

static const char *get_extension(const char *file_name) {
    const char *dot = strrchr(file_name, '.');

    if (!dot) {
        return "";
    }

    if (dot == file_name) {
        return "";
    }

    if (*(dot + 1) == '\0') {
        return "";
    }

    return dot;
}

static char *create_final_name(
    const char *base_name,
    const char *separator,
    size_t index,
    const char *extension
) {
    char number[64];
    snprintf(number, sizeof(number), "%zu", index);

    size_t len =
        strlen(base_name) +
        strlen(separator) +
        strlen(number) +
        strlen(extension) +
        1;

    char *result = malloc(len);
    if (!result) {
        return NULL;
    }

    snprintf(result, len, "%s%s%s%s", base_name, separator, number, extension);
    return result;
}

static char *create_temp_name(const char *folder, size_t index) {
    char buffer[256];

#ifdef _WIN32
    unsigned long pid = (unsigned long)GetCurrentProcessId();
#else
    unsigned long pid = (unsigned long)getpid();
#endif

    for (int attempt = 0; attempt < 10000; attempt++) {
        snprintf(
            buffer,
            sizeof(buffer),
            ".rename_tmp_%ld_%lu_%zu_%d.tmp",
            (long)time(NULL),
            pid,
            index,
            attempt
        );

        char *full_path = join_path(folder, buffer);
        if (!full_path) {
            return NULL;
        }

        int exists = path_exists(full_path);
        free(full_path);

        if (!exists) {
            return copy_string(buffer);
        }
    }

    return NULL;
}

static void rollback_temp_names(const char *folder, FileItem *items, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (!items[i].temp_name) {
            continue;
        }

        char *temp_path = join_path(folder, items[i].temp_name);
        char *original_path = join_path(folder, items[i].original_name);

        if (temp_path && original_path && path_exists(temp_path)) {
            rename(temp_path, original_path);
        }

        free(temp_path);
        free(original_path);
    }
}

static void free_items(FileItem *items, size_t count) {
    for (size_t i = 0; i < count; i++) {
        free(items[i].original_name);
        free(items[i].temp_name);
        free(items[i].final_name);
    }

    free(items);
}

static int ask_continue(void) {
    char answer[32];

    read_line("\nCzy chcesz dalej zamieniac nazwy plikow? (t/n): ", answer, sizeof(answer));

    if (answer[0] == 't' || answer[0] == 'T' ||
        answer[0] == 'y' || answer[0] == 'Y') {
        return 1;
    }

    return 0;
}

static int run_rename_once(void) {
    char folder[INPUT_SIZE];
    char base_name[INPUT_SIZE];
    char option_text[32];

    read_line("Podaj folder, w ktorym program ma dzialac: ", folder, sizeof(folder));
    read_line("Podaj nowa nazwe bazowa plikow, np. PlikiMiniaturki: ", base_name, sizeof(base_name));

    if (is_base_name_invalid(base_name)) {
        fprintf(stderr, "Nieprawidlowa nazwa bazowa pliku.\n");
        return 1;
    }

    printf("\nWybierz konwencje numerowania:\n");
    printf("1. %s1\n", base_name);
    printf("2. %s-1\n", base_name);
    printf("3. %s_1\n", base_name);

    read_line("Twoj wybor: ", option_text, sizeof(option_text));

    int option = atoi(option_text);
    const char *separator = "";

    if (option == 1) {
        separator = "";
    } else if (option == 2) {
        separator = "-";
    } else if (option == 3) {
        separator = "_";
    } else {
        fprintf(stderr, "Nieprawidlowy wybor.\n");
        return 1;
    }

    FileItem *items = NULL;
    size_t count = 0;

    if (!collect_files(folder, &items, &count)) {
        fprintf(stderr, "Nie udalo sie odczytac folderu: %s\n", folder);
        free_items(items, count);
        return 1;
    }

    if (count == 0) {
        printf("W podanym folderze nie znaleziono plikow.\n");
        free_items(items, count);
        return 0;
    }

    qsort(items, count, sizeof(FileItem), compare_files);

    for (size_t i = 0; i < count; i++) {
        const char *extension = get_extension(items[i].original_name);

        items[i].final_name = create_final_name(
            base_name,
            separator,
            i + 1,
            extension
        );

        if (!items[i].final_name) {
            fprintf(stderr, "Brak pamieci.\n");
            free_items(items, count);
            return 1;
        }
    }

    for (size_t i = 0; i < count; i++) {
        items[i].temp_name = create_temp_name(folder, i + 1);

        if (!items[i].temp_name) {
            fprintf(stderr, "Nie udalo sie utworzyc nazwy tymczasowej.\n");
            rollback_temp_names(folder, items, count);
            free_items(items, count);
            return 1;
        }

        char *old_path = join_path(folder, items[i].original_name);
        char *temp_path = join_path(folder, items[i].temp_name);

        if (!old_path || !temp_path) {
            fprintf(stderr, "Brak pamieci.\n");
            free(old_path);
            free(temp_path);
            rollback_temp_names(folder, items, count);
            free_items(items, count);
            return 1;
        }

        if (rename(old_path, temp_path) != 0) {
            fprintf(stderr, "Nie udalo sie zmienic nazwy: %s\n", items[i].original_name);
            fprintf(stderr, "Powod: %s\n", strerror(errno));

            free(old_path);
            free(temp_path);

            rollback_temp_names(folder, items, count);
            free_items(items, count);
            return 1;
        }

        free(old_path);
        free(temp_path);
    }

    for (size_t i = 0; i < count; i++) {
        char *final_path = join_path(folder, items[i].final_name);

        if (!final_path) {
            fprintf(stderr, "Brak pamieci.\n");
            rollback_temp_names(folder, items, count);
            free_items(items, count);
            return 1;
        }

        if (path_exists(final_path)) {
            fprintf(stderr, "Docelowa nazwa juz istnieje: %s\n", items[i].final_name);
            free(final_path);

            rollback_temp_names(folder, items, count);
            free_items(items, count);
            return 1;
        }

        free(final_path);
    }

    for (size_t i = 0; i < count; i++) {
        char *temp_path = join_path(folder, items[i].temp_name);
        char *final_path = join_path(folder, items[i].final_name);

        if (!temp_path || !final_path) {
            fprintf(stderr, "Brak pamieci.\n");
            free(temp_path);
            free(final_path);
            rollback_temp_names(folder, items, count);
            free_items(items, count);
            return 1;
        }

        if (rename(temp_path, final_path) != 0) {
            fprintf(stderr, "Nie udalo sie ustawic nazwy koncowej: %s\n", items[i].final_name);
            fprintf(stderr, "Powod: %s\n", strerror(errno));

            free(temp_path);
            free(final_path);
            rollback_temp_names(folder, items, count);
            free_items(items, count);
            return 1;
        }

        printf("%s -> %s\n", items[i].original_name, items[i].final_name);

        free(temp_path);
        free(final_path);
    }

    printf("\nGotowe. Zmieniono nazwy %zu plikow.\n", count);

    free_items(items, count);
    return 0;
}

int main(void) {
    int keep_running = 1;

    while (keep_running) {
        run_rename_once();
        keep_running = ask_continue();
    }

    printf("\nProgram zakonczony.\n");
    return 0;
}
