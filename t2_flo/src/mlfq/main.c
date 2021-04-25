#include <stdio.h>
#include <stdlib.h>
#include "../file_manager/manager.h"
#include "main.h"
#define RUNNING 0
#define READY 1
#define WAITING 2
#define FINISHED 3
#define N 2048;

int main(int argc, char **argv)
{
  //Guardar argumentos y leer archivo
  InputFile* data = read_file(argv[1]); 
  char* output = argv[2];
  int Q = atoi(argv[3]);
  int q = atoi(argv[4]);
  int S = atoi(argv[5]);
  printf("%s %i %i %i\n",output,Q,q,S);

  //Creamos las Q colas y le asignamos prioridad y quantum
  Queue** colas = malloc(Q * sizeof(*colas));
  for( int i = 0 ; i < Q ; i++ ){  
    colas[i]= malloc(sizeof(Queue));
    colas[i]->p = Q-1-i;
    colas[i]->quantum = (Q - colas[i]->p)*q;
    colas[i]->inicio=0;
    colas[i]->final=0;
  }
  

  int ciclo = 0;
  int ocupado = 0;

  int procesos_terminados = 0;
  int procesos_totales = data->len;

  int ciclos_proceso_actual = 0;
  int n_cola = 0;
  int indice_final = 0;
  Process* proceso_actual;

  //while(procesos_terminados<procesos_totales)
  while(ciclo<10){
    printf("Ciclo: %i\n",ciclo);
   
    print_colas(colas,Q);

    
      //buscamos procesos que llegan en este ciclo    
      //no se si crear todos los procesos antes y meterlos a una lista y ordenarlos
      for( int i = 0;i<data->len;i++){
        int llegada = atoi(data->lines[i][2]);
        printf("LLegada %i   %i",llegada,ciclo);
        
        printf("\n");
        //si corresponde creamos el proceso y lo metemos en la primera fila al final
          if(ciclo==llegada){   
            
            Process* pro = create_process(data->lines[i]);  
            printf("Llegó el proceso %i\n",pro->pid);    
            //siempre llegan a la primera cola al primer espacio vacio
            
            indice_final = colas[0]->final%N;            
            colas[0]->fila[indice_final]=pro;  
            colas[0]->final++;                       
          }        
      }


    if(ocupado){
      proceso_actual->turn_time++;
      proceso_actual->cpu_time ++;
      proceso_actual->tiempo_A ++;
      //proceso termina hay que ponerle finished
      if(proceso_actual->cycles==proceso_actual->cpu_time){
        proceso_actual->state = FINISHED;
        procesos_terminados++;
        //calcular turnaround time;
        proceso_actual->turnaround_time = ciclo - proceso_actual->start;

        indice_final = colas[proceso_actual->p]->final%N;
        colas[proceso_actual->p]->fila[indice_final]=indice_final;    

      }
      //si el proceso uso todo su quantum
      else if(proceso_actual->turn_time==colas[n_cola]->quantum){
        ocupado=0;
        proceso_actual->interruptions++;
         if (proceso_actual->turn_time==proceso_actual->wait){
           proceso_actual->state = WAITING;           
         }
        //sacarlo de mi cola
        indice_final = colas[proceso_actual->p]->final%N;
        colas[proceso_actual->p]->fila[indice_final]=indice_final; 

        //ponerlo en una más abajo 
        proceso_actual->p++;

 
      //si el proceso cede la cpu
      }else if (proceso_actual->turn_time==proceso_actual->wait){
        ocupado=0;
        proceso_actual->state = WAITING;

        //sacarlo de mi cola
        indice_final = colas[proceso_actual->p]->final%2048;
        colas[proceso_actual->p]->fila[indice_final]=indice_final;       

        //subirlo de cola
        proceso_actual->p--;

      }
    }

    //si se libero el proceso metemos otro para que ejecute el siguiente ciclo
    if(!ocupado){
      //buscar procesos READY en las colas (en orden)
      //sumarle un ciclo de waiting a todos los procesos
      Process* proceso;
      for (int i = 0; i < Q ;i++){   
        printf("%i   %i   \n",colas[i]->inicio,colas[i]->final);             
        for(int j = colas[i]->inicio; j<colas[i]->final; j++ ){

          if(colas[i]->inicio!=colas[i]->final){
                
              proceso = colas[i]->fila[j];

              if (proceso->state!=FINISHED && proceso->state!=RUNNING){
                proceso->waiting_time ++;
                if (ciclo - proceso->start > S && proceso->p!=0){
                  //sacarlo de su cola
                  //meterlo a la primero cola
                }
              }
              if(proceso->state==WAITING){            
                proceso->tiempo_B ++;
                proceso->io_time ++;
                if(proceso->tiempo_B==proceso->waitd){
                  proceso->state = READY;
                }
              }

              if(proceso->state==READY && !ocupado ){
                proceso_actual=proceso;
                proceso_actual->state = RUNNING;            
                proceso_actual->turns ++;            
                ocupado = 1;
                colas[i]->inicio ++;
                colas[i]->fila[j] = 0;

                //Response time
                if(proceso_actual->first_time){
                  proceso_actual->response_time = proceso_actual->waiting_time;
                  proceso_actual->first_time = 0;
                }
              }
          }

        }
      }


    }
    ciclo++;





    


  }
  for( int i = 0 ; i < Q ; i++ ){  
     free(colas[i]);
  }
  free(colas);
  input_file_destroy(data);


  return 0;
  

}



void print_colas(Queue** colas, int Q){
  Process* proceso;
  for (int i = 0; i<Q ; i++){
    printf("Cola %i\n",i);
    for(int j = colas[i]->inicio; j<colas[i]->final ; j++){
      proceso = colas[i]->fila[j];
      printf("%i- %s \n",j,proceso->name);
    }
    printf("\n");


  }
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
  pro->turn_time=0;


  return pro;
};