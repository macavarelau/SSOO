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
    int finished_childs;
    pid_t childs[255]; 
    

} Manager;

// Declare functions
void write_file(Worker* w);
void free_worker(Worker* w);
void kill_childs();

void join(Manager* mg, char*** proceso);

int manage(Manager* mg, char*** proceso, int index);
void free_manager(Manager* m);
Worker* create_worker(char*** proceso, int index);
Manager* create_manager(Manager* mg,char*** proceso, int index);
void intHandler(int x);
void abrtHandler(int x);
void alarm_handler(int x);
void kill_childs();
void child_handler();