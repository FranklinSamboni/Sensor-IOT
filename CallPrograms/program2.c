#include <stdio.h>
int main(int argc, char *argv[]) {
   char str[50];
   if(argc > 1){
	printf("Argc: %d \n", argc);
	printf("Argv: %s\n", argv[1]);
	printf("Ingrese algo: ");
	gets(str);
	printf("\nIngreso: %s",str);
   }else{
	printf("No se ingresaron argumentos\n");
	}
   return 0;
}
