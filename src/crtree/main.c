#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define _POSIX_C_SOURCE 199309L
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../file_manager/manager.h"
#include "main.h"


int main(int argc, char **argv)
{  
  int index;
  char tipo;
  Manager* mg = malloc(sizeof(Manager));  
  InputFile* data;
  int first;
  // Recibe el archivo y el número de proceso por el que se quiere partir
  // Define el tipo de procesos
  index = atoi(argv[2]);
  data = read_file(argv[1]); 
  char*** proceso = data->lines;

  
  
  printf("indice %i\n",index);
  int continuar =1;

  while(continuar){

    tipo = proceso[index][0][0];


    if(tipo=='M' || tipo=='R'){
     
      //Procesos Manager
     
      mg = create_manager(mg, proceso,index);

      index = manage(mg,proceso,index);
      if(index==-1){
        free(mg);  
        input_file_destroy(data);
        return 0;
      }   
      
    }
    //Proceso worker
    else if (tipo=='W'){
      int status;
      struct timespec start, end;
      Worker* w = create_worker(proceso,index);
       
      clock_gettime(CLOCK_MONOTONIC, &start);      
      pid_t child_worker = fork();
        // Proceso hijo ejecuta el programa
      if (child_worker == 0) {                  
          sleep(3);
          execl("/bin/date", "date", 0, (char*) NULL); 
        }
      
      waitpid(child_worker, &status, 0);
      // Tiempo al que terminó
      clock_gettime(CLOCK_MONOTONIC, &end);
      w->time = ((double)end.tv_sec + 1.0e-9*end.tv_nsec) - ((double)start.tv_sec + 1.0e-9*start.tv_nsec);
      printf("Hjo de worker demoro %.5f segundos\n", w->time);

      if ( WIFEXITED(status) )
      {
          w->exit_status = WEXITSTATUS(status);        
          printf("Exit status of the child was %d\n", w->exit_status);
      }
     
      
      write_file(w);         
    
      free_worker(w); 
      free(mg);
      input_file_destroy(data);
      return 0;
    }
    

  }

}



Manager* create_manager(Manager* mg,char*** proceso,int index){
  
  mg->timeout = atoi(proceso[index][1]);
  mg->n_args = atoi(proceso[index][2]);
  mg->index=index;
  return mg;
}

Worker* create_worker(char*** proceso,int index){
  Worker* wk = malloc(sizeof(Worker));
  wk->archivo =proceso[index][1];    
  wk->index = index;
  wk->n_args = atoi(proceso[index][2]);
  wk->params = malloc(wk->n_args * sizeof(char*));    
  wk->exit_status = 0;
  wk->time = 0;
  
  for(int i = 0; i < wk->n_args; i++) {
        wk->params[i]= malloc(sizeof(proceso[index][3+i]));
        wk->params[i] = proceso[index][3+i];
      }
  return wk;
}



int manage(Manager* mg,char*** proceso,int index){ 
  for (int i=0; i<mg->n_args;i++){
    index = atoi(proceso[mg->index][3+i]);
    printf("%i\n",index);    
    char tipo = proceso[index][0][0];
    printf("%c tipo fork\n",tipo);
    pid_t child_pid = fork();
    if(child_pid==0){       
      return index;
    }
  }
  for (int i=0; i<mg->n_args;i++){
    wait(NULL);
  }
  return -1; 
}
  

//Funcion que escribe el output de un worker en un archivo.
void write_file(Worker* w){


    // creating file pointer to work with files
    FILE *file;

    // opening file in writing mode
    
    char filename[64];
    sprintf(filename, "%i.txt", w->index);
    file = fopen( filename, "w");

    // exiting program 
    if (file == NULL) {
        printf("Error!");
        exit(1);
    }
    fprintf(file, "%s,",w->archivo);
    for(int i = 0; i<w->n_args;i++){
      w->params[i][strcspn(w->params[i], "\n")] = '\0';
      fprintf(file,"%s,",w->params[i]);

    }  
  
    fprintf(file,"%.5f,", w->time);  
    fprintf(file,"%i", w->exit_status);   
  
    fclose(file);
}

void free_worker(Worker* w){
  printf("%i\n",w->n_args);

  for(int i = 0; i<w->n_args;i++){        
    free(w->params[i]); 
          } 
  free(w->params);
  free(w->archivo);
  free(w);
}