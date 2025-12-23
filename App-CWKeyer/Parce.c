/**


*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFERLENGTH  80

int main( int argc, char *argv[] ){
  FILE *fpin;  /* file pointer */
  FILE *fpout;  /* file pointer */
  char inbuffer[];
	
  /* open file to read */
  if( ( fpin = fopen( argv[2], "r" ) ) == NULL )
  {
    perror( "Error opening input file" );
    exit( -1 );
  }
  printf( "Reading data file: %s\n\n", argv[2] );

  /* open file to write */
  if( ( fpout = fopen( argv[3], "w" ) ) == NULL )
  {
    perror( "Error opening output file" );
    exit( -1 );
  }
  printf( "Opening output file: %s\n", argv[3] );  

  While(!EOF){
		for (int i=0;i<BUFFERLENTH;i++){
			inbuffer[i] = '\0';
		}
		fgets(inbuffer, '\n', fpin);
		printf("%s",inbuffer);
	}
	
}
