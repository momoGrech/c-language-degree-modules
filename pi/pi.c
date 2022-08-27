#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

//Struct to control the legth of the iterations and split them
struct PIValue{
   double PILength;
   int PI_start;
   int PI_end;
};

// Struct to hold the total value of PI at particulare batch
struct Parameters {
   double total; 
   int start; 
   int end;
};

//Constant size values to assign allocated memory for the structs
const int PIVALUE_SIZE = sizeof(struct PIValue);
const int PARAMETER_SIZE = sizeof(struct Parameters);

// calculate PI
void* calculatePI(void *tempoPI){

   // these variables are used to hold value of PI temporarily and print them
   double getTotal;
   double temporaryValue; 

   //extracting start and end values from the void parameter
   int start = ((struct Parameters*)tempoPI)->start;
   int end = ((struct Parameters*)tempoPI)->end;

   //Loop through the batch from start to end do the calculation to for PI
   for(int i = start; i <= end; i++)
   {
      temporaryValue = pow(-1, i) / (2 * i + 1);
      getTotal += temporaryValue;
      printf("Calculated PI at %d is: %.10f\n", i, getTotal);
   }

   //Passing the total value of PI to the tempoPI struct so that in the main we can sum up the number of threads
   ((struct Parameters*)tempoPI)->total = getTotal;
}

// Function to split the length of the iteration based on the number of threads
struct Parameters *splitFileLength(int threads, int iterations){

   int start = 0;
   int end = 0;
   int index = 0;
   int arrSize = 1;
   int temp_start = 0;
   int temp_end = 0;

   // This struct was used to be returned and use its values of the current split in the main
   struct Parameters *temporaryPar = calloc(arrSize, PARAMETER_SIZE);

   // for each thread, split the length equally
   while (threads)
   {
      end = iterations / threads;

      /*Dedicated for the struct*/
      temp_start = start;
      temp_end = start + end-1;

      // Pass the values of starting point and ending point to the struct 
      temporaryPar[index].start = temp_start;
      temporaryPar[index].end = temp_end;

      //Expanding the size of the struct in order to pass more values of start to end
      arrSize++;
      //Dynamically allocate memory to the struct
      temporaryPar = (struct Parameters*)realloc(temporaryPar, arrSize*PARAMETER_SIZE);

      // Just to demonstrate my work (for testing purposes)
      printf("\nstarting at: %d\t ending at: %d\n", temp_start, temp_end);
   
      start += end;
      iterations -= end;
      threads -= 1;
      index++;
   }
   return temporaryPar;
}

int main(int argc, char* argv[]){

   // Variables to store iterations and thread numbers
   int iteration;
   int threadsNum;

   //Confirm if user is inserting a number
   if (argc < 2)
   {
      printf("You need to insert a number\n");
      return EXIT_FAILURE;
   }

   //Confirm that user is inserting positive number
   if(argv[1] <= 0 || argv[2] <= 0){
      printf("inserted argument must be positive\n");
      return EXIT_FAILURE;
   }
   else{
      //Converting char to int
      int toIterate = strtol(argv[1], NULL, 10);
      int toThreads = strtol(argv[2], NULL, 10);
      
      //Iteration and number of threadsa will be used as a parameter in the split Iteration function
      iteration = toIterate;
      threadsNum = toThreads;
   }
   
   //calling split Iteration and passing its values to the param struct 
   struct Parameters *splitted_param = splitFileLength(threadsNum, iteration);

   //multithreading
   pthread_t *threads = calloc(threadsNum, sizeof(pthread_t));
   
   //Looping through the number of threads
   for (int i = 0; i < threadsNum; i++)
   {
      pthread_create(&threads[i], NULL, (void *)calculatePI, &splitted_param[i]);
   }
   
   //Temporary variable to hold the value of PI
   double tempo = 0;
   
   //Joining the threads and sum them up
   for (int i = 0; i < threadsNum; i++)
   {
      pthread_join(threads[i],NULL);
      tempo += splitted_param[i].total;
      splitted_param->total = tempo;
   }
   
   printf("\nValue of PI is %.10f\n", splitted_param->total *4);
   
   //Free the memory dedicated for threads and splitted param struct
   free(splitted_param);
   free(threads);
   
   return EXIT_SUCCESS;
}

// clear; gcc Task_2/pi.c -lm -pthread; ./a.out 1000 3; rm ./a.out;