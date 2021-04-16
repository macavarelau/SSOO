
// Tells the compiler to compile this file once
#pragma once

// Define compile-time constants
#define MAX_SPLIT 255
#define BUFFER_SIZE 4096

// Define the struct
typedef struct inputfile {
  int len;
  char*** lines;  // This is an array of arrays of strings
} InputFile;

typedef struct inputfilechild {
  int len;
  int count;
  char** lines;  // This is an array of strings
} InputFileChild;


// Declare functions
InputFile* read_file(char* filename);
InputFileChild* read_file_child(char* filename);
void input_file_destroy(InputFile* input_file);
void input_file_destroy_child(InputFileChild* input_file_child);