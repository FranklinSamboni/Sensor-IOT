#include "sacsubc.h"
#include <stdio.h>
/* this is a sample program that creates two sac files
   one of a simple impulse and the other with the impulse passed
   through a lowpass recursive digital filter
*/
/* define the maximum number of points  and the two float arrays */
#define NPTS 200
float x[NPTS];
float y[NPTS];

void outputsac(int npts, float *arr, float dt, char *filename);

int main()
{
	printf("inicio\n");
        int i;
        float dt = 0.005 ;
        int offset = 10;
        int offset2 = 90;
        float wc=31.415927;
        float f,a,b;
        f = 2./(dt*wc);
        a = 1. + f;
        b = 1. - f;
        /* initialize the impulse */
        for(i=0;i< NPTS;i++)
                x[i] = 0.0;
        x[offset] = 1.0/dt ;
        x[offset2] = 1.0/dt ;
        /* now apply a recursive digital filter to create the
           output */
        y[0] = 0.0;
        for(i=1;i < NPTS; i++){
                y[i] = (x[i]+x[i-1] - b*y[i-1])/a;
        }
        outputsac(NPTS, x, dt, "imp.sac");
        outputsac(NPTS, y, dt, "filt.sac");
        return 0;
}
void outputsac(int npts, float *arr, float dt, char *filename)
{
        /* create the SAC file
           instead of using the wsac1 I will use the lower level
           routines to provide more control on the output */
        int nerr;
        float b, e, depmax, depmin, depmen;
        /* get the extrema of the trace */
                scmxmn(arr,npts,&depmax,&depmin,&depmen);
        /* create a new header for the new SAC file */
                newhdr();
        /* set some header values */
                setfhv("DEPMAX", depmax, &nerr);
                setfhv("DEPMIN", depmin, &nerr);
                setfhv("DEPMEN", depmen, &nerr);
                setnhv("NPTS    ",npts,&nerr);
                setfhv("DELTA   ",dt  ,&nerr);
                b = 0;
                setfhv("B       ",b  ,&nerr);
                setihv("IFTYPE  ","ITIME   ",&nerr);
                e = b + (npts -1 )*dt;
                setfhv("E       ",e     ,&nerr);
                setlhv("LEVEN   ",1,&nerr);
                setlhv("LOVROK  ",1,&nerr);
                setlhv("LCALDA  ",1,&nerr);
        /* put is a default time for the plot */
                setnhv("NZYEAR", 2017, &nerr);
                setnhv("NZJDAY", 2, &nerr);
                setnhv("NZHOUR", 1, &nerr);
                setnhv("NZMIN" , 1, &nerr);
                setnhv("NZSEC" , 1, &nerr);
                setnhv("NZMSEC", 1, &nerr);

                setkhv("KNETWK", "MEC",&nerr);
                setkhv("KSTNM", "POP",&nerr);
                setkhv("KCMPNM", "BHZ",&nerr);
        /* output the SAC file */
                bwsac(npts,filename,arr);
                printf("metodo\n");
}
