/*
 *
 * factory_manager.c
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <stddef.h>
 #include <semaphore.h>
 #include <sys/stat.h>
 #include <pthread.h>

typedef struct {
    int id;
    int max_size;
    int num_elements;
    sem_t sem;
} ProcessManager;

typedef struct {
    int max_belts;
    ProcessManager *managers;
    int count;
} FactoryState;

static void cleanup_resources(FactoryState *state) {
    if (state) {
        for (int i = 0; i < state->count; i++) {
            sem_destroy(&state->managers[i].sem);
        }
        free(state->managers);
    }
}

static int parse_input_file(const char *filename, FactoryState *state) {
    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    if (fscanf(file, "%d", &state->max_belts) != 1 || state->max_belts <= 0) {
        fclose(file);
        return -1;
    }

    state->managers = malloc(state->max_belts * sizeof(ProcessManager));
    if (!state->managers) {
        fclose(file);
        return -1;
    }

    int id, size, elements;
    while (fscanf(file, "%d %d %d", &id, &size, &elements) == 3) {
        if (state->count >= state->max_belts) {
            fclose(file);
            return -1;
        }
        state->managers[state->count++] = (ProcessManager){id, size, elements, {0}};
    }

    if (!feof(file) || ferror(file)) {
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}

static void* process_manager_wrapper(void *arg) {
    ProcessManager *pm = (ProcessManager *)arg;
    extern int process_manager(int, int, int, sem_t*);  // Defined in process_manager.c
    
    sem_wait(&pm->sem);
    int result = process_manager(pm->id, pm->max_size, pm->num_elements, &pm->sem);
    return (result == 0) ? (void*)0 : (void*)-1;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
        return -1;
    }

    FactoryState state = {0};
    if (parse_input_file(argv[1], &state) != 0) {
        fprintf(stderr, "[ERROR][factory_manager] Invalid file.\n");
        cleanup_resources(&state);
        return -1;
    }

    // Initialize semaphores
    for (int i = 0; i < state.count; i++) {
        if (sem_init(&state.managers[i].sem, 0, 0) != 0) {
            cleanup_resources(&state);
            return -1;
        }
    }

    // Create threads
    pthread_t *threads = malloc(state.count * sizeof(pthread_t));
    if (!threads) {
        cleanup_resources(&state);
        return -1;
    }

    for (int i = 0; i < state.count; i++) {
        if (pthread_create(&threads[i], NULL, process_manager_wrapper, &state.managers[i]) != 0) {
            for (int j = 0; j < i; j++) pthread_cancel(threads[j]);
            free(threads);
            cleanup_resources(&state);
            return -1;
        }
        printf("[OK][factory_manager] Process_manager with id %d has been created.\n", state.managers[i].id);
    }

    // Signal semaphores in order to enforce execution sequence
    for (int i = 0; i < state.count; i++) {
        sem_post(&state.managers[i].sem);
    }

    // Wait for completion
    int any_error = 0;
    for (int i = 0; i < state.count; i++) {
        void *retval;
        pthread_join(threads[i], &retval);
        if ((long)retval != 0) {
            fprintf(stderr, "[ERROR][factory_manager] Process_manager with id %d failed.\n", state.managers[i].id);
            any_error = 1;
        } else {
            printf("[OK][factory_manager] Process_manager with id %d has finished.\n", state.managers[i].id);
        }
    }

    free(threads);
    cleanup_resources(&state);
    printf("[OK][factory_manager] Finishing.\n");
    return any_error ? -1 : 0;
}