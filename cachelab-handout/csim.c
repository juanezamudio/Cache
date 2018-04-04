/**
 * A Cache Simulator that takes arbitary s, E, and b values.
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
#include <math.h>
#include "cachelab.h"

/**
 * Global vars
 */

int verbosity;
int set_bits;
int associativity;
int block_bits;
char *trace_file;
int hits;
int misses;
int evicts;

typedef struct {
  int lru;
  char valid_bit;
  unsigned long int tag;
} line;

typedef struct {
  line *lines;
} set;

typedef struct {
  set *sets;
} cache;

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
 * Command-line options processor
 * @param argv command line arguments
 */
 void process_options (int argc, char* argv[]) {
     char flagchar;
     verbosity = 0;
     set_bits = 0;
     associativity = 0;
     block_bits = 0;
     trace_file = NULL;
     while ((flagchar = getopt(argc,argv,"s:E:b:t:vh")) != -1) {
       switch (flagchar) {
         case 's':
           set_bits = atoi(optarg);
           break;
         case 'E':
           associativity = atoi(optarg);
           break;
         case 'b':
           block_bits = atoi(optarg);
           break;
         case 't':
           trace_file = optarg;
           break;
         case 'v':
           verbosity = 1;
           break;
         case 'h':
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
 * Create an empty cache with 0 values and the proper parameters
 * inputted as arguments
 * @param num_sets   The total number of sets (S = 2^s)
 * @param num_lines  The total number of lines (S * E)
 * @param num_blocks The total number of blocks (B = 2^b)
 */
cache createCache (long long num_sets, int num_lines, long long num_blocks) {
  cache new_cache;
  set new_set;
  line new_line;
  int set_index;
  int line_index;

  new_cache.sets = (set *)malloc(sizeof(set) * num_sets);

  for (set_index = 0; set_index < num_sets; set_index++) {
    new_set.lines = (line *)malloc(sizeof(line) * num_lines);

    new_cache.sets[set_index] = new_set;

    for (line_index = 0; line_index < num_lines; line_index++) {
      new_line.lru =  0;
      new_line.valid_bit = 0;
      new_line.tag = 0;
      new_set.lines[line_index] = new_line;
    }
  }

  return new_cache;
}

/**
 * Finds the empty line in a set
 * @param set_accessed the set
 */
 int findEmptyLine (set set_accessed) {

 	int num_lines = associativity;
 	int line_index;
 	line line_accessed;

 	for (line_index = 0; line_index < num_lines; line_index++) {

 		line_accessed = set_accessed.lines[line_index];

 		if (line_accessed.valid_bit == 0) {
 			return line_index;
 		}
 	}

 	return -1;
 }

/**
 * Finds the line to evict from a set
 * @param  accessed_set the set in question
 * @param  lines_used   the lines used in the set
 * @return              the min_index
 */
 int findEvictLine (set accessed_set, int *lines_used) {

  int num_lines = associativity;
	int max = accessed_set.lines[0].lru;
	int min = accessed_set.lines[0].lru;
	int min_index = 0;
	int line_index;


	//very basic loop, compare each line with max & min to decide
	for (line_index = 1; line_index < num_lines; line_index++) {

		if (min > accessed_set.lines[line_index].lru) {
			min_index = line_index;
			min = accessed_set.lines[line_index].lru;
		}

		if (max < accessed_set.lines[line_index].lru) {
			max = accessed_set.lines[line_index].lru;
		}
	}

	lines_used[0] = min;
	lines_used[1] = max;

	return min_index;
}

/**
 * Checks if the set full
 * @param new_cache the cache in question
 * @param address   the address
 */
 int checkSetFull (set accessed_set) {

   for (int i = 0; i < associativity; i++){
     if (accessed_set.lines[i].valid_bit == 0) {
       return 0;
     }
   }

   return 1;
 }

/**
 * Accesses the cache and updates the corresponding global variables with
 * the corresponding cache update values
 * @param new_cache the cache
 * @param address the address in question
 */
 void accessCache (cache new_cache, unsigned long long address) {
 		int line_index;

 		int num_lines = associativity;
 		int prev_hits = hits;

 		int tag_size = (64 - (set_bits + block_bits));

    // To get the tag, we right shift the address by (s + b) bits
 		long long tag_index = address >> (set_bits + block_bits);

    // To find the set, we left shift the address by (tag size) bits and then right
    // shift that value by the (tag size + b) bits
 		long long set_index = (address << (tag_size)) >> (tag_size + block_bits);

   	set found_set = new_cache.sets[set_index];

    int checkFull = checkSetFull(found_set); // we need to check if the set is full

 		for (line_index = 0; line_index < num_lines; line_index++) 	{

 			if (found_set.lines[line_index].valid_bit) {
 				if (found_set.lines[line_index].tag == tag_index) {

 					found_set.lines[line_index].lru += 1;
 					hits += 1;
 				}
 			}
 		}

 		if (prev_hits == hits) {
 			misses += 1;
 		} else {
      return;
    }

 		int *lines_used = (int*)malloc(sizeof(int) * 2);
 		int min_index = findEvictLine(found_set, lines_used);

    // Perform the eviction if set is full
 		if (checkFull) {
 			evicts += 1;
 			found_set.lines[min_index].tag = tag_index;
      found_set.lines[min_index].valid_bit = 1;
 			found_set.lines[min_index].lru = lines_used[1] + 1;

 		} else {
      int empty_index = findEmptyLine(found_set);
 			found_set.lines[empty_index].tag = tag_index;
 			found_set.lines[empty_index].valid_bit = 1;
 			found_set.lines[empty_index].lru = lines_used[1] + 1;
 		}

 		free(lines_used);
 }


int main(int argc, char **argv) {
  cache new_cache;

  process_options(argc, argv);

  unsigned long long num_sets = pow(2, set_bits);
  unsigned long long num_blocks = pow(2, block_bits);

  new_cache = createCache(num_sets, associativity, num_blocks);

  char inputline[100];
  char mem_type;
  unsigned long long address;
  int size;

  FILE* trace_file_pointer = fopen(trace_file, "r");

  if (!trace_file_pointer) {
      fprintf(stderr, "%s: %s\n", trace_file, strerror(errno));
      exit(1);
  }

  // Continues reading the rest of the lines in the file
  while (fgets(inputline, 100, trace_file_pointer) != NULL) {
    if (inputline[0] != 'I') {
      sscanf(inputline, " %c %llx,%d", &mem_type, &address, &size);

      switch (mem_type) {
        case 'L':
          accessCache(new_cache, address);
          break;
        case 'S':
          accessCache(new_cache, address);
          break;
        case 'M':
          accessCache(new_cache, address);
          accessCache(new_cache, address);
          break;
      }
    }
 }

 fclose(trace_file_pointer);

 printSummary(hits, misses, evicts);

 return 0;
}
