#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>

// struct to represent data from file
struct DataToLoad {
   double valueToCheck;
};

/*This struct is storing the start and end of each batch of array*/
struct ArrayOfValues {
  int start; 
  int end;
};

//Constant size to assign allocated memory for the structs
const int DATATOLOAD_SIZE = sizeof(struct DataToLoad);
const int ARRAYOFVALUES_SIZE = sizeof(struct ArrayOfValues);

// A function that when we give it a file it will give us back a list of numbers
struct DataToLoad* getDataFromFile(FILE *filePath, int *length){
    
   //Assigning memory allocation to DataToLoad struct in order to store the values in the file
   struct DataToLoad *collectedData = (struct DataToLoad*)calloc(1, DATATOLOAD_SIZE);

   FILE *file = (FILE*)filePath;

   /*while we have data in the file,
   proceed with extracting them and store them in a struct*/
   while (fscanf(file, "%lf,\n", &collectedData[*length].valueToCheck) != EOF)
   {  
      //increment the length by 1 on every loop
      (*length)++;
      //Re-size the memory of the struct in order to store more values
      collectedData = realloc(collectedData, DATATOLOAD_SIZE * (*length+1));
   }
   return collectedData;
}

// Function to split the length of the array based on the number of batches
struct ArrayOfValues *splitFileLength(int threads, int char_count){

   int start_at = 0;
   int tempoStart = 0;
   int end_at = 0;
   int tempoEnd = 0;
   int index = 0;
   int arrSize = 1;

   int end = char_count;
   int start = 0;

   //allocate memory to the struct
   struct ArrayOfValues *arrayOfValues = (struct ArrayOfValues*)calloc(arrSize, ARRAYOFVALUES_SIZE);

   //loop throught the length of the file and split it according to the number of the threads
   while (threads)
   {
      end_at = end / threads;

      /*Dedicated for the struct*/
      tempoStart = start_at;
      tempoEnd = start_at + end_at-1;

      //Pass the values of start, end and the list of primes to the struct 
      arrayOfValues[index].start = tempoStart;
      arrayOfValues[index].end = tempoEnd;
      
      int tmp_index = 0;
      int resizeNum = 1;
      
      //Expanding the size of the struct in order to pass more values of start to end
      arrSize++;

      //Dynamically allocate memory to the struct
      arrayOfValues = (struct ArrayOfValues*)realloc(arrayOfValues, arrSize*ARRAYOFVALUES_SIZE);

      start_at += end_at;
      end -= end_at;
      
      threads -= 1;
      index++;
   }
   return arrayOfValues;
}

//Function to return the total values
double *getTotal(void *array, int length){
   //using memory allocation to store total value
   double *total = calloc(sizeof(double), 1);
   *total = 0;
   double *batchOfData = ((double*)array);
   //loop through the length of the batch
   for (int i = 0; i < length; i++)
   {  
      //use subtotal to store each value from the bacth
      double subTotal = batchOfData[i];
      //do the addition
      *total += subTotal;
   }
   return total;
}

//Function to return the average values
double *getAverage(double *sum, int length){
   //using memory allocation to store average value
   double *average = malloc(sizeof(double) * 1);

   double *subTotal = ((double*)sum);
   //divide the total to the length
   *average = *subTotal / length;
   return average;
}

//Function to return the largest negative numbers
double *getMaxNegative(void *array, int length) {
   //using memory allocation to store the largest negative value
   double *largestNegative = calloc(sizeof(double), 1);

   double *batchOfData = ((double*)array);

   *largestNegative = 0;
   //loop through the length of the batch
   for (int i = 0; i < length; i++) {
      //find the largest negative number
      if (batchOfData[i] < 0 && batchOfData[i] < *largestNegative) {
         *largestNegative = batchOfData[i];
      }
   }
   return largestNegative;
}

//Function to return the largest positive numbers
double  *getMaxPositive(void *array, int length) {
   //using memory allocation to store the largest positive value
   double *largestPositive = calloc(sizeof(double), 1);
   double *batchOfData = ((double*)array);
   *largestPositive = 0;
   //loop through the length of the batch
   for (int i = 0; i < length; i++) {
      //find the largest positive number
      if (batchOfData[i] > 0 && batchOfData[i] > *largestPositive) {
         *largestPositive = batchOfData[i];
      }
   }
   return largestPositive;
}

int main(int argc, char* argv[]) {
   //open MPI
   MPI_Init(&argc, &argv);

   /* rank: is a number like an ID to differentiate the processes
      size: the total number of processes
   */
   int rank, size;
   MPI_Status status;

   // Get the rank of the current process
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   // Get the size of the process
   MPI_Comm_size(MPI_COMM_WORLD, &size); 

   if (size < 2 || size > 10)
   {
      printf("\nThe size of the processor must be larger than 1 and equal or less than 10. Please review the size in the comand line.\n");
      return 0;
   }
   
   //number of batches (-1 is refering to the ROOT, rank 0)
   int batches = size -1;

   //Aloocate memory to the below variables in order to recieve and store values from each batch
   double *subTotal = malloc(sizeof(double)* 1);
   double *total = malloc(sizeof(double)* 1);
   double *subMaxNegative = malloc(sizeof(double)* 1);
   double *maxNegative = malloc(sizeof(double)* 1);
   double *subMaxPositive = malloc(sizeof(double)* 1);
   double *maxPositive = malloc(sizeof(double)* 1);
   double *subAverage = malloc(sizeof(double)* 1);
   double *average = malloc(sizeof(double)* 1);

   *maxNegative = 0;
   *maxPositive = 0;
   *total = 0;
   *average = 0;

   //The size of a batch
   int *sizeOfOneBatch = malloc(sizeof(int) * 1);

   // File pointer
   FILE *file_ptr;
   // Open the file and pass the stream to the file pointer
   file_ptr = fopen("NumbersForMPI.txt", "r");

   // Error handling if file_ptr returned null
   if(file_ptr == NULL)
   {
      printf("Cannot read file.\n");
      return EXIT_FAILURE;
   }

   //Allocate memory for length of the file
   int *length = calloc(1, sizeof(int));

   //rank 0 is the Root processor
   if(rank == 0)
   {
      // Read the file
      struct DataToLoad *loadedData = getDataFromFile(file_ptr, length);
      fclose(file_ptr);
   
      // Split the array length
      struct ArrayOfValues *arrayBatches = splitFileLength(batches, *length);

      //get the size of one batch
      *sizeOfOneBatch = *length/batches;

      //This loop get each batch and send them to each process
      for (int batch = 0; batch < batches; batch++){
         // allocate memory to batchOfNumbers to store numbers and use it in the send MPI
         double *batchOfNumbers = calloc(sizeof(double), *length);

         int start = arrayBatches[batch].start;
         int end = arrayBatches[batch].end;

         //reset is used to determine the length of the batch
         int reset = 1;
         //based on the length from start to end, pass values to batchOfNumbers
         for (int index = start; index < end+1; index++)
         {
            batchOfNumbers[reset] = loadedData[index].valueToCheck;
            reset++;
         }
         //The exact size of a batch
         *sizeOfOneBatch = reset;

         //send the size of the current batch and the total length of the array
         int tag = *length;
         MPI_Send(sizeOfOneBatch, 1, MPI_INT, batch+1, tag, MPI_COMM_WORLD);

         //send the data associated to the current batch
         MPI_Send(batchOfNumbers, *sizeOfOneBatch, MPI_DOUBLE, batch+1, 100, MPI_COMM_WORLD);
         free(batchOfNumbers);
      }

      //this loop is used to receive the result of each bach from the processes
      for (int batch = 0; batch < batches; batch++)
      {
         //receive batches of subtotals
         MPI_Recv(subTotal, 1, MPI_DOUBLE, batch+1, 101, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
         *total += *subTotal;

         // Get subaverage
         MPI_Recv(subAverage, 1, MPI_DOUBLE, batch+1, 102, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
         *average += *subAverage;

         //receive batches of largest negative values
         MPI_Recv(subMaxNegative, 1, MPI_DOUBLE, batch+1, 103, MPI_COMM_WORLD,MPI_STATUS_IGNORE);

         //receive batches of largest positive values
         MPI_Recv(subMaxPositive, 1, MPI_DOUBLE, batch+1, 104, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
          
         //find the largest negative from the subMaxNegative
         if (*maxNegative > *subMaxNegative)
         {
            *maxNegative = *subMaxNegative;
         }

         //find the largest positive from the subMaxPositive
         if (*maxPositive < *subMaxPositive)
         {
            *maxPositive = *subMaxPositive;
         }
      }
      //Print out the result
      printf("\nTOTAL: %.6f\n", *total);
      printf("\nAVERAGE: %.6f\n", *average);
      printf("\nLARGEST NEGATIVE: %.6f\n", *maxNegative);
      printf("\nLARGEST POSITIVE: %.6f\n", *maxPositive);

      //free the allocated memories dedicated for the above send and recieve variables
      free(total);
      free(maxNegative);
      free(subMaxNegative);
      free(subTotal);
      free(subMaxPositive);
      free(maxPositive);
      free(subAverage);
      free(average);
      free(sizeOfOneBatch);
   }
   else{
      //Alocate memory to the size of a batch which will be recieved from the ROOT
      int *rcv_batch_size = malloc(sizeof(int)* 1);

      //recieve the batch size and the array length
      MPI_Recv(rcv_batch_size, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      //array length will be used in the average function
      int arrayLength = status.MPI_TAG;

      //allocate memory to a vraible based on the size of the above received batch size
      double *batch_number = calloc(sizeof(double), *rcv_batch_size);
      //recieve the batch of numbers and pass them to the functions as a parameter
      MPI_Recv(batch_number, *rcv_batch_size, MPI_DOUBLE, 0, 100, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

      //call the below functions with the above data and size as parameters
      // 1-Total
      double *subTotal = getTotal(batch_number, *rcv_batch_size);      
      //Send the subtotal values to the ROOT(rank 0)
      MPI_Send(subTotal, 1, MPI_DOUBLE, 0, 101, MPI_COMM_WORLD);

      // 2-Average
      /* The logic for this average is:
         to divide each batch to the total length of the array
         return the result of the division and sum them up in the ROOT
      */ 
      double *subAverage = getAverage(subTotal, arrayLength);
      //Send the average values to the ROOT(rank 0)
      MPI_Send(subAverage, 1, MPI_DOUBLE, 0, 102, MPI_COMM_WORLD);

      // 3-The largest negative number
      double *subMaxNegative = getMaxNegative(batch_number, *rcv_batch_size);
      //Send the max negative values to the ROOT(rank 0)
      MPI_Send(subMaxNegative, 1, MPI_DOUBLE, 0, 103, MPI_COMM_WORLD);

      // 4- The largest positive number 
      double *subMaxPositive = getMaxPositive(batch_number, *rcv_batch_size);
      //Send the max positive values to the ROOT(rank 0)
      MPI_Send(subMaxPositive, 1, MPI_DOUBLE, 0, 104, MPI_COMM_WORLD);

      /*free the allocated memories dedicated for:
         the above send and recieve variables
         the memories allocated in the functions
       */
      free(rcv_batch_size);
      free(batch_number);
      free(subTotal);
      free(maxNegative);
      free(maxPositive);
      free(subAverage);
   }
   //Close MPI-cleans up all state related to MPI
   MPI_Finalize();

   free(length);
   return 0;
}

//clear; mpicc task2.c -o task2; mpiexec -n 8 -oversubscribe ./task2; rm task2;
