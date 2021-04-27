#include <stdio.h>
#include <stdlib.h>
#include "../file_manager/manager.h"
#include "main.h"
#define RUNNING 0
#define READY 1
#define WAITING 2
#define FINISHED 3
#define N 2048

int process_list_index = 0;

int main(int argc, char **argv)
{
  //Guardar argumentos y leer archivo
  InputFile* data = read_file(argv[1]); 
  char* output = argv[2];
  int Q = atoi(argv[3]);
  int q = atoi(argv[4]);
  int S = atoi(argv[5]);

  
  //Ordenamos la lista de procesos según tiempo de llegada
  char*** sorted_array = create_sorted_array(data);  
  Queue** colas = create_queues(Q,q);
  Process* proceso_actual;
  

  int ciclo=0, ocupado=0, procesos_terminados=0;
  int procesos_totales = data->len;

 
  // while(ciclo<30)
  while(procesos_terminados<procesos_totales)
 {    
    printf("\n************************************\n");  
    printf("**************Ciclo: %i*************",ciclo); 
    printf("\n************************************\n\n");   
    
    printf("Procesos terminados: %i / %i\n",procesos_terminados,procesos_totales);   
    
    colas = search_process(data->len, sorted_array, colas, ciclo);  
    



    if(ocupado){
      printf("   [CPU OCUPADA]\n");
      //tiempo cpu acumulado
      proceso_actual->cpu_time ++;
      //timepo cpu en este turno
      proceso_actual->tiempo_A ++;
      //proceso termina hay que ponerle finished
      if(proceso_actual->cycles==proceso_actual->cpu_time){
        printf("%s termino\n", proceso_actual->name);        
        ocupado = 0;
        proceso_actual->state = FINISHED;
        procesos_terminados++;
        //calcular turnaround time;
        proceso_actual->turnaround_time = ciclo - proceso_actual->start;
        //ACA ESCRIBIR PROCESO EN ARCHIVO DE OUTPUT
        write_csv(proceso_actual, output); 
        
        free(proceso_actual);
        if(procesos_terminados==procesos_totales){
          break;
        }
        
        
         

      }
      //si el proceso uso todo su quantum
      else if(proceso_actual->tiempo_A==colas[proceso_actual->p]->quantum){
        printf("%s consumió su quantum\n",proceso_actual->name);
        ocupado=0;
        proceso_actual->interruptions++;
        proceso_actual = out_cpu(proceso_actual);    

        //controlamos caso borde en que el quantum termina al mismo tiempo que cede la cpu
         if (proceso_actual->tiempo_A==proceso_actual->wait){
           proceso_actual->state = WAITING;           
         }else{
           proceso_actual->state = READY; 
         }


        //Si es posible baja al proceso a una cola de menor prioridad 
        if (proceso_actual->p < Q-1){
          proceso_actual->p++;
        }
        colas = cpu_to_queue(colas, proceso_actual, proceso_actual->p);
 
      //si el proceso cede la cpu
      }else if (proceso_actual->tiempo_A==proceso_actual->wait){
        printf("%s cedio cpu\n",proceso_actual->name);

        ocupado=0;
        proceso_actual->state = WAITING;
        proceso_actual = out_cpu(proceso_actual);

        
        //Si es posible baja al proceso a una cola de mayor prioridad;
        if (proceso_actual->p > 0){
          proceso_actual->p--;           
        }
        colas = cpu_to_queue(colas, proceso_actual, proceso_actual->p);


      }else{
        printf("Proceso ejecutando: %s\n",proceso_actual->name);
        printf("Cola: %i \n",proceso_actual->p);
        printf("Tiempo total en cpu: %i\n",proceso_actual->cpu_time);
        printf("Tiempo total en wait: %i\n",proceso_actual->io_time);
        printf("Tiempo en este turno en cpu: %i\n",proceso_actual->tiempo_A);
        printf("Interrupciones: %i\n",proceso_actual->interruptions);
        printf("Turnos: %i\n",proceso_actual->interruptions);

      }
    }else{
      printf("   [CPU LIBRE] \n");
    }

    print_colas(colas,Q); 

    Process* proceso;
    for (int i = 0; i < Q ;i++){              
      for(int j = 0; j < colas[i]->n; j++ ){

        //si la cola no esta vacia
        if(colas[i]->n > 0){  
            //voy sacando los procesos en orden  
            //en las colas solo estan waiting o redi porque el finished se libera y el que esta running se saca de su cola            
            proceso = colas[i]->fila[j];          
            proceso->waiting_time ++;
              //chequeamos si paso su tiempo S
            if (ciclo - proceso->start > S && proceso->p!=0){
              
              colas = change_queue(colas, proceso->p, 0, j);
              proceso->p = 0;
              
            }
            
            if(proceso->state==WAITING){ 
              //modifico atributos del proceso (podría pasar a ready por eso no hay un else if despues)
              proceso = process_waiting(proceso);   
            
            }

            if(proceso->state==READY && !ocupado ){
              //modifico atributos del proceso que pasa a cpu
        
              proceso = select_process(proceso); 
              //lo saco de la cola en que estaba
              colas[i] = process_to_cpu(colas[i], j);   
              proceso_actual=proceso;
              ocupado = 1;         

            
            }
          }
        }
      }

    ciclo++;


    }

  printf("Procesos terminados: %i / %i\n",procesos_terminados,procesos_totales); 
    printf("\n************************************\n");  
    printf("****************FIN*****************"); 
    printf("\n************************************\n");   

  //libero la memoria de procesos, colas y el input file.  
  for( int i = 0 ; i < Q ; i++ ){ 
    for( int j = 0 ; j < colas[i]->n ; j++ ) {
      free(colas[i]->fila[j]);
    }    
    free(colas[i]);    
  }
  free(colas);
  input_file_destroy(data);

  return 0;  
}





void print_colas(Queue** colas, int Q){
  Process* proceso;
  printf("\n");
  char lista[4][10] = {"RUNNING","READY","WAITING","FINISHED"};
  for (int i = 0; i<Q ; i++){
    printf("Procesos en cola %i:\n",i);
    if(colas[i]->n == 0){
      printf("NONE\n");
    }

    for(int j = 0; j<colas[i]->n ; j++){
      proceso = colas[i]->fila[j];
      printf("- %s [%s]\n",proceso->name, lista[proceso->state]);
    }
    printf("\n");


  }
  printf("\n");
}

char*** create_sorted_array(InputFile* data){
  char*** sorted_array = data->lines;
  char** temp;
  for (int i=0; i < data->len; i++) {
    for (int j = i+1; j < data->len; j++) {     
        if(atoi(sorted_array[i][2]) > atoi(sorted_array[j][2])) { 
          temp = sorted_array[i];    
          sorted_array[i] = sorted_array[j];    
          sorted_array[j] = temp;     
        }     
    }     
  }
  return sorted_array;
}


Queue** create_queues(int Q, int q){
  //Creamos las Q colas y le asignamos prioridad y quantum
  Queue** colas = malloc(Q * sizeof(*colas));
  for( int i = 0 ; i < Q ; i++ ){  
    colas[i]= malloc(sizeof(Queue));
    colas[i]->p = Q-1-i;
    colas[i]->quantum = (Q - colas[i]->p)*q;
  
    colas[i]->n = 0;
  }
  return colas;
  
}

Process* create_process(char** info){
  Process* pro = malloc(sizeof(Process));
  //siempre entra a la primera cola
  pro->p = 0; 
  pro->state = READY;
  pro->name = info[0];
  pro->pid = atoi(info[1]);
  pro->start = atoi(info[2]);
  pro->cycles = atoi(info[3]);
  pro->wait = atoi(info[4]);
  pro->waitd = atoi(info[5]);

  pro->turns = 0;
  pro->interruptions=0;
  pro->turnaround_time=0;
  pro->response_time=0;
  pro->waiting_time=0;

  pro->first_time = 1;
  pro->cpu_time=0;
  pro->io_time=0;
  pro->tiempo_A=0;
  pro->tiempo_B=0;


  return pro;
};




Process* process_waiting(Process* proceso){
  proceso->tiempo_B ++;
  proceso->io_time ++;
  if(proceso->tiempo_B==proceso->waitd){
    proceso->state = READY;
  }
  return proceso;

};

Process* select_process(Process* proceso){    
    proceso->state = RUNNING;            
    proceso->turns ++;         
    
    //Response time
    if(proceso->first_time){
      proceso->response_time = proceso->waiting_time;
      proceso->first_time = 0;
    }
    return proceso;
};
Queue* process_to_cpu(Queue* cola, int indice_proceso){
    //lo sacamos de la cola y rellenamos los espacios
    cola->n --;   
    for(int i = indice_proceso; i < cola->n - indice_proceso ;i++){
      cola->fila[i]=cola->fila[i+1];
    }
    return cola;
}


Queue** change_queue(Queue** colas, int origen, int destino, int indice_proceso){
  //en la cola origen
  Process* proceso = colas[origen]->fila[indice_proceso];
  colas[origen]->n --; 
  for(int i = indice_proceso; i < colas[origen]->n - indice_proceso ;i++){
      colas[origen]->fila[i]=colas[origen]->fila[i+1];
    }
  //cola destino
  colas[destino]->fila[colas[destino]->n]=proceso;
  colas[destino]->n++;  
  return colas;

}
Queue** cpu_to_queue(Queue** colas, Process* proceso, int destino){
  colas[destino]->fila[colas[destino]->n]=proceso;
  colas[destino]->n++;  
  return colas;

}


Queue** search_process(int len, char*** sorted_array, Queue** colas, int ciclo) {

  int llegada;
  for(int i=process_list_index; i<len; i++) {

    llegada = atoi(sorted_array[i][2]);
    if(ciclo==llegada){  
      
      Process* pro = create_process(sorted_array[i]);   
      printf("Proceso: %s entra a la cola 0\n",pro->name);               
      colas[0]->fila[colas[0]->n] = pro;
      colas[0]->n++;      
    } else if(ciclo<llegada){
      process_list_index = i;
      return colas;
    }     


  }
  return colas;   

}      


Process* out_cpu(Process* process){
  process->tiempo_A = 0;
  process->tiempo_B = 0;
  return process;
};



void create_csv(char* output) {
  FILE *fpt;
  fpt = fopen(output, "w+");
  fclose(fpt);
}

void write_csv(Process* pro, char* output) { 
  FILE *file = fopen(output, "a+" );
  fprintf(file,"%s,%i,%i,%i,%i,%i\n", pro->name,pro->turns,pro->interruptions,pro->turnaround_time,pro->response_time,pro->waiting_time);
  fclose(file);
};