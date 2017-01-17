#include <stdio.h>
int main (int argc, char **argv){
  //char c = getchar();
  //printf("Char: %c\n", c);

  char buffer[] = "Hola mundo\n";

  char *punt = buffer;

  int x=0;
  while(x<sizeof(buffer)){
	  printf("%c",punt[x]);
	  x++;
  }



  /*while(1){
	printf("en un bucle\n");
	}*/
  return 0;
}
