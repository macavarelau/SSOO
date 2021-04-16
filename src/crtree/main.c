#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../file_manager/manager.h"
#include "main.h"
#include <signal.h>


static volatile int MAX_SIZE = 255; 
static volatile int keepRunning = 1;
static volatile int interrupted = 0;
static volatile int isTimeout = 0;
static volatile char tipo = 'x';
static volatile pid_t child_worker;
Manager** man;


//Handler de SIGINT
void intHandler(int x) {
  Manager* mg = man;
  pid_t hijo;
  if (tipo=='R'){
    keepRunning=0;       
    kill(getpid(),SIGABRT);    
  }    
}

//Handler de SIGABRT
void abrtHandler(int x) {
  Manager* mg = man;
  pid_t hijo;
  keepRunning = 0;
  if (tipo=='R'){
    //Propaga kill a sus hijos
    for(int i =0;i<mg->n_args;i++){
      hijo = mg->childs[i];  
  
      kill(hijo,SIGABRT);
    }
  }else if (tipo =='M'){
    //Propaga kill a sus hijos
    for(int i =0;i<mg->n_args;i++){
      hijo = mg->childs[i];
      kill(hijo,SIGABRT);
     
    }

  }else if (tipo=='W'){    
    printf("SIGABRT A HIJO WORKER\n");   
    kill(child_worker,SIGABRT);   
  }  
}

void alarm_handler(int x){
  Manager* mg = man;
  kill(getpid(),SIGABRT);  
  printf("timeput\n\n");
}





int main(int argc, char **argv)
{   
  int index;
/*   int father_index;
  char sfather_path[64];
  char* hijo_index; */

  Manager* mg = malloc(sizeof(Manager));
  //mg->childs = malloc(MAX_SIZE * sizeof(pid_t));

  
  // Recibe el archivo y el número de proceso por el que se quiere partir
  // Define el tipo de procesos
  index = atoi(argv[2]);
  InputFile* data = read_file(argv[1]); 
  char*** proceso = data->lines;
  signal(SIGINT, intHandler); 
  signal(SIGABRT, abrtHandler);  
  signal(SIGALRM, alarm_handler);

  while(keepRunning){

    tipo = proceso[index][0][0];

    if(tipo=='M' || tipo=='R'){ 

      //Procesos Manager     
      mg = create_manager(mg, proceso,index);
      index = manage(mg,proceso,index);
      //father_index = mg->index;

      if(index==-1){

        /* write_father_file(mg);
        for(int c=0;c<mg->n_args;c++){
          hijo_index = strdup(proceso[father_index][3+c]);
          if (hijo_index[strlen(hijo_index)-1] == '\n' ) {
              hijo_index[strlen(hijo_index)-1] = 0;
          }
          printf("Padre line: %i, hijo actual: %s\n", father_index, hijo_index);
          InputFileChild* child_data = read_file_child(strcat(strdup(hijo_index), ".txt"));
          sprintf(sfather_path, "%i.txt", father_index);
          append_father_file(sfather_path, child_data, c);
          input_file_destroy_child(child_data);
        } */
        join(mg->index,mg,proceso);       
        free(mg);  //libera la memoria utilizara para el proceso manager
        input_file_destroy(data); // elimina la estructura de data
        return 0;
      }        
    }
    
    else if (tipo=='W'){
      //Proceso worker
      int status;
      Worker* w = create_worker(proceso,index);       
      time_t start_time = time(NULL);
        
      child_worker = fork();
 
      // Proceso hijo ejecuta el programa
      if (child_worker == 0) { 
          tipo = 'e'; 
          sleep(3);
          char* args[w->n_args+1];
          args[0] = w->archivo;
          if(w->n_args > 0){
            for(int i=0;i<w->n_args;i++){
              args[i+1] = w->params[i];
            }  
          }
          args[w->n_args+1]= NULL;  
          execvp(w->archivo, args); 
        }
      
      waitpid(child_worker, &status, 0);
      // Tiempo al que terminó
      time_t duration = time(NULL)-start_time;
      w->time = (int) duration;
      printf("Hjo de worker demoro %i segundos\n", w->time);

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
    
    }else{
      //free_manager(mg);   
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
  //mg->childs = malloc(mg->n_args * sizeof(char*));

  return mg;
} 

/* Manager* create_manager(Manager* mg,char*** proceso,int index){
    
  mg->timeout = atoi(proceso[index][1]);
  mg->n_args = atoi(proceso[index][2]);
  mg->index=index;  

  return mg;
}  */

Worker* create_worker(char*** proceso,int index){
  Worker* wk = malloc(sizeof(Worker));
  wk->archivo = strdup(proceso[index][1]);    
  wk->index = index;
  wk->n_args = atoi(proceso[index][2]);
  wk->params = malloc(wk->n_args * sizeof(char*));    
  wk->exit_status = 0;
  wk->time = 0;
  for(int i = 0; i < wk->n_args; i++) {
        wk->params[i] = strdup(proceso[index][3+i]);
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
    mg->childs[i] = (pid_t) child_pid;
    if(child_pid==0){       
      return index;
    }
  }
    // install an alarm to be fired after TIME_LIMIT
 
  //while(wait(NULL) > 0);
  


  alarm(mg->timeout);   
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
  
    fprintf(file,"%i,", w->time);  
    fprintf(file,"%i,", w->exit_status);   
    fprintf(file,"%i\n", interrupted);  
  
    fclose(file);
}




int join(int ind,Manager* m,char*** proceso) {

    char filePath[64];
    sprintf(filePath, "%i.txt", ind);

    FILE *ffp;
    ffp = fopen(filePath, "w");
    if (ffp == NULL) {
        printf("\nError al leer el archivo '%i'.\n", ind);
        exit(0);
    }

    char c;
    char filename[64];
    FILE* child_file;

    for (int i=0;i< m->n_args;i++){        
        sprintf(filename, "%c.txt", proceso[m->index][3+i][0]);
        printf("\n\nfilename: %s\n\n",filename);
        child_file = fopen( filename, "r+");
        if (child_file == NULL) {
          printf("\nError al leer el archivo '%s'.\n", filename);
          exit(0);
        }
        c = fgetc(child_file);
        while (c != EOF)
        {       
          fputc(c, ffp);
          c = fgetc(child_file);
     
        }
            
        fclose(child_file);
    } 


    fclose(ffp);

    return 0;
}

















/* void free_manager(Manager* m){

  free(m->childs);
  free(m);
}
 */

//Función que libera memoria
void free_worker(Worker* w){
  printf("%i\n",w->n_args);

  for(int i = 0; i<w->n_args;i++){        
    free(w->params[i]); 
          } 
  free(w->params);
  free(w->archivo);
  free(w);
}



void write_father_file(Manager* mg){

    FILE *father_file;
 
    char filename[64];
    sprintf(filename, "%i.txt", mg->index);
    father_file = fopen( filename, "w");

    // exiting program 
    if (father_file == NULL) {
        printf("Error!");
        exit(1);
    }
    fclose(father_file);
}

int append_father_file(char* filePath, InputFileChild* child_data, int c) {

    FILE *ffp;
    printf("FILEPATH PADRE: %s\n", filePath);

    ffp = fopen(filePath, "a");

    if (ffp == NULL) {
        printf("\nError al leer el archivo '%s'.\n", filePath);
        exit(0);
    }

    for (int k=0; k<child_data->count; k++) {
      printf("Lineas: %i\n", child_data->count);
      printf("Linea: %s\n", child_data->lines[k]);
      if (c>0 && k==0) {
        fprintf(ffp,"\n%s", child_data->lines[k]);
      } else {
        fprintf(ffp,"%s", child_data->lines[k]);
      }
    }

    fclose(ffp);

    return 0;
}


