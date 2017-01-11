#include <stdio.h>

int main(){
	int pl = 2;
	int n = 0;
	char payload[2] = {0};
	char cs = 0x00;
	payload[0] = 0x03;
	payload[1] = 0x01;
	while (n < pl){
		cs = cs ^ payload[n];
		n = n + 1;
	}
	printf("CS es : !%hhX!\n", cs);
	return 0;
}
