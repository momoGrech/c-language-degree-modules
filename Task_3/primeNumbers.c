#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

// struct to represent data from file
struct PrimeNumbers {
   int valueToCheck;
};

/*This struct is responsible for 
- storing values retrieved from the prime number file
- storing count of true prime numbers
- passing its true prime numbers to write
  to a file where we keep prime numbers
*/
struct Parameters {
  struct PrimeNumbers *data; 
  int start; 
  int end;
  int listOfPrimes;
  int countOfPrimes;
};

//Constant size values to assign allocated memory for the structs
const int PRIMENUMBERS_SIZE = sizeof(struct PrimeNumbers);
const int PARAMETER_SIZE = sizeof(struct Parameters);
const int BUFFER_SIZE = 1024;

//Function to check for prime numbers
void* CheckforPrimeNumbers(void *params){

  //passing the data in the parameter to the struct primeNumbers
  struct PrimeNumbers *data = ((struct Parameters*)params)->data;
  
  //identifying the start and the end point of the array
  int start = ((struct Parameters*)params)->start;
  int end = ((struct Parameters*)params)->end;

  int total = 0;
  int counter = 0;
  int arrSize = 1;
  
  //Assigning memory allocation to parameter struct in order to store true prime numbers
  struct Parameters *truePrimes = (struct Parameters*)calloc(arrSize, PARAMETER_SIZE);

  //Loop throught a batch of numbers passed by the parameter called "params"
  for (int index = start; index <= end; index++)
  {
      //deafult setup is that a value is prime number
      int prime = 1; 

      //Assigning a temporarily value with the current value in the file
      int numToCheck = data[index].valueToCheck;

      //loop throw the number and break if value is not a prime number
      for (int i = 2; i <= sqrt(numToCheck); i++) {
         // condition for non-prime
         if (numToCheck % i == 0) {
            prime = 0;
            break;
         }
      }

      //Condition to check if value is less than or equal to 1, and if so, it's not prime number
      if (numToCheck <= 1){
         prime = 0;
      }
      
      // Prime numbers are stored in the struct variable as to be used later
      if (prime == 1){
         truePrimes[counter].listOfPrimes = data[index].valueToCheck;

         //Incrementing the total prime numbers for that particular batch
         total += 1;
         //Move to the next position in the array
         counter++;
      }
      
      //Using memory reallocation if we need to store more values in the struct
      if (total >= arrSize)
      {
         arrSize++;
         truePrimes = (struct Parameters*)realloc(truePrimes, arrSize*PARAMETER_SIZE);
      }
   }

   //Update the count of prime numbers with the total
   truePrimes[0].countOfPrimes = total;

   /*return truePrimes which will be used in 
   Pthread_join */
   pthread_exit(truePrimes);
}

// A function that when we give it a file it will give us back a list of numbers
struct PrimeNumbers* getDataFromFile(FILE *filePath, int *length){
    
   //Assigning memory allocation to PrimeNumbers struct in order to store the values in the file
   struct PrimeNumbers *collectedData = (struct PrimeNumbers*)calloc(1,PRIMENUMBERS_SIZE);

   //Open the file (read)
   FILE *file = (FILE*)filePath;
   
   //Using memory allocation for the buffer which is called line
   char *line = calloc(BUFFER_SIZE, sizeof(char));

   /*while we have data in the file,
   proceed with extracting them and pass them to a struct*/
   while (fgets(line, BUFFER_SIZE, file))
   {
      // temporary struct to store (values to check) from the buffer
      struct PrimeNumbers temp_struct;
      sscanf(line, "%d\n", &temp_struct.valueToCheck);

      // Pass the data from the temporary struct to collectedData which will be returned
      collectedData[*length] = temp_struct;

      (*length)++;

      //Re-size the collected data in order to store more values
      collectedData = realloc(collectedData, PRIMENUMBERS_SIZE * (*length+1));
   }
   free(line);
   return collectedData;
}

 // Function to split the length of the iteration based on the number of threads
struct Parameters *splitFileLength(int threads, void *paramStruct){

   int start_at = 0;
   int tempoStart = 0;
   int end_at = 0;
   int tempoEnd = 0;
   int index = 0;
   int arrSize = 1;

   // This temporarily struct was used to pass the value of the current split to the main
   struct Parameters *temporaryPar = calloc(arrSize, PARAMETER_SIZE);

   temporaryPar->data = ((struct Parameters*)paramStruct)->data;
   int end = ((struct Parameters*)paramStruct)->end;
   int start = ((struct Parameters*)paramStruct)->start;

   //loop throught the length of the file and split it according to the number of the threads
   while (threads)
   {
      end_at = end / threads;

      /*Dedicated for the struct*/
      tempoStart = start_at;
      tempoEnd = start_at + end_at-1;

      //Pass the values of start, end and the list of primes to the struct 
      temporaryPar[index].start = tempoStart;
      temporaryPar[index].end = tempoEnd;
      temporaryPar[index].data = temporaryPar->data;

      //Expanding the size of the struct in order to pass more values of start to end
      arrSize++;
      //Dynamically allocate memory to the struct
      temporaryPar = (struct Parameters*)realloc(temporaryPar, arrSize*PARAMETER_SIZE);

      // Just to display my work (for testing purposes)
      printf("\nstarting at: %d\t ending at: %d\n", tempoStart, tempoEnd);

      start_at += end_at;
      end -= end_at;
      
      threads -= 1;
      index++;
   }
   return temporaryPar;
}

int main(int argc, char* argv[]) {

   //Temporarily thread number
   int threadsNum;

   //Confirm if user is inserting a number
   if (argc < 2)
   {
      printf("You need to insert a number or a file name\n");
      return EXIT_FAILURE;
   }

   //Confirm that user is inserting positive number
   if(argv[1] <= 0){
      printf("inserted argument must be positive\n");
      return EXIT_FAILURE;
   }
   else{
      //Converting char to int
      int toThreads = strtol(argv[1], NULL, 10);
      
      // Number of threadsa will be used as a parameter in the split Iteration function
      threadsNum = toThreads;
   }

   //Create file variable to store the file name from the comandline
   FILE *file_ptr;

   // get file path from the argument
   file_ptr = fopen(argv[2], "r");

   // Error handling if file_ptr returned null
   if(file_ptr == NULL)
   {
      printf("Cannot read file.\n");
      return EXIT_FAILURE;
   }

   //Allocate memory for length of the file
   int *length = calloc(1, sizeof(int));

   /* 1) read/open the file */
   struct PrimeNumbers *loadedData = getDataFromFile(file_ptr, length);
   fclose(file_ptr);
   printf("total lines are : %d\n", *length);

   //Constructing Paramaters
   struct Parameters params;
   //Pass loaded data from Primenumbers struct to the Parameter struct
   params.data = loadedData;
   params.start = 0;
   params.end = *length;

   /* 2) split the file */
   struct Parameters *splitedParam = splitFileLength(threadsNum, &params);

   /* 3) Multi_threading */
   // Allocate memory for thread numbers
   pthread_t *threads = calloc(threadsNum, sizeof(pthread_t));

   /* for each thread call CheckforPrimeNumbers function and 
      pass splitedParam at the number of the thread's position position
   */
   for(int i = 0; i < threadsNum; i++)
   {
      pthread_create(&threads[i], NULL, (void *)CheckforPrimeNumbers, &splitedParam[i]);
   }

   //Increment the total counts of primes for each batch
   int count_primes = 0;

   /* struct to store results and assigning memory allocation */
   struct Parameters *primeResult = (struct Parameters*)calloc(1, PARAMETER_SIZE);

   // Using pthread_joint to suspend the execution of the calling thread until the target thread terminates
   for(int index = 0; index< threadsNum; index++)
   {
      /* primeResult is a pointer to a pointer */
      void ** result_ptr = (void **) &primeResult;

      pthread_join(threads[index], result_ptr);

      /* Display results */
      count_primes += primeResult->countOfPrimes;

      //storing the prime numbers in a file
      char buffer_out[1024];
      FILE *fileToStoreData = fopen("Task_3/filteredPrimeNumbers.txt", "a");
      
      //Check if the file is opened successfully
      if (fileToStoreData == NULL)
      {
         printf("Error: file cannot be opened");
         exit(-1);
      }
      else{
         printf("file opened successfully\n");
      }

      for (int i = 0; i < primeResult->countOfPrimes; i++)
      {
         //Loop through the prime numbers and display them
         printf("prime numbers %d\n", primeResult[i].listOfPrimes);

         //Store the prime numbers in the output file
         fprintf(fileToStoreData, "%d\n",primeResult[i].listOfPrimes);
      }

      //Closing the output file after storing the prime numbers
      fclose(fileToStoreData);

      printf("Batch %d : Total lines for prime numbers are are : %d\n",index+1 , primeResult->countOfPrimes);
   }

   primeResult->countOfPrimes = count_primes;
   printf("\nTotal counts of prime numbers : %d\n", primeResult->countOfPrimes);

   // Free all the allocated memories
   free(splitedParam);
   free(primeResult);
   free(length);
   free(threads);
   free(loadedData);

   return EXIT_SUCCESS;
}

// clear; gcc Task_3/primeNumbers.c -lm -pthread; ./a.out 4 Task_3/PrimeData1.txt; rm ./a.out;


