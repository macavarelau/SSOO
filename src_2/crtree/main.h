#pragma once



// Define the struct
typedef struct worker {
    int index;
    char* archivo;
    int n_args;
    char** params;
    double time;
    int exit_status;
    
} Worker;

typedef struct manager {
    int index;
    int timeout;
    int n_args;
    int root;
    
} Manager;

// Declare functions
int manage(Manager* mg, char*** proceso, int index);
void write_file(Worker* w);
void write_father_file(Manager* mg);
int append_father_file(char* filePath, InputFileChild* child_data, int c);
void free_worker(Worker* w);
Worker* create_worker(char*** proceso, int index);
Manager* create_manager(Manager* mg, char*** proceso, int index);