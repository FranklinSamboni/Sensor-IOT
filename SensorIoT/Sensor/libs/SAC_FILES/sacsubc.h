#ifndef _SACHDR_H
#define _SACHDR_H
/* this is the include file for the
	C language SAC codes
*/

#define True 1
#define False 0

#ifdef MSDOS
#define INT long
#else
#define INT int
#endif


/* data structures */

struct sachdr_  {
	float rhdr[70];
	INT ihdr[40];
	char chdr[24][8];
	}  ;

typedef struct fullDate fullDate;

struct fullDate{
	int year;
	int month;
	int day;
	int hour;
	int min;
	int seg;
	int mseg;
};

typedef struct depValues depValues;
struct depValues{
	 float depmax;
	 float depmin;
	 float depmen;
	 float dt;
	 int npts;
	 int dataNumber;
};
/* function prototypes */

void brsac (int npts,char *name,float **data,int *nerr);
void brsach(char *name,int *nerr);
void arsac (int npts,char *name,float **data,int *nerr);
void arsach(char *name,int *nerr);
void getfhv(char *strcmd,float *fval,int *nerr);
void getnhv(char *strcmd,int *ival,int *nerr);
void getkhv(char *strcmd,char *cval,int *nerr);
void getlhv(char *strcmd,int *lval,int *nerr);
void bwsac (int LN,char *name,float *data);
void awsac (int npts,char *name,float *data);
void setfhv(char *strcmd,float  fval,int *nerr);
void setnhv(char *strcmd,int  ival,int *nerr);
void setkhv(char *strcmd,char *cval,int *nerr);
void setlhv(char *strcmd,int lval,int *nerr);
void newhdr(void);
void inihdr(void);
void getihv(char *strcmd,char *strval,int *nerr);
void setihv(char *strcmd,char *strval,int *nerr);
int streql(char *str1,char *str2);
void wsac1(char *ofile,float *y,int npts,float btime,float dt,int *nerr);
void scmxmn(float *x, int npts, float *depmax, float *depmin, float *depmen);

void createFile(char *name);
void updateHeaders(char *name);
void updateData(char *name,int LN,float *data);

#endif
