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
  }


  

  int ciclo = 0;

  //while(1){
    // buscamos procesos que llegan en este ciclo

    for( int i = 0;i<data->len;i++){
      int llegada = data->lines[i][2];
      printf("%s",data->lines[i][2]);
      printf("\n");
      //si corresponde creamos el proceso y lo metemos en la primera fila al final
        if(ciclo==llegada){        
          Process* pro = create_process(data->lines[i]);   
           



          
        }
    }
    //break;



    


  //}
  for( int i = 0 ; i < Q ; i++ ){  
     free(colas[i]);
  }
  free(colas);
  input_file_destroy(data);


  return 0;
  

}

Process* create_process(char** info){
  Process* pro = malloc(sizeof(Process));
  
  pro->state = READY;
  pro->name = info[0];
  pro->pid = info[1];
  pro->cycles = info[3];
  pro->wait = info[4];
  pro->waitd = info[5];

  pro->turns = 0;
  pro->interruptions=0;
  pro->turnaround_time=0;
  pro->response_time=0;
  pro->waiting_time=0;

  return pro;
};