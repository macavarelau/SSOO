#pragma once

typedef struct queue {
    int p;
    Process* fila[2048];
    int quantum;
    //primera posición de la lista
    int inicio;
    //primer espacio libre        
    int final;

} Queue;

typedef struct process{
    int pid;    
    char* name;
    //prioridad equivale a la cola en que está (no sé si es necesario)
    int p;
    //tiempo de llegada
    int start;
    //Estado 0=RUNNING, 1= READY 3=WAITING 4=FINISHED
    int state;
    //ciclos totales en cpu antes de terminar
    int cycles;
    //cantidad de ciclos antes de ceder la cpu (tiempo A)
    int wait;
    //cantidad de ciclos que estara waiting antes de estar ready (tiempo B)
    int waitd;
    //recien llegado
    int first_time;

    //cuantas veces fue elegido para ejecutar la cpu
    int turns;
    //cuantas veces el scheduler lo sacó de ejecución
    int interruptions;
    //tpo entre que llego y termino de ser ejecutado
    int turnaround_time;
    //tpo en es atendido desde que llego
    int response_time;
    //suma del tpo que estuvo waiting y ready (desde que llega hasta que está finished)
    int waiting_time;

    //tiempo total que ha ejecutado acumulado
    int cpu_time;  
    //tiempo total que estuvo waiting acumulado
    int io_time;
    //tiempo que lleva en esta ejecución
    int turn_time;

    //Estos se reinician
    //tiempo que lleva en cpu antes de ceder
    int tiempo_A;
    //tiempo que lleva waiting antes de ceder
    int tiempo_B;
}Process;

void write_output(char* name, Process* process);
Process* create_process(char** info);
