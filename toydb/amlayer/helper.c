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

	if((*length) == level + 1){
		(*length)++;
		// rightmost_page[(*length)] = ;
		// rightmost_buf[(*length)] = ;
	}



}

InsertIntoLeaf(pageBuf, value, recId)
char *pageBuf;/* buffer where the leaf page resides */
char *value;/* attribute value to be inserted*/
int recId;/* recid of the attribute to be inserted */
{
	AM_LEAFHEADER *header;
	bcopy(pageBuf, header, AM_sl);

	int recSize;

	recSize = header->attrLength + AM_ss;

	if(header->recIdPtr - header->keyPtr < AM_si + AM_ss + recSize){
		return FALSE;
	}
	else{

		header->keyPtr = header->keyPtr + recSize;

		// add the new key
		bcopy(value, pageBuf + AM_sl + header->numKeys*recSize, header->attrLength);

		/* make the head of list NULL*/
		//bcopy((char *)&null,pageBuf+AM_sl+(index-1)*recSize+header->attrLength,AM_ss);

		header->numKeys++;
		bcopy(header, pageBuf, AM_sl);
		return TRUE;
	}

}