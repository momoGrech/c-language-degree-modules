#include <stdio.h>
#include <stdlib.h>
#include "lodepng.h"
#include <math.h>

//Struct to strore the average values of RGB
struct RGBValues
{
	unsigned char reds;
	unsigned char greens;
	unsigned char blues;
};

//struct to store starting and ending point of RGB values
struct imageDetails
{
	unsigned char* image;
   int start;
   int end;
};

//Constant size values to assign allocated memory for the structs
const int RGBVALUES_SIZE = sizeof(struct RGBValues*);
const int IMAGE_DETAILS_SIZE = sizeof(struct imageDetails);

//Function to return values of RGB from 1D of array
struct RGBValues **getValueOfRGB(struct imageDetails *imgParam, int height, int width, int threads){

   // counter to identify RGB positions
	int counter = 0;

	//Allocate momery and create 2D array to store RGB and to be returned
   struct RGBValues **rgb_colors = (struct RGBValues**)calloc(height, RGBVALUES_SIZE);

   // Based on the height of the image, allocate momery to each row
   for (int row=0; row < height; row++){
      rgb_colors[row] = (struct RGBValues*)calloc(width, RGBVALUES_SIZE*4);
   }
   
   int start = 0;
   int end = 0;

   // Looping through the threads to get each batch from the parameter array of RGB
   for (int i = 0; i < threads; i++)
   {
      // get the image, starting and ending points from the parameter called imgParam
      unsigned char *imgData = ((struct imageDetails*)imgParam)[i].image;
      start = ((struct imageDetails*)imgParam)[i].start;
      end = ((struct imageDetails*)imgParam)[i].end;

      for(int row=start; row <= end; row++){
         for(int col=0; col<width*4; col=col+4){  
            //converting ID of array to 2D of array
            rgb_colors[row][col].reds = imgData[counter];
            rgb_colors[row][col].greens = imgData[counter+1];
            rgb_colors[row][col].blues = imgData[counter+2];
            counter +=4;
         }
      }
   }
   //return the values of RGB
	return rgb_colors;
}

// This function is responsible for bluring an image 
struct RGBValues ** blurImage(struct RGBValues** averageOfRGB, int height, int width){
    
   // Allocate memory to a new struct which stores Blured Pixeles and will be returned
   struct RGBValues **blurred_RGB = (struct RGBValues**)malloc(height * sizeof(struct RGBValues*));

   // Based on the height of the image, allocate momery to each row
   for (int row=0; row < height; row++){
      blurred_RGB[row] = (struct RGBValues*)malloc(width * sizeof(struct RGBValues)*4);
   }

	// loop through the image parameter and pass its RGB values to the new struct 
   for(int row=0; row < height; row++){
		for(int col=0; col<width*4; col=col+4){
			blurred_RGB[row][col].reds = averageOfRGB[row][col].reds;
			blurred_RGB[row][col].greens = averageOfRGB[row][col].greens;
			blurred_RGB[row][col].blues = averageOfRGB[row][col].blues;
		}
	}

   //variables to store avergae for the total of R-G-B
   int avg_Red;
   int avg_Green;
   int avg_Blue;
   
   // Create a temporary struct to store the RGB values after being blured then pass its values to blurred_RGB struct
   struct RGBValues **temporary_RGB = (struct RGBValues**)malloc(height * sizeof(struct RGBValues*));

   // Based on the height of the image, allocate momery to each row
   for (int row=0; row < height; row++){
      temporary_RGB[row] = (struct RGBValues*)malloc(width * sizeof(struct RGBValues)*4);
   }

   /* -Loop through each pixeles
      -Sum up each RGB
      -Divide each RGB by its around pixeles to get the average
      -Take account for the pixeles located at the corneres and the edges
      -Pixeles at edges are divided by 6, Pixeles at corners are divided by 4 and the rest of the piceles are divided by 9
   */
   for (int row = 0; row < height; row++)
   {
      for (int col= 0; col< width*4; col=col+4)
      {
         avg_Blue = 0;
         avg_Green = 0;
         avg_Red = 0;
         //Top col
         if (row== 0)
         {
            if (col == 0) //top left corner
            {
               avg_Blue = round(ceil(blurred_RGB[row][col].blues
                                    + blurred_RGB[row][col + 4].blues
                                    + blurred_RGB[row+ 1][col].blues
                                    + blurred_RGB[row+ 1][col + 4].blues) / 4);
               avg_Green = round(ceil(blurred_RGB[row][col].greens
                                    + blurred_RGB[row][col + 4].greens
                                    +  blurred_RGB[row+ 1][col].greens
                                    +  blurred_RGB[row+ 1][col + 4].greens) / 4);
               avg_Red = round(ceil(blurred_RGB[row][col].reds
                                    + blurred_RGB[row][col + 4].reds
                                    + blurred_RGB[row+ 1][col].reds
                                    + blurred_RGB[row+ 1][col + 4].reds) / 4);

            }
            else if (col == (width*4) - 4) //top right corner
            {
               avg_Blue = round(ceil(blurred_RGB[row][col].blues
                                    + blurred_RGB[row][col - 4].blues
                                    + blurred_RGB[row+ 1][col].blues
                                    + blurred_RGB[row+ 1][col - 4].blues) / 4);
               avg_Green = round(ceil(blurred_RGB[row][col].greens
                                    + blurred_RGB[row][col - 4].greens
                                    + blurred_RGB[row+ 1][col].greens
                                    + blurred_RGB[row+ 1][col - 4].greens) / 4);
               avg_Red = round(ceil(blurred_RGB[row][col].reds
                                    + blurred_RGB[row][col - 4].reds
                                    + blurred_RGB[row+ 1][col].reds
                                    + blurred_RGB[row+ 1][col - 4].reds) / 4);
            }
            else //top middle pixeles
            {
               avg_Blue = round(ceil(blurred_RGB[row][col].blues
                                    + blurred_RGB[row][col - 4].blues
                                    + blurred_RGB[row][col + 4].blues
                                    + blurred_RGB[row+ 1][col].blues
                                    + blurred_RGB[row+ 1][col - 4].blues
                                    + blurred_RGB[row+ 1][col + 4].blues) / 6);
               avg_Green = round(ceil(blurred_RGB[row][col].greens
                                    + blurred_RGB[row][col - 4].greens
                                    + blurred_RGB[row][col + 4].greens
                                    + blurred_RGB[row+ 1][col].greens
                                    + blurred_RGB[row+ 1][col - 4].greens
                                    + blurred_RGB[row+ 1][col + 4].greens) / 6);
               avg_Red = round(ceil(blurred_RGB[row][col].reds
                                    + blurred_RGB[row][col - 4].reds
                                    + blurred_RGB[row][col + 4].reds
                                    + blurred_RGB[row+ 1][col].reds
                                    + blurred_RGB[row+ 1][col - 4].reds
                                    + blurred_RGB[row+ 1][col + 4].reds) / 6);
            }
         }
         //Bottom
         else if (row== height - 1)
         {
            if (col == 0) //Bottom left corner
            {
               avg_Blue = round(ceil(blurred_RGB[row][col].blues
                                    + blurred_RGB[row][col + 4].blues
                                    + blurred_RGB[row- 1][col].blues
                                    + blurred_RGB[row- 1][col + 4].blues) / 4);
               avg_Green = round(ceil(blurred_RGB[row][col].greens
                                    + blurred_RGB[row][col + 4].greens
                                    + blurred_RGB[row- 1][col].greens
                                    + blurred_RGB[row- 1][col + 4].greens) / 4);
               avg_Red = round(ceil(blurred_RGB[row][col].reds
                                    + blurred_RGB[row][col + 4].reds
                                    + blurred_RGB[row- 1][col].reds
                                    + blurred_RGB[row- 1][col + 4].reds) / 4);
            }

            else if (col == (width*4) - 4) //Bottom right corner (last pixel)
            {
               avg_Blue = round(ceil(blurred_RGB[row][col].blues
                                    + blurred_RGB[row][col - 4].blues
                                    + blurred_RGB[row- 1][col].blues
                                    + blurred_RGB[row- 1][col - 4].blues) / 4);
               avg_Green = round(ceil(blurred_RGB[row][col].greens
                                    + blurred_RGB[row][col - 4].greens
                                    + blurred_RGB[row- 1][col].greens
                                    + blurred_RGB[row- 1][col - 4].greens) / 4);
               avg_Red = round(ceil(blurred_RGB[row][col].reds
                                    + blurred_RGB[row][col - 4].reds
                                    + blurred_RGB[row- 1][col].reds
                                    + blurred_RGB[row- 1][col - 4].reds) / 4);
            }

            else //Bottom middle pixel
            {
               avg_Blue = round(ceil(blurred_RGB[row][col].blues
                                    + blurred_RGB[row][col - 4].blues
                                    + blurred_RGB[row][col + 4].blues
                                    + blurred_RGB[row- 1][col].blues
                                    + blurred_RGB[row- 1][col - 4].blues
                                    + blurred_RGB[row- 1][col + 4].blues) / 6);
               avg_Green = round(ceil(blurred_RGB[row][col].greens
                                    + blurred_RGB[row][col - 4].greens
                                    + blurred_RGB[row][col + 4].greens
                                    + blurred_RGB[row- 1][col].greens
                                    + blurred_RGB[row- 1][col - 4].greens
                                    + blurred_RGB[row- 1][col + 4].greens) / 6);
               avg_Red = round(ceil(blurred_RGB[row][col].reds
                                    + blurred_RGB[row][col - 4].reds
                                    + blurred_RGB[row][col + 4].reds
                                    + blurred_RGB[row- 1][col].reds
                                    + blurred_RGB[row- 1][col - 4].reds
                                    + blurred_RGB[row- 1][col + 4].reds) / 6);
            }
         }

         else
         {
            if (col == 0) //left side of the image
            {
               avg_Blue = round(ceil(blurred_RGB[row][col].blues
                                    + blurred_RGB[row- 1][col].blues
                                    + blurred_RGB[row+ 1][col].blues
                                    + blurred_RGB[row][col + 4].blues
                                    + blurred_RGB[row- 1][col + 4].blues
                                    + blurred_RGB[row+ 1][col + 4].blues) / 6); //6 pixels surrounding the left side of the rgbc
               avg_Green = round(ceil(blurred_RGB[row][col].greens
                                    +  blurred_RGB[row- 1][col].greens
                                    +  blurred_RGB[row+ 1][col].greens
                                    +  blurred_RGB[row][col + 4].greens
                                    +  blurred_RGB[row- 1][col + 4].greens
                                    +  blurred_RGB[row+ 1][col + 4].greens) / 6);
               avg_Red = round(ceil(blurred_RGB[row][col].reds
                                    +  blurred_RGB[row- 1][col].reds
                                    +  blurred_RGB[row+ 1][col].reds
                                    +  blurred_RGB[row][col + 4].reds
                                    +  blurred_RGB[row- 1][col + 4].reds
                                    +  blurred_RGB[row+ 1][col + 4].reds) / 6);
            }

            //Right side of the image
            else if (col == (width*4) - 4)
            {
               avg_Blue = round(ceil(blurred_RGB[row][col].blues
                                    + blurred_RGB[row- 1][col].blues
                                    + blurred_RGB[row+ 1][col].blues
                                    + blurred_RGB[row][col - 4].blues
                                    + blurred_RGB[row- 1][col - 4].blues
                                    + blurred_RGB[row+ 1][col - 4].blues) / 6);
               avg_Green = round(ceil(blurred_RGB[row][col].greens
                                    +  blurred_RGB[row- 1][col].greens
                                    +  blurred_RGB[row+ 1][col].greens
                                    +  blurred_RGB[row][col - 4].greens
                                    +  blurred_RGB[row- 1][col - 4].greens
                                    +  blurred_RGB[row+ 1][col - 4].greens) / 6);
               avg_Red = round(ceil(blurred_RGB[row][col].reds
                                    +  blurred_RGB[row- 1][col].reds
                                    +  blurred_RGB[row+ 1][col].reds
                                    +  blurred_RGB[row][col - 4].reds
                                    +  blurred_RGB[row- 1][col - 4].reds
                                    +  blurred_RGB[row+ 1][col - 4].reds) / 6);
            }

            else //middle pixels (The rest of the pixeles)
            {
               avg_Blue = round(ceil(blurred_RGB[row- 1][col - 4].blues
                                    + blurred_RGB[row- 1][col].blues
                                    + blurred_RGB[row- 1][col + 4].blues
                                    + blurred_RGB[row][col - 4].blues
                                    + blurred_RGB[row][col].blues
                                    + blurred_RGB[row][col + 4].blues
                                    + blurred_RGB[row+ 1][col - 4].blues
                                    + blurred_RGB[row+ 1][col].blues
                                    + blurred_RGB[row+ 1][col + 4].blues) / 9);
               avg_Green = round(ceil(blurred_RGB[row- 1][col - 4].greens
                                    + blurred_RGB[row- 1][col].greens
                                    + blurred_RGB[row- 1][col + 4].greens
                                    + blurred_RGB[row][col - 4].greens
                                    + blurred_RGB[row][col].greens
                                    + blurred_RGB[row][col + 4].greens
                                    + blurred_RGB[row+ 1][col - 4].greens
                                    + blurred_RGB[row+ 1][col].greens
                                    + blurred_RGB[row+ 1][col + 4].greens) / 9); // 9 pixels surrounding the middle ones
               avg_Red = round(ceil(blurred_RGB[row- 1][col - 4].reds
                                    + blurred_RGB[row- 1][col].reds
                                    + blurred_RGB[row- 1][col + 4].reds
                                    + blurred_RGB[row][col - 4].reds
                                    + blurred_RGB[row][col].reds
                                    + blurred_RGB[row][col + 4].reds
                                    + blurred_RGB[row+ 1][col - 4].reds
                                    + blurred_RGB[row+ 1][col].reds
                                    + blurred_RGB[row+ 1][col + 4].reds) / 9);
            }
         }
         //Pass the calculated values to the temporary struct
         temporary_RGB[row][col].blues = round((avg_Blue));
         temporary_RGB[row][col].greens = round((avg_Green));
         temporary_RGB[row][col].reds = round((avg_Red));
      }
   }

   // Store the blured pixeles in the blurred_RGB struct then return these values
   for (int row = 0; row < height; row++)
   {
      for (int col= 0; col< width*4; col=col+4)
      {
         blurred_RGB[row][col].blues = temporary_RGB[row][col].blues;
         blurred_RGB[row][col].greens = temporary_RGB[row][col].greens;
         blurred_RGB[row][col].reds = temporary_RGB[row][col].reds;
      }
   }

   // Free the memory dedicated to the struct which is passed as a parameter in this function
	for (int i = 0; i < height; i++){
      free(averageOfRGB[i]);
   }
   free(averageOfRGB);

   // Free the memory of the struct which is local to this function
   for (int i = 0; i < height; i++){
      free(temporary_RGB[i]);
   }
   free(temporary_RGB);

   return blurred_RGB;
}

//Split the image length into batches
struct imageDetails *splitFileLength(int threads, struct imageDetails *paramStruct){

   int start_at = 0;
   int tempoStart = 0;
   int end_at = 0;
   int tempoEnd = 0;
   int index = 0;
   int arrSize = 1;

   // This temporarily struct was used to pass the value of the current split to the main
   struct imageDetails *temporaryPar = calloc(arrSize, IMAGE_DETAILS_SIZE);

   temporaryPar->image = ((struct imageDetails*)paramStruct)->image;
   int end = ((struct imageDetails*)paramStruct)->end;
   int start = ((struct imageDetails*)paramStruct)->start;

   //loop throught the length of the file and split it according to the number of the threads
   while (threads)
   {
      end_at = end / threads;

      /*Dedicated for the struct*/
      tempoStart = start_at;
      tempoEnd = start_at + end_at-1;

      // Pass the values of start and end to the struct
      temporaryPar[index].start = tempoStart;
      temporaryPar[index].end = tempoEnd;
      temporaryPar[index].image = temporaryPar->image;

      //Expanding the size of the struct in order to pass more values of start to end
      arrSize++;
      //Dynamically allocate memory to the struct
      temporaryPar = (struct imageDetails*)realloc(temporaryPar, arrSize*IMAGE_DETAILS_SIZE);

      // Just to display my work (for demonstration purposes)
      printf("\nstarting at: %d\t ending at: %d\n", tempoStart, tempoEnd);
   
      start_at += end_at;
      end -= end_at;
      
      threads -= 1;
      index++;
   }
   return temporaryPar;
}

int main(int argc, char* argv[]){

   // Variable to store thread number
   int threads_number;

   //Confirm if user is inserting a number
   if (argc < 1)
   {
      printf("You need to insert a number\n");
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
      threads_number = toThreads;
   }

   // store the image in a pointer variable of type char
	char *filename = "Task_4/source_img/tom&jerry.png";
   // this variable will return the number of error in the event that lodepng returns an error
	unsigned int error;
   // image is a pointer variable which stores the image content
	unsigned char* image;
   // width and heigh of the image
	unsigned int width, height;

   /* Decode the image
      By passing the filename, we will obtain:
      1- array of RGB values of the image
      2- width and height of the image
   */
	error = lodepng_decode32_file(&image, &width, &height, filename);

   // error handling for lodepng
	if(error){
		printf("Error %d: %s\n", error, lodepng_error_text(error));
		return EXIT_FAILURE;
	}
		
	printf("Image details:\nWidth = %d Height = %d\n", width, height);

   // Constructing imageDetails
   struct imageDetails imageDetails;
   imageDetails.image = image;
   imageDetails.end = height;
   imageDetails.start = 0;

   // Split the image length into batches according to the number of the threads
   struct imageDetails *splitted_image = splitFileLength(threads_number, &imageDetails);
   
   // Get the value of RGB from 1D of array into a struct of 2D array
	struct RGBValues **valueOfRGB = getValueOfRGB(splitted_image, height, width, threads_number);

   // This function returns the values of the blurred pixeles
   struct RGBValues **blurred_img = blurImage(valueOfRGB, height, width);

   //Position of the RGB in the image
   int imgCount = 0;

   //Pass the value from the struct to the image in order to be encoded
   for (int row = 0; row < height; row++)
   {
      for (int col= 0; col< width*4; col=col+4)
      {
         //converting 2D array to 1D array
         image[imgCount] = blurred_img[row][col].reds;
         image[imgCount+1] = blurred_img[row][col].greens;
         image[imgCount+2] = blurred_img[row][col].blues;
         imgCount += 4;
      }
   }
   // Encode the image and stored at a location
	lodepng_encode32_file("Task_4/source_img/tom&jerry_blured.png", image, width, height);

   // free the memory dedicated for blurred image struct
   for (int i = 0; i < height; i++){
      free(blurred_img[i]);
   }
   free(blurred_img);

	return EXIT_SUCCESS; 
}

// clear; gcc Task_4/tas4_boxBlur.c Task_4/lodepng.c -lm; ./a.out 5; rm a.out
