#pragma once

typedef struct queue {
    int p;
    Process* fila[2048];
    int quantum;

} Queue;

typedef struct process{
    int pid;
    char* name;
    int p;
    //Estado 0=RUNNING, 1= READY 3=WAITING 4=FINISHED
    int state;
    int cycles;
    int wait;
    int waitd;
    int turns;
    int interruptions;
    int turnaround_time;
    int response_time;
    int waiting_time;
}Process;

void write_output(char* name, Process* process);
Process* create_process(char** info);
