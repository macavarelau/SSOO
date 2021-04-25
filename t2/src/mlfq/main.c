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
    printf("caca\n");
    colas[i]= malloc(sizeof(Queue));
    colas[i]->p = Q-1-i;
    colas[i]->quantum = (Q - colas[i]->p)*q;
    colas[i]->inicio=0;
    colas[i]->final=0;
  }
  

  int ciclo = 0;
  int ocupado = 0;
  int ciclos_proceso_actual = 0;
  int n_cola;
  int indice_final = 0;
  Process* proceso_actual;

  while(1){


    

      //buscamos procesos que llegan en este ciclo    
      //no se si crear todos los procesos antes y meterlos a una lista y ordenarlos
      for( int i = 0;i<data->len;i++){
        int llegada = data->lines[i][2];
        printf("%s",data->lines[i][2]);
        printf("\n");
        //si corresponde creamos el proceso y lo metemos en la primera fila al final
          if(ciclo==llegada){        
            Process* pro = create_process(data->lines[i]);  
            //siempre llegan a la primera cola al primer espacio vacio
            indice_final = colas[0]->final%2048;
            colas[0]->fila[indice_final]=pro;                         
          }        
      }


    if(ocupado){
      proceso_actual->turn_time++;
      proceso_actual->cpu_time ++;
      proceso_actual->tiempo_A ++;
      //proceso termina hay que ponerle finished
      if(proceso_actual->cycles==proceso_actual->cpu_time){
        proceso_actual->state = FINISHED;
        //calcular turnaround time;
        proceso_actual->turnaround_time = ciclo - proceso_actual->start;

        indice_final = colas[proceso_actual->p]->final%2048;
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
        indice_final = colas[proceso_actual->p]->final%2048;
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
      for (int i ; i<=Q ;i++){        
        for(int j = colas[i]->inicio; j<colas[i]->final; j++ ){

          if (proceso_actual->state!=FINISHED && proceso_actual->state!=RUNNING){
            proceso_actual->waiting_time ++;

            if (ciclo - proceso_actual->start>S && proceso_actual->p!=0){
              //sacarlo de su cola
              //meterlo a la primero cola
            }

          }



          if(colas[i]->fila[j]->state==WAITING){            
            proceso_actual->tiempo_B ++;
            proceso_actual->io_time ++;
            if(proceso_actual->tiempo_B==proceso_actual->waitd){
              proceso_actual->state = READY;
            }

          }


          if(colas[i]->fila[j]->state==READY){
            proceso_actual=colas[i]->fila[j];
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
    ciclo++;




    break;



    


  }
  for( int i = 0 ; i < Q ; i++ ){  
     free(colas[i]);
  }
  free(colas);
  input_file_destroy(data);


  return 0;
  

}

Process* create_process(char** info){
  Process* pro = malloc(sizeof(Process));
  //siempre entra a la primera cola
  pro->p = 0;
  
  
  pro->state = READY;
  pro->name = info[0];
  pro->pid = info[1];
  pro->start = info[2];
  pro->cycles = info[3];
  pro->wait = info[4];
  pro->waitd = info[5];

  pro->turns = 0;
  pro->interruptions=0;
  pro->turnaround_time=0;
  pro->response_time=0;
  pro->waiting_time=0;

  pro->first_time = 1;

  return pro;
};