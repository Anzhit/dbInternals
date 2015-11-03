AddtoParent(fileDesc,level,rightmost_page,rightmost_buf,value,attrLength,buff_hits,num_nodes,buff_access,length)
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
    int errVal;

    /*Page number of page to be added*/
    int pageNum = rightmost_page[level];
    /*Page number of parent page to which pageNum has to be added*/
    int parentPageNum = rightmost_page[level+1];
    /*Page buffer of parent page*/
    char* parentPageBuf = rightmost_buf[level+1];
    AM_INTHEADER head, *header;

    /*Initialise header */
    header = &head;

    /* copy the header from buffer */
    bcopy(parentPageBuf, header, AM_sint);
    int recSize = header->attrLength + AM_si;

    if(header->numKeys < header->maxKeys){
        AM_AddtoIntPage(parentPageBuf, value, pageNum, header, header->numKeys);
        /*copy updated header to parentPageBuf*/
        bcopy(header, parentPageBuf, AM_sint);
        // NEXT LINE PROBABLY NOT REQUIRED 
        rightmost_buf[level+1] = parentPageBuf;
        return (AME_OK);
    }
    else{
        int newPageNum;
        char* newPageBuf;
        errVal = PF_AllocPage(fileDesc, &newPageNum, &newPageBuf);
        fprintf(stderr, "Something %d\n", newPageNum);
        AM_Check; //check if there is no error in the PF layer functionality
        AM_INTHEADER newhead, *newheader;
        newheader = &newhead;


        //NOTE: INCREMENT NUMBER OF NODES, IF MAINTAINED HERE
        (*num_nodes)++;

        /*Initialise newheader*/
        newheader->pageType = header->pageType;
        newheader->attrLength = header->attrLength;
        newheader->maxKeys = header->maxKeys;
        newheader->numKeys = 0;
        bcopy(newheader, newPageBuf, AM_sint);

        /*Right most key of parentPageBuf has to be deleted and put into the next node*/
        header->numKeys = header->numKeys - 1;
        bcopy(header, parentPageBuf, AM_sint);
        /*For putting the correct pointer (of the lower level node) into the new node on the right*/
        bcopy(parentPageBuf + AM_sint + recSize*(header->numKeys + 1), newPageBuf + AM_sint, AM_si);
        /*Add value to this newly created parent*/
        AM_AddtoIntPage(newPageBuf, value, pageNum, newheader, newheader->numKeys);
        bcopy(newheader, newPageBuf, AM_sint);

       //  /*value of key to be added to parent, i.e., value of rightmost key in left node*/
        // char* val = (char*)malloc(header->attrLength);
        // bcopy(parentPageBuf + AM_sint + (header->numKeys)*recSize + AM_si, val, header->attrLength);  
            

        if(parentPageNum == AM_RootPageNum){ // If a new root needs to be created
            /*Allocate new root page*/
            int newRootNum;
            char* newRootBuf;
            errVal = PF_AllocPage(fileDesc, &newRootNum, &newRootBuf);
            fprintf(stderr, "Something1 %d\n", newRootNum);
            AM_Check;

            // bcopy(parentPageBuf, newRootBuf, PF_PAGE_SIZE);

            // AM_FillRootPage(parentPageBuf, newRootNum, newPageNum, value, header->attrLength, header->maxKeys);

            //NOTE: INCREMENT NUMBER OF NODES, IF MAINTAINED, HERE
            (*num_nodes)++;
            /*Fill in new root*/
            AM_FillRootPage(newRootBuf, parentPageNum, newPageNum, value, header->attrLength, header->maxKeys);
            fprintf(stderr, "NewRootNum : %d, newPageNum: %d, parentPageNum: %d \n", newRootNum, newPageNum, parentPageNum);

            /*Modify rightmost buf and num arrays*/
            (*length)++;

            AM_RootPageNum = newRootNum;

            rightmost_page[level+2] = AM_RootPageNum;
            rightmost_buf[level+2] = newRootBuf;

            /*level + 1 entries to be set to the newPageBuf and Num*/
            rightmost_page[level+1] = newPageNum;
            rightmost_buf[level+1] = newPageBuf;

            /*Unfix left sibling of new page or left child of new root*/
            errVal = PF_UnfixPage(fileDesc, parentPageNum, TRUE);
            AM_Check;
            return (AME_OK);
        }
        else{
            /*level + 1 entries to be set to the newPageBuf and Num*/
            rightmost_page[level+1] = newPageNum;
            rightmost_buf[level+1] = newPageBuf;

            /*Unfix left sibling of new page*/
            errVal = PF_UnfixPage(fileDesc, parentPageNum, TRUE);
            AM_Check;
            
            errVal = AddtoParent(fileDesc, level+1, rightmost_page, rightmost_buf, value, attrLength, buff_hits, num_nodes, buff_access, length);
            AM_Check;   
        }

        

    }
    return (AME_OK);
}
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
    int errVal; /* return value of functions within this function */
    static char *rightmost_buf[100000];
    static int rightmost_page[100000];
    static int length=0;
    if(length==0){
        length++;
        errVal=PF_GetFirstPage(fileDesc,&pageNum,&pageBuf,buff_hits);
        AM_Check;
        rightmost_page[0]=pageNum;
        rightmost_buf[0]=pageBuf;
    }
    int isInserted= InsertintoLeaf(rightmost_buf[0],attrLength,attrType,value,recId,buff_hits,num_nodes,buff_access);
    if(isInserted==TRUE){
        //done
        if(last==1){
            for(int i=0;i<length;i++)
            {
                errVal = PF_UnfixPage(fileDesc,rightmost_page[i],TRUE);
                AM_Check;
            }
            if(print_check == 1)
                Print_check(fileDesc,rightmost_page,rightmost_buf,length,buff_hits);
        }
    }
    else if(isInserted==FALSE){

        /* Create a new leafnode*/
        char *tempPageBuf,*tempPageBuf1; /*Stores the buffer location of newly allocated page*/
        int tempPageNum,tempPageNum1;/* Stores the page number of newly allocated page*/
        AM_LEAFHEADER *tempheader;//To store the header of newly allocated page

        errVal = PF_AllocPage(fileDesc,&tempPageNum,&tempPageBuf);
        AM_Check;
        (*num_nodes)++;
        /*Initialize the header of new leafnode*/
        tempheader=(AM_LEAFHEADER*)malloc(sizeof(AM_LEAFHEADER));
        tempheader->pageType = 'l';
        tempheader->numKeys = 0;
        tempheader->numinfreeList = 0;
        tempheader->nextLeafPage = AM_NULL_PAGE;
        tempheader->recIdPtr = PF_PAGE_SIZE;
        tempheader->keyPtr = AM_sl;
        tempheader->freeListPtr = AM_NULL;
        tempheader->attrLength = attrLength;
        tempheader->maxKeys = (PF_PAGE_SIZE - AM_sint - AM_si)/(AM_si + attrLength);


        /* copy the header onto the page */
        bcopy(tempheader,tempPageBuf,AM_sl);
        /* Make the next leaf page pointer of previous page point to the newly created page*/
        bcopy(rightmost_buf[0],tempheader,AM_sl);
        tempheader->nextLeafPage = tempPageNum;
        bcopy(tempheader,rightmost_buf[0],AM_sl);

        /*Insert the value to newly created leaf node*/
        InsertintoLeaf(tempPageBuf,attrLength,attrType,value,recId,buff_hits,num_nodes,buff_access);

        if(length==1)
        {
            //Allocate new page for root
            errVal = PF_AllocPage(fileDesc,&tempPageNum1,&tempPageBuf1);
            AM_Check;
            //Assign global Vars 
            AM_LeftPageNum = rightmost_page[0];
            AM_RootPageNum=tempPageNum1;
            //Init Root
            AM_FillRootPage(tempPageBuf1,rightmost_page[0],tempPageNum,value,
            tempheader->attrLength ,tempheader->maxKeys);
            length++;
            (*num_nodes)++;
            /*unfix the left most leaf node*/
            errVal = PF_UnfixPage(fileDesc,AM_LeftPageNum,TRUE);
            AM_Check;
            //Assign new leaf to rightmost[0], root to rightmost[1]
            rightmost_page[0]=tempPageNum;
            rightmost_buf[0]=tempPageBuf;
            rightmost_page[1]=tempPageNum1;
            rightmost_buf[1]=tempPageBuf1;
            if(last==1)
            {   
                //Unfix all pages
                for(int i=0;i<length;i++)
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

            if(last==1)
            {
                //TODO Redistribute
                for(int i=0;i<length;i++)
                {
                    errVal = PF_UnfixPage(fileDesc,rightmost_page[i],TRUE);
                    AM_Check;
                }
                if(print_check == 1)
                    Print_check(fileDesc,rightmost_page,rightmost_buf,length,buff_hits);
            }
        }
    }
    if (errVal < 0)
    {
        AM_EmptyStack();
        AM_Errno = errVal;
        return(errVal);
    }
    AM_EmptyStack();
    return(AME_OK);
}