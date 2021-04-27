#pragma once


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


    //Estos se reinician
    int tiempo_S;
   
    //tiempo que lleva en cpu antes de ceder
    int tiempo_A;
    //tiempo que lleva waiting antes de ceder
    int tiempo_B;
}Process;


typedef struct queue {
    int n;
    int p;
    Process* fila[2048];
    
    int quantum;


} Queue;

//imprime los procesos de cada cola con sus estado
void print_colas(Queue** colas, int Q);

//crea el sorted array
char*** create_sorted_array(InputFile* data);Queue* process_to_cpu(Queue* cola, int index);

//crea las Q colas
Queue** create_queues(int Q, int q);

//crea un proceso
Process* create_process(char** info);


//pasa tiempo S y sube a la primera.
Queue** first_priority(Queue** colas, Process* proceso);


Queue** search_process(int len, char*** sorted_array, Queue** colas, int ciclo) ;


//cambia atributos de proceso que esta WAITING
Process* process_waiting(Process* process);

//reinicia atributos cuando un proceso sale de la cpu (cede o interrumpido)
Process* out_cpu(Process* process);


//cambia atributos de proceso que pasará a estar RUNNING
Process* select_process(Process* process);
Queue** change_queue(Queue** colas, int origen, int destiny, int indice_proceso);
Queue** cpu_to_queue(Queue** colas, Process* proceso, int destino);
void write_output(char* name, Process* process);
void create_csv(char* output);
void write_csv(Process* pro, char* output);
