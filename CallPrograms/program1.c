#include <stdio.h>

int main()
{
   char str[50];

   printf("Enter a string : ");
   gets(str);

   printf("You entered: %s\n", str);
   system("./program2 jj", str);
   printf("\nFinalizo programa 1\n");
   return(0);
}

