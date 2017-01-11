#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define SLAVE_ADDRESS 0x68
#define FILE_NAME "/dev/i2c-1"
#define BUF_SIZE 0x13

FILE *fecha;

char* getDay(int day){
    switch(day){
	case 1:
	    return "Domingo";
	break;
	case 2:
	    return "Lunes";
	break;
	case 3:
	    return "Martes";
	break;
	case 4:
	    return "Miercoles";
	break;
	case 5:
	    return "Jueves";
	break;
	case 6:
	    return "Viernes";
	break;
	case 7:
	    return "Sabado";
	break;
	default:
		printf("Error Dia valor diferente de 1-7");
		exit(1);
		break;
    }
}

char* getMonth(int Month){
    switch(Month){
	case 1:
	    return "Enero";
	break;
	case 2:
	    return "Febrero";
	break;
	case 3:
	    return "Marzo";
	break;
	case 4:
	    return "Abril";
	break;
	case 5:
	    return "Mayo";
	break;
	case 6:
	    return "Junio";
	break;
	case 7:
	    return "Julio";
	break;
	case 8:
	    return "Agosto";
	break;
	case 9:
	    return "Septiembre";
	break;
	case 16:
	    return "Octubre";
	break;
	case 17:
		return "Noviembre";
		break;
	case 18:
	    return "Diciembre";
		break;
	default:
		printf("Error Mes valor diferente de 1-12");
		exit(1);
		break;
    }
}

void writeReg(unsigned int addr, unsigned int data, int file){
    unsigned char write_Buf[2];
   // unsigned int addr = 0b00000000;
   // unsigned int data = 0b00000000;
    printf("valor: %x \n", data);
    write_Buf[0] = addr;
    write_Buf[1] = data;
    if(write(file,write_Buf,2) != 2) {
        perror("Write Error");
    }
}

char* read_Rtc() {

    int file;
    if ((file = open(FILE_NAME, O_RDWR | O_NOCTTY | O_NDELAY)) < 0) {
        // Aqui se puede ver si hay algun error abriendo el archivo
        perror("Fallo a abrir el bus i2c. \n");
        exit(1);
    }
    //Ahora modificamos la direccion del sclavo para que el RTC comience a enviar datos
    if (ioctl(file, I2C_SLAVE, SLAVE_ADDRESS) < 0) {
        printf("Fallo al intentar acceder al dispositivo esclavo.\n");
        // Podemos obervar que ha ido mal
        exit(1);
    }

  //Es necesario escribir en a posicion 0 para que al realizar la lectura los 
   // datos lleguen en orden (en el orden descrito en la hoja de especificaciones
  // del rtc).
/*    writeReg(0x06,0x16,file);
    writeReg(0x05,0x10,file);
    writeReg(0x04,0x13,file);
    writeReg(0x03,0x01,file);
    writeReg(0x02,0x11,file);
    writeReg(0x01,0x45,file);

*/
    
//   writeReg(0x00,0x00,file);

    unsigned char buffer[BUF_SIZE];

    int read_in = read(file,buffer,BUF_SIZE);
    if(read_in == -1){
        printf("Fallo al intentar leer el bus i2c: %s. \n", strerror(errno));
    }
    else{
        //read_in = read(file,buffer,BUF_SIZE);
       printf("numero de bytes es: %i \n",read_in);
       int i=0;
       while(i < BUF_SIZE){
	   printf("Dato: %d ",i);
 	   printf("Es: !%x! \n", buffer[i]);
	   if(i==8){
		   printf("\n");
	   	   }
	   i = i+1;
       }
	char* day = getDay(buffer[3]);
	//printf("---%x----%d----%hhX",buffer[3],buffer[3],buffer[3]);
	//printf("---%x----%d----%hhX",buffer[5],buffer[5],buffer[5]);
	char* Month = getMonth(buffer[5]);

	/*printf("\nEl dia es: %s, La hora es: %x:%x:%x. \n",day,buffer[2],buffer[1],buffer[0]);
	printf("La fecha es: %x/%x/%x. \n", buffer[4],buffer[5],buffer[6]);*/
	char *result = malloc(255);

	sprintf(result,"\n%s, %x de %s de 20%x hora %x:%x:%x. \n",day, buffer[4],Month,buffer[6],buffer[2],buffer[1],buffer[0]);
	printf("\n%s, %x de %s de 20%x hora %x:%x:%x. \n",day, buffer[4],Month,buffer[6],buffer[2],buffer[1],buffer[0]);
	close(file);
	return result;
    }
}

int keepGoing = 1;
void signal_handler(int sig){

	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepGoing = 0;
	fclose(fecha);
}

int main(){

	fecha = fopen ("fecha.txt", "w+");
	signal(SIGINT, signal_handler);
	while(keepGoing){
		fprintf(fecha,read_Rtc());

	}
	fclose(fecha);


    /*read_in = read(file,buffer,BUF_SIZE);
    if(read_in == -1){
        printf("Fallo al intentar leer el bus i2c: %s. \n", strerror(errno));
    }
    else{
        //read_in = read(file,buffer,BUF_SIZE);
        printf("numero de bytes es: %i \n ",read_in);
        for(int i=0; i < BUF_SIZE; i++){
        printf("El valor es: %c \n", data[i]);
    }
    }*/
    /*
    for (int i = 0; i<4; i++) {
         // Using I2C Read
        if (read(file,buf,2) != 2) {
            // ERROR HANDLING: i2c transaction failed
            printf("Failed to read from the i2c bus: %s.\n", strerror(errno));
            printf("\n\n");
        } else {
            printf("Estos son los datos: " + buf[0] +"\n" );
        }
    }*/
    return 0;
}
