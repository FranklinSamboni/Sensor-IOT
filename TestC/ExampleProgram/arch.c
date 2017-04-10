#include <stdio.h>

int main(){

	FILE *archivo;

	archivo = fopen ("archivo.txt", "r+");

	char str1[] = "la zorra de le gusta mamar";
	char str2[] = "hola";

	fwrite(str1,1,sizeof(str1),archivo);
	fwrite(str2,1,sizeof(str2),archivo);
	fclose(archivo);

	return 0;
}
