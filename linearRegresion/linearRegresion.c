#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>

/*1*/
// struct to store value of X and Y from the provided txt file
struct LinearFacts {
   float value_x, value_y;
};

// Struct to store file paths
struct FilePaths
{
   char filepath[PATH_MAX];
};

// struct to store the value of linear claculation
struct CalculatedLinearFacts {
   float sumOfX, sumOfY, sumOfXY, powerOfX, value_A, value_B, finalValue_X, finalValue_Y;
};

//Constant values that can be re-used multiple times
const int LINEARFACT_SIZE = sizeof(struct LinearFacts);
const int CALCULATEDLINEARFACT_SIZE = sizeof(struct CalculatedLinearFacts);
const int BUFFER_SIZE = 1024;

/*2*/
// function that when we give a file it will give us back a list of LinearFactss
struct LinearFacts* getDataFromFile(struct FilePaths *filepaths, int *num_files, int *length){
    
   // Allocate memory to a Struct which stores value of x and y then to be returned at the end of function
   struct LinearFacts *collectedData = calloc(1,LINEARFACT_SIZE);

   //Loop through the number of files which is specified as an argument when running the application
   for(int i = 0; i < *num_files; i++)
   {
      //Open and read a file at an index from filepath
      FILE *file = fopen(filepaths[i].filepath, "r");

      //the If statement is for error handling
      if (file != NULL)
      {
         // allocating memory to a buffer type of char
         char *line = calloc(BUFFER_SIZE, sizeof(char));

         //running through the file until the end of it
         while (!feof (file))
         {
            fgets(line, BUFFER_SIZE, file);
            //temporary stuct to store value of x and y
            struct LinearFacts temporaryStruct;

            // pass the values of X and Y from the buffer to the temprary struct
            sscanf(line, "%f,%f\n", &temporaryStruct.value_x, &temporaryStruct.value_y);

            //assign main  struct to the current data in the temporary struct
            collectedData[*length] = temporaryStruct;

            (*length)++;

            // resizing the main struct in order to allow store more data
            collectedData = realloc(collectedData, LINEARFACT_SIZE * (*length+1));
         }

         fclose(file);

         free(line);
      } 
      else
      {
      return NULL;
      }
   }
   return collectedData;
}



/*3*/
// Function to accept the facts as a parameter and do the calculation
struct CalculatedLinearFacts* getValueOfBandA(struct LinearFacts *collectedData, int length){

   //struct to store the calculated values and return them at the end of the struct
   struct CalculatedLinearFacts *calculatedFacts = calloc (1,CALCULATEDLINEARFACT_SIZE);
   
   // temporary variables to hold the values during the calculation
   float sumX = 0;
   float sumY = 0;
   float sumXY = 0;
   float sumXPow2 = 0;

   //Looping throught the file to proceed with the calculation
   for (int i = 0; i < length; i++)
   {
      sumX += collectedData[i].value_x;
      sumY = sumY + collectedData[i].value_y;
      sumXY += collectedData[i].value_x * collectedData[i].value_y;
      sumXPow2 += pow(collectedData[i].value_x, 2);
   }
   // Pass the temprary values to the struct
   calculatedFacts->sumOfX = sumX;
   calculatedFacts->sumOfY = sumY;
   calculatedFacts->sumOfXY = sumXY;
   calculatedFacts->powerOfX = sumXPow2;
   
   //Calculate the Equation of A and B
   calculatedFacts->value_B = ((length * sumXY) - sumX * sumY) / ((length*sumXPow2) - sumX * sumX);
   calculatedFacts->value_A = ((sumY * sumXPow2) - (sumX * sumXY)) / ((length * sumXPow2) - pow(sumX, 2));
   
   return calculatedFacts;
}


int main(int argc, char *argv[]){

   // identify the number of files being passed as an argument
   int num_files = argc - 1;

   // Allocate memory to filepath struct
   struct FilePaths *filepaths = (struct FilePaths*)malloc(num_files * sizeof(struct FilePaths));  

   // based on the number of files, pass each path(file name) to the struct of file path
   for(int i = 0; i < num_files; i++)
   {
      strcpy(filepaths[i].filepath, argv[i+1]);
   }

   // allocate memory to the length which identify the length of the file
   int *length = calloc(1, sizeof(int));

   // call the fuction which get the data from each file and pass it to data struct   
   struct LinearFacts* data = getDataFromFile(filepaths, &num_files, length);
   // Error handling
   if(data == NULL)
   {
      printf("One of the supplied file paths is not correct. Please review arguments and try again\n");
      exit(EXIT_FAILURE);
   }
    
   printf("Total data collected; %d\n", *length);

   //Calling the function which calculate the values of B and A then passing it through a struct
   struct CalculatedLinearFacts* calculatedData = getValueOfBandA(data, *length);

   printf("Value of A: is %.2f\n", calculatedData->value_A);
   printf("Value of B: is %.2f\n\n", calculatedData->value_B);

   printf("The equation is: Y = %.2fx + %.2f\n", calculatedData->value_B, calculatedData->value_A);

   //Asking user to provide value of X
   printf("\nPlease insert value of X in order to calculate Y : ");
   scanf("%f", &calculatedData->finalValue_X);

   // Calculate the value of Y
   calculatedData->finalValue_Y = (calculatedData->value_B * calculatedData->finalValue_X) + calculatedData->value_A;

   printf("\nValue of Y is : %.2f\n\n", calculatedData->finalValue_Y);

   //Displaying the iteration is just for demonstration purposes
   for (int i = 0; i < *length; i++)
   {
      printf("Value of X: %.2f\tValue of Y: %.2f\n",
         data[i].value_x,
         data[i].value_y);
   }
   
   free(calculatedData);
   free(filepaths);
   free(length);
   free(data);
   
   return EXIT_SUCCESS; 
}



// clear; gcc Task_1/linearRegresion.c -lm; ./a.out Task_1/datasetLR1.txt Task_1/datasetLR2.txt Task_1/datasetLR3.txt Task_1/datasetLR4.txt; rm ./a.out;