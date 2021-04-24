
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include "../file_manager/manager.h"
#include "main.h"
static volatile int MAX_SIZE = 255; 
static volatile int keepRunning = 1;
static volatile int interrupted = 0;
static volatile int is_timeout = 0;
static volatile int child_done = 0;
static volatile char tipo = 'x';
static volatile pid_t child_worker;
static volatile int child_worker_done = 0;
Manager** man;


int main(int argc, char **argv){ 

  InputFile* data = read_file(argv[1]); 
  Manager* mg = malloc(sizeof(Manager));
  man = mg;
  int index = atoi(argv[2]);
  char*** proceso = data->lines;

  signal(SIGINT, intHandler); 
  signal(SIGABRT, abrtHandler);  

  while( keepRunning ){
    tipo = proceso[index][0][0];

    if(tipo=='M' || tipo=='R'){    

      mg = create_manager(mg, proceso,index);
      index = manage(mg,proceso,index);   

      if(index==-1){

        join(mg,proceso);       
        free(mg);  //libera la memoria utilizara para el proceso manager
        input_file_destroy(data); // elimina la estructura de data
        return 0;

      }        
    } else if ( tipo=='W' ){
      int status;

      Worker* w = create_worker(proceso,index);      
      time_t start_time = time(NULL);
        
      child_worker = fork();

      if (child_worker == 0) { 
          tipo = 'e'; 
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
      child_worker_done = 1;
      time_t duration = time(NULL)-start_time;
      w->time = (int) duration;

      if ( WIFEXITED(status) )
      {
          w->exit_status = WEXITSTATUS(status);        
      }  

      write_file(w);  
      free_worker(w);   
      free(mg);
      input_file_destroy(data); 
      return 0;
    
    }else{

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
  mg->finished_childs = 0;
  return mg;
} 



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
  pid_t child_pid ;

  
  for (int i=0; i<mg->n_args;i++){    
    index = atoi(proceso[mg->index][3+i]);
    printf("%i\n",index);    
    char tipo = proceso[index][0][0];
    printf("%c tipo fork\n",tipo);
    child_pid = fork();
    mg->childs[i] = (pid_t) child_pid;
    if(child_pid==0){       
      return index;
    }
  }

  signal(SIGALRM, alarm_handler);
  signal(SIGCHLD, child_handler); 

  alarm(mg->timeout);   
  
  int pid;
  while(mg->finished_childs<mg->n_args){  
    pause();
    if (is_timeout){
      break;
    }
    else if (child_done){    
      printf("child finished normally\n");
      mg->finished_childs ++;
      printf("%i  %i\n", mg->finished_childs, mg->n_args);
      
      for(int i=0 ; i<mg->n_args;i++){
        if(pid==mg->childs[i]){
          mg->childs[i]=0;
        }
      }
      pid = wait(NULL);
    }
  }

   
  return -1; 
}
  

//Funcion que escribe el output de un worker en un archivo.
void write_file(Worker* w){

    FILE *file;
    char filename[64];
    sprintf(filename, "%i.txt", w->index);
    file = fopen( filename, "w");

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




void join(Manager* m,char*** proceso) {
  
    int ind = m->index;

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


//MANEJO ARCHIVOS

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


//SEÑALES

//Handler de SIGINT
void intHandler(int x) {
  if (tipo=='R'){
    keepRunning=0;       
    kill(getpid(),SIGABRT);    
  }    
}

//Handler de SIGABRT
void abrtHandler(int x) {

  Manager* mg = (Manager*) man;
  pid_t hijo;
  keepRunning = 0;
  if (tipo=='R'){
    //Propaga kill a sus hijos
    for(int i =0;i<mg->n_args;i++){
      hijo = mg->childs[i];
      if(hijo!=0){
      kill(hijo,SIGABRT);   }  
    }
  }else if (tipo =='M'){
    //Propaga kill a sus hijos
    for(int i =0;i<mg->n_args;i++){
      hijo = mg->childs[i];
      if(hijo!=0){
      kill(hijo,SIGABRT);  }  
    }

  }else if (tipo=='W'){    
    if (!child_worker_done) {
      interrupted = 1;
      printf("SIGABRT A HIJO WORKER\n");   
      kill(child_worker,SIGABRT);   

    }
  }   
}

void alarm_handler(int x){

  is_timeout=1;
  printf("alarm\n");
  kill_childs();
}


void kill_childs(){

  Manager* mg = (Manager*) man;
  int pid;
  int result;
  for(int i=0 ; i< mg->n_args ; i++){
      pid = mg->childs[i];  
      if (pid){      
      result = waitpid(pid, NULL, WNOHANG);
        if (result == 0) {
            // child still running, so kill it
            printf("killing child\n");
            kill(pid, SIGABRT);
            wait(NULL);        
        }
      }
    }
}

void child_handler(int x){
  child_done =1;    
}



