# include <stdio.h>
# include "pf.h"
# include "am.h"
#include "testam.h"

//if print_check is 1 print the tree else do not
int print_check =1;


/* Inserts a key into a leaf node */
InsertintoLeaf(pageBuf,attrLength,attrType,value,recId,buff_hits,num_nodes,buff_access)
char *pageBuf;/* buffer where the leaf page resides */
int attrLength;
char attrType;
char *value;/* attribute value to be inserted*/
int recId;/* recid of the attribute to be inserted */
int *buff_hits;
int * num_nodes;
int * buff_access;
{
    int recSize;
    char tempPage[PF_PAGE_SIZE];
    AM_LEAFHEADER head,*header;
    int errVal;
    int high;
    int compareVal; /* result of comparison of key with value */
    int index; /*index where key is to be inserted*/
    short tempPtr;
    short oldhead;
    //printf("Insert into leaf called");
    /* initialise the header */
    header = &head;
    bcopy(pageBuf,(char*) header,AM_sl);
    ////////
    high = header->numKeys;
    index=high+1;
    recSize = header->attrLength + AM_ss;
    /* The leaf is Empty */
    if (high == 0)
    {
        header->recIdPtr = header->recIdPtr - AM_si - AM_ss;
        tempPtr = header->recIdPtr;
        recSize = header->attrLength + AM_ss;
        /* Update the header */
        header->keyPtr = header->keyPtr + recSize;
        /* copy the new key */
        bcopy(value,pageBuf+AM_sl+(index-1)*recSize,header->attrLength);
        /* Set the head of recId list to the new recid to be added */
        bcopy((char *)  &tempPtr,pageBuf+AM_sl + (index-1)*recSize +
        header->attrLength,AM_ss);
        /* Copy the recId*/
        bcopy((char *)&recId,pageBuf + tempPtr,AM_si);
        header->numKeys++;
        bcopy((char*) header,pageBuf,AM_sl);
        return(TRUE);
    }
    else
    {
        /* compare value with the last inserted key */
        compareVal = AM_Compare(pageBuf + AM_sl + (high - 1)*recSize,
        attrType,attrLength,value);
        if(compareVal==0)
        {
            /* key is already present */
            if ((header->recIdPtr - header->keyPtr) <(AM_si + AM_ss))
            {
                /* no room for one more record */
                //printf("no space in leaf\n");
                return(FALSE);
            }
            else
            {
                recSize = header->attrLength + AM_ss;
                header->recIdPtr = header->recIdPtr - AM_si - AM_ss;
                tempPtr = header->recIdPtr;
                /* save  the old head of recId list */
                bcopy(pageBuf+AM_sl+(index-1)*recSize + header->attrLength,
                (char *)&oldhead, AM_ss);
                /* Update the head of recId list to the new recid to be added */
                bcopy((char *)  &tempPtr,pageBuf+AM_sl + (index-1)*recSize +
                header->attrLength,AM_ss);
                /* Copy the recId*/
                bcopy((char *)&recId,pageBuf + tempPtr,AM_si);
                /* make the old head of list the second on list */
                bcopy((char *)&oldhead,pageBuf + tempPtr+AM_si,AM_ss);
                bcopy((char*) header,pageBuf,AM_sl);
                return (TRUE);
            }
        }
        else
        {
            /*key is not already present */
            if ((header->recIdPtr - header->keyPtr) < (AM_si + AM_ss + recSize))
            {
                //No space available
                return(FALSE);
            }
            else
            {
                //Space is available
                header->recIdPtr = header->recIdPtr - AM_si - AM_ss;
                tempPtr = header->recIdPtr;
                recSize = header->attrLength + AM_ss;
                /* Update the header */
                header->keyPtr = header->keyPtr + recSize;
                /* copy the new key */
                bcopy(value,pageBuf+AM_sl+(index-1)*recSize,header->attrLength);
                /* Set the head of recId list to the new recid to be added */
                bcopy((char *)  &tempPtr,pageBuf+AM_sl + (index-1)*recSize +
                header->attrLength,AM_ss);
                /* Copy the recId*/
                bcopy((char *)&recId,pageBuf + tempPtr,AM_si);
                header->numKeys++;
                //printf("recid added is %d \n",recId);
                bcopy((char*) header,pageBuf,AM_sl);
                return(TRUE);
            }
        }
    }
    ////////
}
Print_check(fd,rightmost_page,rightmost_buf,length,buff_hits)
int fd;
int *rightmost_page;
char **rightmost_buf;
int length;
int * buff_hits;
{
    AM_INTHEADER head3,*header3;
    header3 = &head3;
    //char *belowroot_buf[100000];
    int belowroot_page[100000];
    int len_below_root= 0;
    char * temp1 = (char *)malloc(4);
    char * temp = (char *)malloc(250);
    printf("Printing Root \n");
    bcopy(rightmost_buf[length-1], header3,AM_sint);
    int recrootsize = header3->attrLength +  AM_si;
    for(int j=0; j<header3->numKeys; j++)
    {
        bcopy(rightmost_buf[length-1]+AM_sint+recrootsize*j,temp1,AM_si);
        bcopy(rightmost_buf[length-1]+AM_sint+recrootsize*j+AM_si,temp,header3->attrLength);
        printf("Pointer: %d  Key: %d ",*((int *)temp1),*((int *)temp));
        len_below_root++;
        belowroot_page[j]=*((int *)temp1);
    }
    bcopy(rightmost_buf[length-1]+AM_sint+recrootsize*(header3->numKeys),temp1,AM_si);
    printf("Pointer: %d \n",*((int *)temp1));
    belowroot_page[len_below_root]=*((int *)temp1);
    len_below_root++;
    PF_UnfixPage(fd,rightmost_page[length-1],TRUE);
    printf("\n");
    printf("Printing Level below Root \n");
    int  belowmax;
    int flag = 0;
    int level3[100000];
    int length_level3 =0;
    int errVal;
    char *pagebuf;
    if(length ==3)
    {
        for(int k=0; k<len_below_root; k++)
        {
            printf(" Next node in same level with page number %d\n",belowroot_page[k]);
            errVal = PF_GetThisPage(fd,belowroot_page[k],&pagebuf,buff_hits);
            AM_Check;
            AM_INTHEADER head2,*header2;
            header2 = &head2;
            bcopy(pagebuf, header2,AM_sint);
            int recrootsize = header2->attrLength +  AM_si;
            for(int j=0; j<header2->numKeys; j++)
            {
                bcopy(pagebuf+AM_sint+recrootsize*j,temp1,AM_si);
                bcopy(pagebuf+AM_sint+recrootsize*j+AM_si,temp,header2->attrLength);
                printf("Pointer: %d  Key: %d ",*((int *)temp1),*((int *)temp));
                if(flag ==0)
                {
                    belowmax = *((int *)temp1);
                    flag = 1;
                }
            }
            bcopy(pagebuf+AM_sint+recrootsize*(header2->numKeys),temp1,AM_si);
            printf("Pointer: %d \n",*((int *)temp1));
            errVal = PF_UnfixPage(fd,belowroot_page[k],TRUE);
        }
    }
    else
    {
        for(int k=0; k<len_below_root; k++)
        {
            printf(" Next node in same level with page number %d\n",belowroot_page[k]);
            errVal = PF_GetThisPage(fd,belowroot_page[k],&pagebuf,buff_hits);
            AM_Check;
            AM_INTHEADER head2,*header2;
            header2 = &head2;
            bcopy(pagebuf, header2,AM_sint);
            int recrootsize = header2->attrLength +  AM_si;
            for(int j=0; j<header2->numKeys; j++)
            {
                bcopy(pagebuf+AM_sint+recrootsize*j,temp1,AM_si);
                bcopy(pagebuf+AM_sint+recrootsize*j+AM_si,temp,header2->attrLength);
                printf("Pointer: %d  Key: %d ",*((int *)temp1),*((int *)temp));
                level3[length_level3]= *((int *)temp1);
                length_level3++;
            }
            bcopy(pagebuf+AM_sint+recrootsize*(header2->numKeys),temp1,AM_si);
            printf("Pointer: %d \n",*((int *)temp1));
            level3[length_level3]=*((int *)temp1);
            length_level3++;
            errVal = PF_UnfixPage(fd,belowroot_page[k],TRUE);
        }
        printf("\n Printing nodes at level above leaf \n");
        for(int k=0; k<length_level3; k++)
        {
            printf(" Next node in same level with page number %d\n",level3[k]);
            errVal = PF_GetThisPage(fd,level3[k],&pagebuf,buff_hits);
            AM_Check;
            AM_INTHEADER head4,*header4;
            header4 = &head4;
            bcopy(pagebuf, header4,AM_sint);
            int recrootsize = header4->attrLength +  AM_si;
            for(int j=0; j<header4->numKeys; j++)
            {
                bcopy(pagebuf+AM_sint+recrootsize*j,temp1,AM_si);
                bcopy(pagebuf+AM_sint+recrootsize*j+AM_si,temp,header4->attrLength);
                printf("Pointer: %d  Key: %d ",*((int *)temp1),*((int *)temp));
                if(flag ==0)
                {
                    belowmax = *((int *)temp1);
                    flag = 1;
                }
            }
            bcopy(pagebuf+AM_sint+recrootsize*(header4->numKeys),temp1,AM_si);
            printf("Pointer: %d \n",*((int *)temp1));
            errVal = PF_UnfixPage(fd,level3[k],TRUE);
        }
    }
    printf("\n Printing leaves \n");
    printf("\n Leaf with page number %d \n", belowmax);

    errVal = PF_GetThisPage(fd,belowmax,&pagebuf,buff_hits);
    AM_Check;
    int temp123=2;
    AM_LEAFHEADER head1,*header1;
    header1 = &head1;
    while(1)
    {
        bcopy(pagebuf, header1,AM_sl);
        recrootsize = header1->attrLength +  AM_ss;
        for(int j=0; j<header1->numKeys; j++)
        {
            bcopy(pagebuf+AM_sl+recrootsize*j,temp1,header1->attrLength);
            printf(" Key: %d ",*((int *)temp1));
        }
        int tempo=header1->nextLeafPage;
        errVal = PF_UnfixPage(fd,temp123,TRUE);
        if(tempo != AM_NULL_PAGE)
        {
            temp123=tempo;
            errVal = PF_GetThisPage(fd,header1->nextLeafPage,&pagebuf,buff_hits);
            AM_Check;
            printf("\n Next Leaf with page number %d \n", header1->nextLeafPage);
        }
        else
        break;
    }

}
AddtoParent(fileDesc,level,rightmost_page,rightmost_buf,value,attrLength, length,buff_hits,num_nodes,buff_access)
int fileDesc;
int level;/*Level at which the node which has to be added to parent is present*/
int *rightmost_page;
char **rightmost_buf;
char *value; /*  pointer to attribute value to be added -
gives back the attribute value to be added to it's parent*/
int attrLength;
int * buff_hits;
int * num_nodes;
int * buff_access;
int *length;/*Length of rightmost_page and rightmost_buf array*/
{
    //printf(" @# %d \n", *length);
    int pageNum;/*Page number to be added to the parent*/
    char tempPage[PF_PAGE_SIZE];/* temporary page for manipulating page */
    int pageNumber; /* pageNumber of parent to which key is to be added-
    got from stack*/
    int offset; /* Place in parent where key is to be added -
    got from stack*/
    int errVal;
    int pageNum1,pageNum2; /* pagenumber of new pages to be allocated */
    int tempPageNumber;
    char *tempPagebuffer;
    char *pageBuf,*pageBuf1,*pageBuf2;
    AM_INTHEADER head,*header;
    AM_INTHEADER temphead,*tempheader;
    int recSize;
    char *tempval;
    //printf("Add to parent called \n");
    pageNum=rightmost_page[level];
    /* Get the parent node buffer pointer*/
    pageBuf=rightmost_buf[level+1];
    /* Get the parent node page number*/
    pageNumber=rightmost_page[level+1];
    /* initialise header */
    header = &head;
    tempheader=&temphead;
    /* copy the header from buffer */
    bcopy(pageBuf,header,AM_sint);
    tempval=(char *)malloc(header->attrLength);
    recSize = header->attrLength + AM_si;
    //printf("parent header numkeys %d \n",header->numKeys);
    /* check if there is room in this node for another key */
    if ((header->numKeys) < (header->maxKeys))
    {
        //printf("%d \n",header->numKeys);
        //printf("%d \n",header->maxKeys);
        //printf("If block entered \n");
        /* add the attribute value to the node */
        AM_AddtoIntPage(pageBuf,value,pageNum,header,header->numKeys);
        rightmost_buf[level+1]=pageBuf;
        /* copy the updated header into buffer*/
        bcopy(header,pageBuf,AM_sint) ;
        return(AME_OK);
    }
    else
    {
        /*Not sufficient space in parent node*/
        //////////////////////////
        /*Allocate a new page*/
        //printf("Else block entered \n");
        errVal = PF_AllocPage(fileDesc,&tempPageNumber,&tempPagebuffer);
        AM_Check;
        (*num_nodes)++;
        pageNum1= tempPageNumber;
        pageBuf1=tempPagebuffer;
        /*Set header for the new page*/
        tempheader->pageType = header->pageType;
        tempheader->attrLength = header->attrLength;
        tempheader->maxKeys = header->maxKeys;
        tempheader->numKeys=0;
        bcopy(tempheader,pageBuf1,AM_sint);
        /*Right most key of its left sibling has to be deleted*/
        header->numKeys=header->numKeys -1;
        bcopy(header,pageBuf,AM_sint) ;
        /*Right most pointer of its left sibling has to added to this node*/
        bcopy(pageBuf + AM_sint + (header->maxKeys)*recSize,pageBuf1+AM_sint,AM_si);
        /*Add the value and recid to the newly created node*/
        AM_AddtoIntPage(pageBuf1,value,pageNum,tempheader,tempheader->numKeys);
        //printf("New value of numKeys is %d \n",tempheader->numKeys);
        bcopy(tempheader,pageBuf1,AM_sint);
        /*Set value= value to be added to the parent, that is the right most key of the left sibling*/
        //printf("ghusa");
        bcopy(pageBuf + AM_sint +  AM_si +(header->numKeys)*recSize,tempval,header->attrLength);
        //printf("chala");
        if (pageNumber == AM_RootPageNum)//DOUBT ADITYA NAMBIAR
        {
            /*New root has to be created*/
            /* allocate a new page for a new root */
            //printf("Crearing new root \n");
            errVal = PF_AllocPage(fileDesc,&pageNum2,&pageBuf2);
            AM_Check;
            (*num_nodes)++;
            /* copy the left node to the new buffer space */
            bcopy(pageBuf,pageBuf2,PF_PAGE_SIZE);
            /* fill the header of new root page and the
            attribute value */
            AM_FillRootPage(pageBuf,pageNum2,pageNum1,tempval,
            header->attrLength, header->maxKeys);
            (*length)=(*length)+1;
            /*Set the page number of newly created root node equal to the previous root node*/
            rightmost_page[level+2]=rightmost_page[level+1];
            rightmost_buf[level+2]=pageBuf;
            /*Reset the rightmost entries for level+1*/
            rightmost_page[level+1]=pageNum1;
            rightmost_buf[level+1]=pageBuf1;
            /*Unfix the left sibling of rightmost page at level+1*/
            errVal = PF_UnfixPage(fileDesc,pageNum2,TRUE);
            AM_Check;
            return(AME_OK);
        }
        else
        {
            /*No new root needs to be created*/
            //printf("recursive call \n");
            /*Unfix the left sibling of rightmost node at level+1*/
            errVal = PF_UnfixPage(fileDesc,pageNumber,TRUE);
            AM_Check;
            /*Set the new rightmost entries for level+1*/
            rightmost_page[level+1]=pageNum1;
            rightmost_buf[level+1]=pageBuf1;
            /*Recursive call to add to parent*/
            errVal=AddtoParent(fileDesc,level+1,rightmost_page,rightmost_buf,tempval,attrLength,length,buff_hits,num_nodes,buff_access);
            AM_Check;
        }
        /////////////////////////
    }
    return(AME_OK);
}
/* Inserts a value,recId pair into the tree */
InsertEntry(fileDesc,attrType,attrLength,value,recId,last,buff_hits,num_nodes,buff_access)
int fileDesc; /* file Descriptor */
char attrType; /* 'i' or 'c' or 'f' */
int attrLength; /* 4 for 'i' or 'f', 1-255 for 'c' */
char *value; /* value to be inserted */
int recId; /* recId to be inserted */
int last; /*Wheteher the value to be inserted is the last value*/
int *buff_hits;
int * num_nodes;
int * buff_access;
{
    char *pageBuf; /* buffer to hold page */
    int pageNum; /* page number of the page in buffer */
    int index; /* index where key can be found or can be inserted */
    int status; /* whether key is old or new */
    int inserted; /* Whether key has been inserted into the leaf or
    splitting is needed */
    int addtoparent; /* Whether key has to be added to the parent */
    int errVal; /* return value of functions within this function */
    char key[AM_MAXATTRLENGTH]; /* holds the attribute to be passed
    back to the parent */
    int tempPageNum,tempPageNum1;/* Stores the page number of newly allocated page*/
    char *tempPageBuf,*tempPageBuf1; /*Stores the buffer location of newly allocated page*/
    static char *rightmost_buf[100000];
    static int rightmost_page[100000];
    static int length=0;
    //printf("length is %d \n",length);
    AM_LEAFHEADER temphead,*tempheader;//To store the header of newly allocated page
    int i;//To be used in a loop
    int maxKeys;
    /* check the parameters */
    if ((attrType != 'c') && (attrType != 'f') && (attrType != 'i'))
    {
        AM_Errno = AME_INVALIDATTRTYPE;
        return(AME_INVALIDATTRTYPE);
    }
    if (value == NULL)
    {
        AM_Errno = AME_INVALIDVALUE;
        return(AME_INVALIDVALUE);
    }
    if (fileDesc < 0)
    {
        AM_Errno = AME_FD;
        return(AME_FD);
    }
    if(length==0)
    {
        errVal = PF_GetFirstPage(fileDesc,&pageNum,&pageBuf,buff_hits);
        AM_Check;
        length=1;
        rightmost_buf[0]=pageBuf;
        rightmost_page[0]=pageNum;
    }
    /* Insert into leaf the key,recId pair */
    inserted = InsertintoLeaf(rightmost_buf[0],attrLength,attrType,value,recId,buff_hits,num_nodes,buff_access);
    /* if key has been inserted then done */
    if (inserted == TRUE)
    {
        //printf("Value inserted is %d \n",*((int*)value));
        if(last==1)
        {
            //Unfix all pages
            for(i=0;i<length;i++)
            {
                errVal = PF_UnfixPage(fileDesc,rightmost_page[i],TRUE);
                AM_Check;
            }
            if(print_check == 1)
            Print_check(fileDesc,rightmost_page,rightmost_buf,length,buff_hits);
            AM_EmptyStack();
            return(AME_OK);
        }
        else
        {
            //More values to be inserted
        }
    }
    /* check if there is any error */
    if (inserted < 0)
    {
        AM_EmptyStack();
        AM_Errno = inserted;
        return(inserted);
    }
    /* if not inserted then have to add another leaf node*/
    if (inserted == FALSE)
    {
        //printf("Need to create new leaf node \n");
        //Handle the case when leaf node is full
        /////////
        /* Create a new leafnode*/
        errVal = PF_AllocPage(fileDesc,&tempPageNum,&tempPageBuf);
        AM_Check;
        (*num_nodes)++;
        /*Initialize the header of new leafnode*/
        tempheader=&temphead;
        tempheader->pageType = 'l';
        tempheader->nextLeafPage = AM_NULL_PAGE;
        tempheader->recIdPtr = PF_PAGE_SIZE;
        tempheader->keyPtr = AM_sl;
        tempheader->freeListPtr = AM_NULL;
        tempheader->numinfreeList = 0;
        tempheader->attrLength = attrLength;
        tempheader->numKeys = 0;
        /* the maximum keys in an internal node- has to be even always*/
        maxKeys = (PF_PAGE_SIZE - AM_sint - AM_si)/(AM_si + attrLength);
        if (( maxKeys % 2) != 0)
        tempheader->maxKeys = maxKeys - 1;
        else
        tempheader->maxKeys = maxKeys;
        /* copy the header onto the page */
        bcopy(tempheader,tempPageBuf,AM_sl);
        /* Make the next leaf page pointer of previous page point to the newly created page*/
        bcopy(rightmost_buf[0],tempheader,AM_sl);
        tempheader->nextLeafPage = tempPageNum;
        bcopy(tempheader,rightmost_buf[0],AM_sl);
        ////printf("leaf %d points to %d \n",rightmost_page[0],tempPageNum);
        /*Insert the value to newly created leaf node*/
        //printf("attrLength sent to function %d \n",attrLength);
        inserted = InsertintoLeaf(tempPageBuf,attrLength,attrType,value,recId,buff_hits,num_nodes,buff_access);
        //printf("Value inserted is %d \n",*((int*)value));
        //~ printf("Value of inserted when inserted to new leaf is %d \n",inserted);
        if (inserted < 0)
        {
            AM_EmptyStack();
            AM_Errno = inserted;
            return(inserted);
        }
        if(length==1)
        {
            /*A new root node has to be created*/
            /* the page being split is the root*/
            /* Allocate a new page for another leaf as a new root has
            to be created*/
            errVal = PF_AllocPage(fileDesc,&tempPageNum1,&tempPageBuf1);
            AM_Check;
            (*num_nodes)++;
            AM_LeftPageNum = tempPageNum1; /* this will remain the
            leftmost page hence*/
            /* copy the old leaf page left most page into a new page */
            bcopy(rightmost_buf[0],tempPageBuf1,PF_PAGE_SIZE);
            /* Initialise the new root page */
            rightmost_buf[1]=rightmost_buf[0];
            rightmost_page[1]=rightmost_page[0];
            AM_FillRootPage(rightmost_buf[1],tempPageNum1,tempPageNum,value,
            tempheader->attrLength ,tempheader->maxKeys);
            length=length+1;
            /*unfix the left most leaf node*/
            errVal = PF_UnfixPage(fileDesc,tempPageNum1,TRUE);
            AM_Check;
            rightmost_page[0]=tempPageNum;
            rightmost_buf[0]=tempPageBuf;
            //printf("%d \n",rightmost_page[0]);
            if(last==1)
            {
                
                //Unfix all pages
                for(i=0;i<length;i++)
                {
                    errVal = PF_UnfixPage(fileDesc,rightmost_page[i],TRUE);
                    AM_Check;
                }
                if(print_check == 1)
                Print_check(fileDesc,rightmost_page,rightmost_buf,length,buff_hits);
                AM_EmptyStack();
                return(AME_OK);
            }
        }
        else
        {
            //Root node already present
            errVal = PF_UnfixPage(fileDesc,rightmost_page[0],TRUE);
            AM_Check;
            rightmost_page[0]=tempPageNum;
            rightmost_buf[0]=tempPageBuf;
            errVal=AddtoParent(fileDesc,0,rightmost_page,rightmost_buf,value,attrLength,&length,buff_hits,num_nodes,buff_access);
            if (errVal < 0)
            {
                AM_EmptyStack();
                AM_Errno = errVal;
                return(errVal);
            }
            if(last==1)
            {
                //Redistribute
                
                //Unfix all pages
                for(i=0;i<length;i++)
                {
                    errVal = PF_UnfixPage(fileDesc,rightmost_page[i],TRUE);
                    AM_Check;
                }
                if(print_check == 1)
                Print_check(fileDesc,rightmost_page,rightmost_buf,length,buff_hits);
                AM_EmptyStack();
                return(AME_OK);
            }
        }


    }
    AM_EmptyStack();
    return(AME_OK);
}
main()
{
    int fd;    /* file descriptor for the index */
    char fname[80];    /* file name */
    char buf[250];    /* buf to store data to be inserted into index */
    int recnum;    /* record number */
    int sd;    /* scan descriptor */
    int numrec;    /* # of records retrieved */
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
    //printf("fd is %d \n",fd);
    //printf("inserting into index\n");
    for (recnum=0; recnum < 200; recnum++)
    {
        if(recnum<199)
        {

            InsertEntry(fd,INT_TYPE,sizeof(int),(char *)&recnum,
            recnum,0,buff_hits,num_nodes,buff_access);

        }
        else
        {

            InsertEntry(fd,INT_TYPE,sizeof(int),(char *)&recnum,
            recnum,1,buff_hits,num_nodes,buff_access);

        }
    }

    xPF_CloseFile(fd);
    xAM_DestroyIndex(RELNAME,0);
    printf("test done!\n");
    printf("Statistics  \n");
    printf("Number of nodes created = %i\n", (*num_nodes));
    
     	//printf("Number of Read Seeks = %i\n", nReadSeeks);
        printf("Number of Read Access = %i\n", nReadTransfers);
        //printf("Number of Write Seeks = %i\n", nWriteSeeks);
        printf("Number of Write Access = %i\n", nWriteTransfers);

}
