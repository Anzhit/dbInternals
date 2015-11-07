/* test3.c: tests deletion and scan. */
#include <cstdio>
#include <cstdlib>
#include "testam.h"
#include "amcppheader.h"
# include "pf.h"

#define MAXRECS	100000	/* max # of records to insert */
#define FNAME_LENGTH 80	/* file name size */

int main()
{
int fd;	/* file descriptor for the index */
char fname[FNAME_LENGTH];	/* file name */
int recnum;	/* record number */
int sd;	/* scan descriptor */
int numrec;	/* # of records retrieved */
int testval;	

int *num_nodes, *buff_hits, *buff_access;
	num_nodes = (int *)malloc(sizeof(int));
	buff_hits = (int *)malloc(sizeof(int));
	buff_access = (int *)malloc(sizeof(int));
	(*num_nodes) = 1;
	(*buff_hits) = 0;
	(*buff_access) = 1;

	/* init */
	//printf("initializing\n");
	PF_Init();

	/* create index */
	//printf("creating index\n");

	AM_CreateIndex(RELNAME,0,INT_TYPE,sizeof(int));

	/* open the index */
	//printf("opening index\n");
	sprintf(fname,"%s.0",RELNAME);
	fd = PF_OpenFile(fname);

	/* first, make sure that simple deletions work */
	//printf("inserting into index\n");
	for (recnum=0; recnum < 200; recnum++){
		AM_InsertEntry(fd,INT_TYPE,sizeof(int),(char *)&recnum,
				recnum,buff_hits,num_nodes,buff_access);
	}
	
	numrec= 0;
	/*sd = AM_OpenIndexScan(fd,INT_TYPE,sizeof(int),EQ_OP,NULL);
	while((recnum=AM_FindNextEntry(sd))>= 0){
		printf("%d\n",recnum);
		numrec++;
	}
	printf("retrieved %d records\n",numrec);
	AM_CloseIndexScan(sd);

	/* destroy everything */
	/*printf("closing down\n");*/
	PF_CloseFile(fd);
		AM_DestroyIndex(RELNAME,0);

	printf("test3 done!\n");
	printf("Statistics  \n");
  	printf("Number of nodes created = %i\n", (*num_nodes));
  	printf("Buffer hits %d \n",*buff_hits);
  	 	//printf("Number of Read Seeks = %i\n", nReadSeeks);
        printf("Number of Read Access = %i\n", nReadTransfers);
        //printf("Number of Write Seeks = %i\n", nWriteSeeks);
        printf("Number of Write Access = %i\n", nWriteTransfers);
}
