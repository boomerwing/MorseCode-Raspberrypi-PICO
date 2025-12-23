/**  Parce Function
 *  Takes a sentence from a file such as the following: 
 *  ABOUT CAN GOOD LARGE ON THAN WAS AFTER COULD GOT LAST ONE THAT WE
 *  It separates each word, looking for a SPACE or \0 to end each word.
 *  It writes each word in a separate line in an output file
 *  formated for use in another program.
 *   char p0[] = "ABOUT";
 *   char p1[] = "CAN";
 *   char p2[] = "GOOD";
 *   char p3[] = "LARGE";

cd ~/FreeRTOS-CW-Play/Parce
gcc Parce_2.c -o parce

./parce RAW_TEXT_1.txt parce2_out_1.txt
./parce RAW_TEXT_11.txt parce2_out_11.txt
./parce RAW_TEXT_2.txt parce2_out_2.txt
./parce RAW_TEXT_2.txt parce2_out_22.txt
 
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFERLENGTH  120

int main( int argc, char *argv[] ){
    FILE *fpin;   /* file pointer */
    FILE *fpout;  /* file pointer */
    char *inNow;   // pointer to character to be removed from inbuffer
    char *outNow;  // pointer to character to be placed in outbuffer
    char inbuffer[BUFFERLENGTH];
    char outbuffer[BUFFERLENGTH];
    char c;
    int length;
    int wordCount = 0;
    int flag;
    
        for (int i=0;i<BUFFERLENGTH;i++){
            inbuffer[i] = '\0';
        }
 
 /* open file to read */
    if( ( fpin = fopen( argv[1], "r" ) ) == NULL )
        {
        perror( "Error opening input file" );
        exit( -1 );
        }
        printf( "Reading data file: %s\n", argv[1] );

  /* open file to write */
        if( ( fpout = fopen( argv[2], "w" ) ) == NULL )
            {
            perror( "Error opening output file" );
            exit( -1 );
        }
        printf( "Opening output file: %s\n", argv[2] );
        
    
    while(1){
        // ***********  Start Parse function ********************************        
            for(int i = 0; i<BUFFERLENGTH;i++){ // reset inbuffer
                inbuffer[i] = '\0';
            }
            for(int i = 0; i<BUFFERLENGTH;i++){ // reset outbuffer
                outbuffer[i] = '\0';
            }
            

        fgets(inbuffer, BUFFERLENGTH, fpin);  //  Bring in one line
        if(feof(fpin)) break;  // EOF Forces while(1) loop to end
        
        length = strlen(inbuffer);  // remove '\n'
        inbuffer[length-1] = '\0';
        
        inNow = inbuffer;   // reset Pointer to destination in inbuffer
        outNow = outbuffer; // reset Pointer to destination in outbuffer
        
        flag =1;  // signal to stop word collection in one sentence
        
        
         while(flag) {  // Collect words until Flag is set FALSE
            c = *inNow; 
            
            switch(c){
                case '\0': // End of input sentence
                    fprintf(fpout,"     char p%i[] = \"%s\";", wordCount,outbuffer);
                    fprintf(fpout,"\n");
                    wordCount++;
                    flag=0;   // Break out of the word loop to get new sentence.        
                    break;
               case ' ':   // End of input word
                    fprintf(fpout,"     char p%i[] = \"%s\";", wordCount,outbuffer);
                    fprintf(fpout,"\n");
                     for(int i = 0; i<BUFFERLENGTH;i++){ // reset outbuffer 
                        outbuffer[i] = '\0';
                    }
                    outNow = outbuffer; // rewind outbuffer Pointer for new word
                    inNow++;            // Move inbuffer Pointer to start new word
                    wordCount++;
                    break;
                default:
                    *outNow++ = *inNow++; // Collect characters for word
                }  // end of switch
            } // end of Word search in one sentence
            
        }  // end of file fpin search
        printf( "Closing input file: %s\n", argv[1] );
        fclose(fpin);
        printf( "Closing output file: %s\n", argv[2] );
        fclose(fpout);
}
/**
cd ~/FreeRTOS-CW-Play/Parce
gcc Parce_2.c -o parce

./parce RAW_TEXT_1.txt parce2_out_1.txt
./parce RAW_TEXT_11.txt parce2_out_11.txt
./parce RAW_TEXT_2.txt parce2_out_2.txt
./parce RAW_TEXT_22.txt parce2_out_22.txt

*/
