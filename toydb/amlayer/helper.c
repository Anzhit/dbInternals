# include <stdio.h>
# include "pf.h"
# include "am.h"


/* Inserts a value,recId pair into the tree */
InsertData(fileDesc, attrType, attrLength, value, recId, rightmostPage, rightmostBuf, length)
int fileDesc; /* file Descriptor */
char attrType; /* 'i' or 'c' or 'f' */
int attrLength; /* 4 for 'i' or 'f', 1-255 for 'c' */
char *value; /* value to be inserted */ 
int recId; /* recId to be inserted */
int *rightmostPage; 
char **rightmostBuf;
int *length;
{
	char *pageBuf; /* buffer to hold page */
	int pageNum; /* page number of the page in buffer */
	int status; /* whether key is old or new */
	int inserted; /* Whether key has been inserted into the leaf or a new node is needed */
	int errVal;
	pageBuf=rightmostBuf[*length-1];
	/* check the parameters */
	if ((attrType != 'c') && (attrType != 'f') && (attrType != 'i')){
		AM_Errno = AME_INVALIDATTRTYPE;
		return(AME_INVALIDATTRTYPE);
	}

	if (value == NULL) {
		AM_Errno = AME_INVALIDVALUE;
		return(AME_INVALIDVALUE);
	}

	if (fileDesc < 0) {
		AM_Errno = AME_FD;
		return(AME_FD);
   	}
	
	/* Insert into leaf the key,recId pair */
	inserted = InsertIntoLeaf(pageBuf, value, recId);

	/* if key has been inserted then done */
	if (inserted == TRUE){
		errVal = PF_UnfixPage(fileDesc, pageNum, TRUE);
		AM_Check;
		AM_EmptyStack();
		return(AME_OK);
	}
	else{
		
		PF_UnfixPage(fileDesc, pageNum, TRUE);

		AM_LEAFHEADER head, *header;
		bcopy(pageBuf, header, AM_sl);
		header = &head;

		char *newPageBuf;
		int newPageNum;

		PF_AllocPage(fileDesc, &newPageNum, &newPageBuf);

		header->nextLeafPage = newPageNum;
		bcopy(header, pageBuf, AM_sl);

		pageBuf = newPageBuf;
		pageNum = newPageNum;
		rightmostPage[0] = newPageNum;
		rightmostBuf[0] = newPageBuf;
		
		InsertIntoLeaf(newPageBuf, value, recId);
		PF_UnfixPage(fileDesc, pageNum, TRUE);
		AddToParent(fileDesc, 0, rightmostPage, rightmostBuf, value, attrLength, length);
	}
	
	AM_EmptyStack();
	return(AME_OK);
	
}

AddToParent(fileDesc, level, rightmost_page, rightmost_buf, value, attrLength, length)
int fileDesc;/* file Descriptor */
int level;/*Level at which the node whose pointer has to be added to parent is present*/
int *rightmost_page;/*Array which stores the page number of right most node at each level*/
char **rightmost_buf;/*Array which stores the buffer pointer of right most node at each level*/
char *value; /* pointer to attribute value to be added*/
int attrLength;/* 4 for ’i’ or ’f’, 1-255 for ’c’ */
int *length;/*Length of rightmost_page and rightmost_buf arrays*/
{	
	// /*Node has no parent*/
	// if((*length) == level + 1){
	// 	char *newPageBuf;
	// 	int newPageNum;
	// 	errVal = PF_AllocPage(fileDesc, &newPageNum, &newPageBuf);
	// 	AM_Check;
	// 	rightmost_page[(*length)] = newPageNum ;
	// 	rightmost_buf[(*length)] = newPageBuf;
	// 	(*length)++;
	// }

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
		// rightmost_buf[level+1] = parentPageBuf;
		return (AME_OK);
	}
	else{
		int newPageNum;
		char* newPageBuf;
		errVal = PF_AllocPage(fileDesc, &newPageNum, &newPageBuf);
		AM_Check; //check if there is no error in the PF layer functionality
		AM_INTHEADER newhead, *newheader;
		newheader = &newhead;


		//NOTE: INCREMENT NUMBER OF NODES, IF MAINTAINED HERE
		
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
        bcopy(parentPageBuf + AM_sint + recSize*(header->numKeys + 1), newPageBuf + AM_sint, AM_si); //NOTE: recSize not initialised
        /*Add value to this newly created parent*/
        AM_AddtoIntPage(newPageBuf, value, parentPageNum, newheader, newheader->numKeys);
        bcopy(newheader, newPageBuf, AM_sint);

       //  /*value of key to be added to parent, i.e., value of rightmost key in left node*/
      	// char* val = (char*)malloc(header->attrLength);
      	// bcopy(parentPageBuf + AM_sint + (header->numKeys)*recSize + AM_si, val, header->attrLength);  
        	

        if(parentPageNum == AM_RootPageNum){ // If a new root needs to be created
        	/*Allocate new root page*/
        	int newRootNum;
        	char* newRootBuf;
        	errVal = PF_AllocPage(fileDesc, &newRootNum, &newRootBuf);
        	AM_Check;

        	//NOTE: INCREMENT NUMBER OF NODES, IF MAINTAINED, HERE

          	/*Fill in new root*/
          	AM_FillRootPage(newRootBuf, parentPageNum, newPageNum, value, header->attrLength, header->maxKeys);

        	/*Modiy rightmost buf and num arrays*/
        	*length = *length + 1;

        	rightmost_page[level+2] = AM_RootPageNum;
        	rightmost_buf[level+2] = newRootBuf;

        	/*level + 1 entries to be set to the newPageBuf and Num*/
	    	rightmost_page[level+1] = newPageNum;
	    	rightmost_buf[level+1] = newPageBuf;

	    	/*Unfix left sibling of new page or left child of new root*/
	    	errVal = PF_UnfixPage(fileDesc, parentPageNum, TRUE);
	    	AM_Check;
        }
        else{
        	/*level + 1 entries to be set to the newPageBuf and Num*/
	    	rightmost_page[level+1] = newPageNum;
	    	rightmost_buf[level+1] = newPageBuf;

	    	/*Unfix left sibling of new page or left child of new root*/
	    	errVal = PF_UnfixPage(fileDesc, parentPageNum, TRUE);
	    	AM_Check;
	    	
	    	errVal = AddToParent(fileDesc, level+1, rightmost_page, rightmost_buf, value, attrLength, length);
	    	AM_Check;	
        }

    	

	}
	return (AME_OK);


}

// Pradyot Prakash ka function
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

	int recSize, numElts, attributeLength;
	AM_LEAFHEADER *header, head;

	bcopy(pageBuf, header, AM_sl);
	attributeLength = header->attrLength;
	recSize = attributeLength + AM_ss;
	numElts = header->numKeys;
	header = &head;

	// inserting the first element
	if(!numElts){
		// assume that space is available since it's the first element

		bcopy(pageBuf, header, AM_sl);
		short recordIdPtr;

		header->keyPtr = header->keyPtr + recSize;
		header->numKeys = header->numKeys + 1;

		// insert the key value into the node
		int offset = pageBuf + AM_sl + numElts*recSize;
		bcopy(value, offset, attributeLength);

		recordIdPtr = header->recIdPtr - AM_si;
		header->recIdPtr = recordIdPtr;

		bcopy((char*)&recordIdPtr, offset + attributeLength, AM_ss);

		bcopy((char*) recId, pageBuf + recordIdPtr, AM_si);

		bcopy(header, pageBuf, AM_sl);

		return TRUE;	

	}

	else{
		// space may or may not be available
		// int present = 0;
		// // AM_Search(fileDesc, attrType,attrLength,value,&pageNum,&pageBuf,&index);
		// // use AM_Search here
		
		// // check if key is present
		// if(present){
			
		// 	if(header->recIdPtr - header->keyPtr < AM_si){
		// 		return FALSE;
		// 	}
		// 	else{
		// 		// code I do not understand
				
		// 		bcopy(pageBuf, header, AM_sl);
		// 		short recordIdPtr;

		// 		header->keyPtr = header->keyPtr + recSize;
		// 		header->numKeys = header->numKeys + 1;

		// 		// insert the key value into the node
		// 		int offset = pageBuf + AM_sl + numElts*recSize;
		// 		bcopy(value, offset, attributeLength);

		// 		recordIdPtr = header->recIdPtr - AM_si;
		// 		header->recIdPtr = recordIdPtr;

		// 		bcopy((char*)&recordIdPtr, offset + attributeLength, AM_ss);

		// 		bcopy((char*) recId, pageBuf + recordIdPtr, AM_si);

		// 		bcopy(header, pageBuf, AM_sl);

		// 		return TRUE;
		// 	}

		// }
		// else{

			if(header->recIdPtr - header->keyPtr < AM_si + recSize){
				return FALSE;
			}
			else{
				// space is available
				bcopy(pageBuf, header, AM_sl);
				short recordIdPtr;

				header->keyPtr = header->keyPtr + recSize;
				header->numKeys = header->numKeys + 1;

				// insert the key value into the node
				int offset = pageBuf + AM_sl + numElts*recSize;
				bcopy(value, offset, attributeLength);

				recordIdPtr = header->recIdPtr - AM_si;
				header->recIdPtr = recordIdPtr;

				bcopy((char*)&recordIdPtr, offset + attributeLength, AM_ss);

				bcopy((char*) recId, pageBuf + recordIdPtr, AM_si);

				bcopy(header, pageBuf, AM_sl);

				return TRUE;

			}
		//}
	}

}
