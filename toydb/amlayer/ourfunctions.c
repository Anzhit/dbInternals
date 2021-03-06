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


/* Inserts a key into a leaf node */
// InsertintoLeaf(pageBuf,attrLength,attrType,value,recId,buff_hits,num_nodes,buff_access)
// char *pageBuf;/* buffer where the leaf page resides */
// int attrLength;
// char attrType;
// char *value;/* attribute value to be inserted*/
// int recId;/* recid of the attribute to be inserted */
// int *buff_hits;
// int * num_nodes;
// int * buff_access;
// {
//   int recSize;
//   char tempPage[PF_PAGE_SIZE];
//   AM_LEAFHEADER head,*header;
//   int errVal;
//   int high;
//   int compareVal; /* result of comparison of key with value */
//   int index; /*index where key is to be inserted*/
//   short tempPtr;
//   short oldhead;
//   //printf("Insert into leaf called");
//   /* initialise the header */
//   header = &head;
//   bcopy(pageBuf,(char*) header,AM_sl);
//   ////////
//   high = header->numKeys;
//   index=high+1;
//   recSize = header->attrLength + AM_ss;
//   /* The leaf is Empty */
//   if (high == 0)
//   {
//      printf("%d\n", header->recIdPtr);
//       header->recIdPtr = header->recIdPtr - AM_si - AM_ss;
//       tempPtr = header->recIdPtr;
//       recSize = header->attrLength + AM_ss;
//       /* Update the header */
//       header->keyPtr = header->keyPtr + recSize;
//       /* copy the new key */
//       bcopy(value,pageBuf+AM_sl+(index-1)*recSize,header->attrLength);
//       /* Set the head of recId list to the new recid to be added */
//       bcopy((char *)  &tempPtr,pageBuf+AM_sl + (index-1)*recSize +
//       header->attrLength,AM_ss);
//       /* Copy the recId*/
//       bcopy((char *)&recId,pageBuf + tempPtr,AM_si);
//       header->numKeys++;
//       bcopy((char*) header,pageBuf,AM_sl);
//       return(TRUE);
//   }
//   else
//   {
//       /* compare value with the last inserted key */
//       compareVal = AM_Compare(pageBuf + AM_sl + (high - 1)*recSize,
//       attrType,attrLength,value);
//       if(compareVal==0)
//       {
//           /* key is already present */
//           if ((header->recIdPtr - header->keyPtr) <(AM_si + AM_ss))
//           {
//               /* no room for one more record */
//               //printf("no space in leaf\n");
//               return(FALSE);
//           }
//           else
//           {
//               recSize = header->attrLength + AM_ss;
//               header->recIdPtr = header->recIdPtr - AM_si - AM_ss;
//               tempPtr = header->recIdPtr;
//               /* save  the old head of recId list */
//               bcopy(pageBuf+AM_sl+(index-1)*recSize + header->attrLength,
//               (char *)&oldhead, AM_ss);
//               /* Update the head of recId list to the new recid to be added */
//               bcopy((char *)  &tempPtr,pageBuf+AM_sl + (index-1)*recSize +
//               header->attrLength,AM_ss);
//               /* Copy the recId*/
//               bcopy((char *)&recId,pageBuf + tempPtr,AM_si);
//               /* make the old head of list the second on list */
//               bcopy((char *)&oldhead,pageBuf + tempPtr+AM_si,AM_ss);
//               bcopy((char*) header,pageBuf,AM_sl);
//               return (TRUE);
//           }
//       }
//       else
//       {
//           /*key is not already present */
//           if ((header->recIdPtr - header->keyPtr) < (AM_si + AM_ss + recSize))
//           {
//               //No space available
//               return(FALSE);
//           }
//           else
//           {
//               //Space is available
//               header->recIdPtr = header->recIdPtr - AM_si - AM_ss;
//               tempPtr = header->recIdPtr;
//               recSize = header->attrLength + AM_ss;
//               /* Update the header */
//               header->keyPtr = header->keyPtr + recSize;
//               /* copy the new key */
//               bcopy(value,pageBuf+AM_sl+(index-1)*recSize,header->attrLength);
//               /* Set the head of recId list to the new recid to be added */
//               bcopy((char *)  &tempPtr,pageBuf+AM_sl + (index-1)*recSize +
//               header->attrLength,AM_ss);
//               /* Copy the recId*/
//               bcopy((char *)&recId,pageBuf + tempPtr,AM_si);
//               header->numKeys++;
//               //printf("recid added is %d \n",recId);
//               bcopy((char*) header,pageBuf,AM_sl);
//               return(TRUE);
//           }
//       }
//   }
//   ////////
// }

typedef struct am_bagheader{
    short numKeys;
    short lastPtr;
    short nextPageNum;
}   AM_BAGHEADER ; 

# define AM_sb sizeof(AM_BAGHEADER)


InsertintoLeaf(fd,bagPage,pageBuf,attrLength,attrType,value,recId,buff_hits,num_nodes,buff_access)
int fd; // file descriptor
int *bagPage;
char *pageBuf;/* buffer where the leaf page resides */
int attrLength;
char attrType;
char *value;/* attribute value to be inserted*/
int recId;/* recid of the attribute to be inserted */
int *buff_hits;
int * num_nodes;
int * buff_access;
{
    int recSize, numElts, attributeLength;
    AM_LEAFHEADER head, *header;
    header = &head;
    
    bcopy(pageBuf, (char*)header, AM_sl);

    attributeLength = header->attrLength;
    recSize = attributeLength + AM_si + AM_si + AM_ss; // value, pagenum, pointer within page, single or multiple
    numElts = header->numKeys;
    int ptr, lastAddressinBag;

    // inserting the first element
    if(numElts == 0){
        // assume that space is available since it's the first element
        
        bcopy(pageBuf, (char*)header, AM_sl);
        
        header->keyPtr = header->keyPtr + recSize;
        header->numKeys = 1;

        // insert the key value into the node
        bcopy(value, pageBuf + AM_sl, attributeLength);

        //for now point to some random location as key = record value
        ptr = -1;
        bcopy((char*)&ptr, pageBuf + AM_sl + attributeLength, AM_si);
        // ptr = address of entry
        bcopy((char*)&ptr, pageBuf + AM_sl + attributeLength + AM_si, AM_si);
        short single = 1;
        bcopy((char*)&single, pageBuf + AM_sl + attributeLength + AM_si*2, AM_ss);
        
        // write the header back onto the page
        bcopy((char*)header, pageBuf, AM_sl);

        return TRUE;
    }
    // some elements already exist
    else{

        bcopy(pageBuf, (char*)header, AM_sl);
        
        // get last element inserted
        char lastVal[AM_si], lastToLastVal[AM_si];
        
        bcopy((char*)(pageBuf + AM_sl + (numElts-1)*recSize), lastVal, attributeLength);
        
        int present = strcmp(value, lastVal);
        
        // check if key is present
        if(present == 0){
            // yes, present
            
            short single;
            bcopy((char*)(pageBuf + AM_sl + (numElts-1)*recSize + 2*AM_si), &single, AM_ss);
            int flag = 0;
            if(single == 0){
                flag = 1;
            }

            if(flag){
                // at least two similar values exist, no need to initialize stuff
                
                AM_BAGHEADER *bagHeader, bagHead;
                bagHeader = &bagHead;

                char tempBuf[PF_PAGE_SIZE];
                int pntr, bagPageNum, lastAddressinBag;

                // get the starting address within the correct page where the bag header resides
                bcopy((char*)(pageBuf + AM_sl + (numElts-1)*recSize + attributeLength), (char*)&bagPageNum, AM_si);
                bcopy((char*)(pageBuf + AM_sl + (numElts-1)*recSize + attributeLength + AM_si), (char*)&pntr, AM_si);

                PF_GetThisPage(fd, bagPageNum, &tempBuf, buff_hits);
                bcopy(pageBuf, &lastAddressinBag, AM_si);
                bcopy(tempBuf + pntr, (char*)bagHeader, AM_sb);

                // check if space is available in this node
                if(PF_PAGE_SIZE - lastAddressinBag > AM_si){
                    // add to the remaining elements
                    bagHeader->numKeys++;
                    bcopy((char*)&ptr, (char*)bagHeader->lastPtr, AM_si);
                    bagHeader->lastPtr += AM_si;
                    bcopy((char*)bagHeader, tempBuf + AM_si + pntr, AM_sb); 

                    return TRUE;                
                }
                else{
                    // if not, then create a new node
                    // chain the leaves

                    AM_BAGHEADER *bagHeader1, bagHead1;
                    char tempBuf1[PF_PAGE_SIZE];

                    PF_AllocPage(fd, &bagPageNum, &tempBuf1);
                    *bagPage = bagPageNum;
                    bagHeader->nextPageNum = bagPageNum;
                    bcopy((char*)bagHeader, tempBuf + AM_si + pntr, AM_sb);

                    int s = AM_si + AM_sb + AM_si;
                    bcopy((char*)&s, tempBuf1, AM_si);
                    
                    bagHeader1 = &bagHead1;

                    bagHeader1->numKeys = 1;
                    bagHeader1->nextPageNum = -1;
                    bagHeader1->lastPtr = s;

                    bcopy((char*)bagHeader1, tempBuf1 + AM_si, AM_sb);

                    return TRUE;
                }

            }
            else{
                // second copy of the last value,
                // need to create a new header to store the pointers
                                
                char tempBuf[PF_PAGE_SIZE];
                int pntr, lastAddressinBag;
                PF_GetThisPage(fd, *bagPage, &tempBuf, buff_hits);

                // check is space is available in the page
                bcopy(tempBuf, &lastAddressinBag, AM_si);
                
                if(PF_PAGE_SIZE - lastAddressinBag < AM_sb + AM_si*2){

                    // no space
                    // allocate a new page and write stuff to it

                    AM_BAGHEADER *bagHeader1, bagHead1;
                    char tempBuf1[PF_PAGE_SIZE];
                    int bagPageNum;

                    PF_AllocPage(fd, &bagPageNum, &tempBuf1);
                    *bagPage = bagPageNum;
                    
                    int s = AM_si + AM_sb + AM_si*2;
                    bcopy((char*)&s, tempBuf1, AM_si);
                    
                    bagHeader1 = &bagHead1;

                    bagHeader1->numKeys = 2;
                    bagHeader1->nextPageNum = -1;
                    bagHeader1->lastPtr = s;

                    bcopy((char*)bagHeader1, tempBuf1 + pntr, AM_sb);

                    return TRUE;

                }
                else{

                    // space available
                    AM_BAGHEADER bagHead, *bagHeader;
                    bagHeader = &bagHead;
                    
                    bagHeader->numKeys = 2;
                    bagHeader->nextPageNum = -1;
                    bagHeader->lastPtr = lastAddressinBag + AM_sb + AM_si*2;;
                    bcopy((char*)bagHeader, tempBuf + lastAddressinBag, AM_sb);
                    
                    // copy the two pointers
                    // bcopy();
                    // bcopy();
                    short v = 0;
                    int bagPageNum;
                    // copy the address in the original node
                    bcopy((char*)&bagPageNum, pageBuf + AM_sl + (numElts-1)*recSize + attributeLength, AM_si);
                    bcopy((char*)&lastAddressinBag, pageBuf + AM_sl + (numElts-1)*recSize + attributeLength + AM_si, AM_si);
                    bcopy((char*)&v, pageBuf + AM_sl + (numElts-1)*recSize + attributeLength + AM_si*2, AM_ss);

                    lastAddressinBag += AM_sb + AM_si*2; // one for each pointer
                    bcopy((char*)&lastAddressinBag, tempBuf, AM_si);

                    return TRUE;
                }
            }
        }
        else{
            // element not present before
            // this element is being inserted for the first time
            if(PF_PAGE_SIZE - header->keyPtr < recSize){
                return FALSE;
            }
            else{
                // space is available
                bcopy(pageBuf, (char*)header, AM_sl);

                header->keyPtr = header->keyPtr + recSize;
                header->numKeys = 1;

                // insert the key value into the node
                int offset =  AM_sl + recSize;
                bcopy(value, pageBuf + offset, attributeLength);

                //for now point to some random location as key = record value
                ptr = -1;
                bcopy((char*)&ptr, pageBuf + offset + attributeLength, AM_si);
                // ptr = address of entry
                bcopy((char*)&ptr, pageBuf + offset + attributeLength + AM_si, AM_si);
                short single = 1;
                bcopy((char*)&single, pageBuf + offset + attributeLength + AM_si, AM_ss);
                
                // write the header back onto the page
                bcopy((char*)header, pageBuf, AM_sl);

                return TRUE;
            }
        }
    }
}
