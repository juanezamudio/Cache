/**
 * A Cache Simulator that takes arbitary s, E, and b values
 *
 * Juan Zamudio - jzamudio
 * Errol Francis - efrancis
 */
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "cachelab.h"

/**
 * Command-line options processor
 * @param argv command line arguments
 */
 void process_options(int argc, char* argv[]) {
     char flagchar;
     verbosity = 0;
     set_bits = 0;
     associativity = 0;
     block_bits = 0;
     trace_file = NULL;
     while ((flagchar = getopt(argc,argv,"s:E:b:t:vh")) != -1) {
       switch (flagchar) {
         case ’s’:
           set_bits = atoi(optarg);
           break;
         case ’E’:
           associativity = atoi(optarg);
           break;
         case ’b’:
           block_bits = atoi(optarg);
           break;
         case ’t’:
           trace_file = optarg;
           break;
         case ’v’:
           verbosity = 1;
           break;
         case ’h’:
           printUsage(argv);
           exit(0);
         default:
           printUsage(argv);
           exit(1);
       }
     }

     if (set_bits == 0 ||
         associativity == 0 ||
         block_bits == 0 ||
         trace_file == NULL) {
           printf("Missing required argument.\n");
           printUsage(argv);
           exit(1);
         }
 }

/**
 * Prints usage message on how to use the cache simulator
 * @return void
 */

void printUsage (char* argv[]) {
      printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
      printf("Options:\n");
      printf("  -h         Print this help message.\n");
      printf("  -v         Optional verbose flag.\n");
      printf("  -s <num>   Number of set index bits.\n");
      printf("  -E <num>   Number of lines per set.\n");
      printf("  -b <num>   Number of block offset bits.\n");
      printf("  -t <file>  Trace file.\n");
      printf("\nExamples:\n");
      printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
      printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
}

/**
 * Trace file parser
 * @return void
 */

void traceReader (char* trace_file) {
  char inputline[100];

  FILE* trace_file_pointer = fopen(trace_file, "r");

  if (!trace_file_pointer) {
      fprintf(stderr, "%s: %s\n", trace_file, strerror(errno));
      exit(1);
  }

  while (fgets(inputline, 100, trace_file_pointer) != NULL) {
      
 }

 fclose(trace_file_pointer);
}

int main()
{
    printSummary(0, 0, 0);
    return 0;
}
