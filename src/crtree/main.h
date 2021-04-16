#pragma once



// Define the struct
typedef struct worker {
    int index;
    char* archivo;
    int n_args;
    char** params;
    int time;
    int exit_status;
    
} Worker;

typedef struct manager {
    int index;
    int timeout;
    int n_args;
    int root;
    pid_t *childs[255]; 
} Manager;

// Declare functions
void write_file(Worker* w);
void free_worker(Worker* w);
void free_manager(Manager* m);
void write_father_file(Manager* mg);
int append_father_file(char* filePath, InputFileChild* child_data, int c);
Worker* create_worker(char*** proceso, int index);
Manager* create_manager(Manager* mg,char*** proceso, int index);