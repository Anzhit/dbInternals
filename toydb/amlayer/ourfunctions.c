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