#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
// #define _POSIX_C_SOURCE 199309L
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../file_manager/manager.h"
#include "main.h"


int main(int argc, char **argv)
{  
  int index; //declara variable index: indice de qué línea leerá (proceso)
  char tipo; //declara variable tipo: si es M, R o W
  int father_index;
  char sfather_path[64];
  char* hijo_index;
  Manager* mg = malloc(sizeof(Manager));  // Pide memoria para crear la estructura Manager
  InputFile* data; // Declara pointer de la data del archivo
  // Recibe el archivo y el número de proceso por el que se quiere partir
  // Define el tipo de procesos
  index = atoi(argv[2]); // linea que queremos leer, se pasa de ASCII a integer
  data = read_file(argv[1]); // Lee el archivo (input)
  char*** proceso = data->lines; //Lista de strings. Se le entrega como data las lineas del archivo



  while(1){ // Inicia while para chequear qué tipo de procesos son, hasta que se hagan todos.

    tipo = proceso[index][0][0]; //Elijo una linea, y su tipo. El otro [0] es un char.

    if(tipo=='M' || tipo=='R'){ //chequeo si es manager o root

      //Procesos Manager     
      mg = create_manager(mg, proceso,index);  //función que define y crea los procesos manager
      index = manage(mg,proceso,index); // maneja la creacion de procesos hijos, linea del proceso hijo
      father_index = mg->index;
      if(index==-1){ // termina de iterar segun cantidad de hijos
        // JUNTAR ARCHIVOS DE LOS HIJOS

        write_father_file(mg);
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
        }
        free(mg);  //libera la memoria utilizara para el proceso manager
        input_file_destroy(data); // elimina la estructura de data
        return 0;

      }        
    }
    
    else if (tipo=='W'){ //chequeo si es proceso worker
      //Proceso worker
      int status; // lo que retorna el hijo al padre. El estado al terminar.
      struct timespec start, end; // estructura de datos con la informacion del tiempo de partida y finalizacion
      Worker* w = create_worker(proceso,index); // se define el proceso worker      
      clock_gettime(CLOCK_MONOTONIC, &start); // tiempo real de un reloj: toma el inicio     
      pid_t child_worker = fork(); // se crea proceso hijo de tipo worker
      // Proceso hijo ejecuta el programa
      if (child_worker == 0) { //chequea si se creo optimamente el proceso           
          sleep(3); //suspende al proceso durante 3 segundos

          char* args[w->n_args+1]; // Se define la lista de argumentos para pasarle al ejecutable
          args[0] = w->archivo; // Es el primero de la lista: nombre
          //args[0] = "ls";
          for(int i=1;i<w->n_args+1;i++){ // itera sobre todos los parámetros
            args[i+1] = w->params[i]; // se asigna a la lista
          }  
          args[w->n_args+1]= NULL;  // tiene que ser NULL por formato de execvp
          execvp(w->archivo, args); // recibe un ejecutable y argumentos. Reemplaza un proceso viejo por uno nuevo (se convierte en otra cosa).
        }
      
      waitpid(child_worker, &status, 0); // Espera a que los hijos terminen su proceso segun su id
      // Tiempo al que terminó
      clock_gettime(CLOCK_MONOTONIC, &end); // tiempo real de un reloj: toma el termino
      w->time = ((double)end.tv_sec + 1.0e-9*end.tv_nsec) - ((double)start.tv_sec + 1.0e-9*start.tv_nsec); // tiempo que tardó en ejecutarse
      printf("Hijo de worker demoro %.5f segundos\n", w->time); // print del tiempo

      if ( WIFEXITED(status) ) // Estado de salida, true o false, 0 o 1
      {
          w->exit_status = WEXITSTATUS(status);   // actualizas el estado de salida
          printf("Exit status of the child was %d\n", w->exit_status); // printea
      }     
      
      write_file(w);  //escribe el archivo de worker
      free_worker(w); // libera memoria usada por worker
      free(mg); // libera memoria usada por manager
      input_file_destroy(data); // destruye la info de data
      return 0; // successfull
    }   

  }
}

Manager* create_manager(Manager* mg,char*** proceso,int index){ //función que define un procesos manager: recibe la struct, la lineas del archivo y el indice del proceso
  
  mg->timeout = atoi(proceso[index][1]); // tiempo limite de ejecucion del manager (tiempo maximo)
  mg->n_args = atoi(proceso[index][2]); // cantidad de procesos hijos
  mg->index = index; // numero de linea
  return mg; // retorna el struct manager con sus atributos
}

Worker* create_worker(char*** proceso,int index){ //funcion que define un proceso worker: recibe las lineas del archivo, y el indice del proceso
  Worker* wk = malloc(sizeof(Worker)); // pide memoria para crear un proceso worker
  wk->archivo = strdup(proceso[index][1]); // The strdup() retorna un puntero a un nuevo string que es un duplicado de otro string, en este caso proceso[index][1], es decir el proceso sum, avg
  wk->index = index; // entrega indice del proceso
  wk->n_args = atoi(proceso[index][2]); // cantidad de argumentos que le quiero pasar al ejecutable
  wk->params = malloc(wk->n_args * sizeof(char*)); //pide memoria para la cantidad de n argumentos que hay que pasarle al proceso
  wk->exit_status = 0; //estado de salida: estado que retorna la funcion (return code)
  wk->time = 0; // tiempo de ejecución
  for(int i = 0; i < wk->n_args; i++) { // itera según la cantidad de argumentos del proceso
        wk->params[i] = strdup(proceso[index][3+i]); // entrega los parametros específicos que usará el proceso
      }
  return wk; //retorna el struct worker
}



int manage(Manager* mg, char*** proceso, int index){ //funcion que maneja las creaciones de los hijos?? recibe: struct manager, lineas de cada proceso, linea a ejecutar
  for (int i=0; i<mg->n_args;i++){ // itera segun la cantidad de procesos hijos que tiene
    index = atoi(proceso[mg->index][3+i]); // pasa de ASCII a integer la linea del proceso hijo que deberá leer y crear
    printf("Indice proceso a crear: %i\n",index);  // print del indice del proceso M o W que debera crear  
    char tipo = proceso[index][0][0]; //se define el tipo del proceso hijo a crear
    printf("%c tipo fork\n",tipo); // se printea el tipo
    pid_t child_pid = fork(); //se crea el proceso hijo
    if(child_pid==0){   // chequea si se creó oportunamente el hijo    
      return index; // retorna la linea del proceso hijo que se acaba de crear
    }
  }
  for (int i=0; i<mg->n_args;i++){ //itera por cantidad de procesos hijos
    wait(NULL); // espera a que los hijos terminen sus procesos
  }
  return -1; // retorna -1, terminó de iterar y esperar a los hijos
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
        exit(EXIT_FAILURE);
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


//Función que libera memoria
void free_worker(Worker* w){
  printf("Nro args a liberar: %i\n",w->n_args);

  for(int i = 0; i<w->n_args;i++){        
    free(w->params[i]); 
          } 
  free(w->params);
  free(w->archivo);
  free(w);
}