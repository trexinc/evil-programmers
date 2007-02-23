/***************************************************************************
                          libdbx.c  -  DBX handling Library
                             -------------------
    begin                : April 2001
    copyright            : (C) 2001 by David Smith
    email                : Dave.S@Earthcorp.Com
    win32.vc6 adapt      : Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//#include <stdio.h>
#include "crt.hpp"
#include <stdlib.h>
#include <string.h>

#include "libdbx.h"
#include "define.h"


int dbx_errno = 0;
//could be 0xE4 or 0x30
#define INDEX_POINTER 0xE4
#define ITEM_COUNT 0xC4

/*Internal Prototypes*/

int _dbx_get (FILE *fp, void *buf, unsigned int size);
int _dbx_getAtPos(FILE *fp, int pos, void* buf, unsigned int size);
int _dbx_getitem (FILE *fp, int pos, void** item, int type, int flags);
int _dbx_getindex(FILE* fd, int pos, DBX *dbx);
int _dbx_getIndexes (FILE* fd, DBX *dbx);
int _dbx_getstruct(FILE *fp, int pos, DBXFOLDER* folder);
int _dbx_get_from_buf(char* buffer, int pos, void** dest, int type, int max);
int _dbx_getBody(FILE *fp, char** x, int ptr);

char * dbx_errmsgs[] = {
	"", //DBX_NOERROR
	"DBX File operation failed. Open or close", //DBX_BADFILE
	"Read of \"Item Count\" from DBX file failed", //DBX_ITEMCOUNT
	"Read of \"Index Pointer\" from DBX file failed", //DBX_INDEX_READ
	"Number of indexes read from dbx file is less than expected", //DBX_INDEX_UNDERREAD
	"Number of indexes read from dbx file is greater than expected", //DBX_INDEX_OVERREAD
	"Request was made for index reference greater than exists (subscript out of range)", //DBX_INDEXCOUNT
	"Reading of data from dbx file failed", //DBX_DATA_READ
	"Item is a news item not an email" //DBX_NEWS_ITEM
};

/* dbx_open - Opens a dbx file and returns a DBX struct to the caller
	@fname - Filename of dbx file to open*/

DBX *dbx_open(const char * fname) {
  FILE *fp;

  DEBUG("[%d] Attempting to open file %s\n", __LINE__,fname);

  if ((fp = fopen(fname, "rb")) ==NULL) {
    DEBUG_WARN3("[%d] Open of file %s failed\n", __LINE__, fname);
    dbx_errno = DBX_BADFILE;
    return NULL;
  }

  return dbx_open_stream(fp);
}

DBX *dbx_open_stream(FILE *fp) {
  DBX *dbx = (DBX*) malloc (sizeof(DBX));
  int signature[4];

  dbx->fd = fp;

  /* SIGNATURE */
  _dbx_getAtPos(dbx->fd,0x0,&signature,16);
  if ((signature[0]==0xFE12ADCF) && (signature[1]==0x6F74FDC5) &&
      (signature[2]==0x11D1E366) && (signature[3]==0xC0004E9A)) {
    /* OE 5 & OE 5 BETA SIGNATURE */
    dbx->type=DBX_TYPE_EMAIL;
  } else
    if ((signature[0]==0x36464D4A) && (signature[1]==0x00010003)) /* OE4 SIGNATURE */ {
      /*It is an OE4 dbx file*/
      WARN("libdbx [Line %d]: File is an OE4 file format. This is unsupported.\n", __LINE__);
      dbx_errno = DBX_BADFILE;
      return NULL;
    } else
      if ((signature[0]==0xFE12ADCF) && (signature[1]==0x6F74FDC6) && /*Difference is C6 instead of C5*/
	  (signature[2]==0x11D1E366) && (signature[3]==0xC0004E9A)) {
	/*It is a Folders.dbx type file*/
	dbx->type=DBX_TYPE_FOLDER;
      } else {
	WARN("libdbx [Line %d]: The signature for file isn't known\n", __LINE__);
	dbx_errno = DBX_BADFILE;
	return NULL;
      }

  if (_dbx_getIndexes(dbx->fd, dbx)) {
    /*dbx_errno is already set by getIndexes*/
    return NULL;
  }

  dbx_errno = DBX_NOERROR;
  return dbx;
}


/* dbx_close - Closes a dbx file and deletes the internal struct
	@dbx - DBX struct associated with dbx */

int dbx_close(DBX *dbx) {
	if (dbx == NULL || dbx->fd == NULL) {
		DEBUG_WARN("[%d] File isn't open when close requested\n", __LINE__);
		dbx_errno = DBX_BADFILE;
		return -1;
	}

	fclose(dbx->fd);
	if (dbx->indexes != NULL) {
		free(dbx->indexes);
	}
	free(dbx);

	dbx_errno = DBX_NOERROR;
	return 0;
}

int dbx_free(DBX *dbx, void *item) {
	return dbx_free_item(item);
}

int dbx_free_item(void *item) {
	DBXNON *it = (DBXNON*)item;
  DBXEMAIL* email;
  DBXFOLDER* fol;

  if (item == NULL)
    return 1;

  if (it->type == DBX_TYPE_EMAIL) {
	  email = (DBXEMAIL*)item;
    if (email->email)
      free(email->email);
    if (email->subject)
      free(email->subject);
    if (email->psubject)
    	free(email->psubject);
    if (email->messageid)
      free(email->messageid);
    if (email->parent_message_ids)
      free(email->parent_message_ids);
    if (email->sender_name)
      free(email->sender_name);
    if (email->sender_address)
      free(email->sender_address);
    if (email->recip_name)
      free(email->recip_name);
    if (email->recip_address)
      free(email->recip_address);
    if (email->oe_account_name)
    	free(email->oe_account_name);
    if (email->oe_account_num)
    	free(email->oe_account_num);
    if (email->fetched_server)
    	free(email->fetched_server);
    free(email);
  } else if (it->type == DBX_TYPE_FOLDER) {
    fol = (DBXFOLDER*)item;
    if (fol->name)
      free(fol->name);
    if (fol->fname)
      free(fol->fname);
    free(fol);
  } else {
  	printf("Aaarghhh. Cannot free an unknown type!\n");
  }
  return 0;
}
/* dbx_get - Gets an item from a dbx file
	@dbx - dbx file to get item from
	@index - item 0..itemcount to fetch
	@flags - which parts to get */

void * dbx_get(DBX *dbx, int index, int flags) {
	int size;
	void * ret = NULL;

	if (dbx == NULL || dbx->fd == NULL) {
		DEBUG_WARN("[%d] File isn't open when a get requested\n", __LINE__);
		dbx_errno = DBX_BADFILE;
		return NULL;
	}

	if (index >= dbx->indexCount || index < 0) {
		DEBUG_WARN("[%d] Request for item greater than the highest or less than zero\n", __LINE__);
		dbx_errno = DBX_INDEXCOUNT;
		return NULL;
	}

	if (dbx->type == DBX_TYPE_EMAIL || dbx->type == DBX_TYPE_FOLDER) {
		size = _dbx_getitem(dbx->fd, dbx->indexes[index], &ret, dbx->type, flags);
		((DBXEMAIL*)ret)->num = index;
	} else {
		DEBUG_WARN("[%d] Request on a folder that has an unknown type\n", __LINE__);
		dbx_errno = DBX_BADFILE;
		return NULL;
	}

	dbx_errno = DBX_NOERROR;
	return ret;
}

/* dbx_perror - Print the error message to stderr
	@str - prepend this message */
int dbx_perror(const char *str) {
	fprintf(stderr, "%s: %s\n", str, dbx_errmsgs[dbx_errno]);
	return 0;
}

/* dbx_get_body - Load the body for the current email
	@dbx - handle for the dbx file
	@start - file offset from email pointer
	@ptr - location to store data */
int dbx_get_body(DBX* dbx, int start, char** ptr) {
	if (dbx == NULL || dbx->fd == NULL) {
		DEBUG_WARN("[%d] Request on invalid dbx handle. Either it is non-existant or closed\n", __LINE__);
		dbx_errno = DBX_BADFILE;
		return -1;
	}
	return _dbx_getBody(dbx->fd, ptr, start);
}

/* dbx_get_email_body - Load the body of an email and store it in the email
	@dbx - handle for the dbx file
	@email - email to fillin
*/
int dbx_get_email_body(DBX *dbx, DBXEMAIL* email) {
	if (dbx == NULL || dbx->fd == NULL) {
		DEBUG_WARN("[%d] Request on invalid dbx handle. Either it is non-existant or closed\n", __LINE__);
		dbx_errno = DBX_BADFILE;
		return -1;
	}
	return _dbx_getBody(dbx->fd, &(email->email), email->data_offset);
}

/* dbx_free_email_body - Clear the body of an email. To be called after dbx_get_email_body
	@email - email to remove body from
*/
int dbx_free_email_body(DBXEMAIL* email) {
	free (email->email);
	email->email = NULL;
	return 0;
}

const char* dbx_strerror(int err) {
	return dbx_errmsgs[err];
}

/* Private Functions */
struct _dbx_tableindexstruct {
	int self;
	int unknown1;
	int anotherTablePtr;
	int parent;
	char unknown2;
	char ptrCount;
	char reserve3;
	char reserve4;
	int indexCount;
};

struct _dbx_indexstruct {
	int indexptr;
	int anotherTablePtr;
	int indexCount;
};

struct _dbx_folder_hdrstruct {
	int self;
	int blocksize;
	short int unknown2;
	char intcount;
	char unknown3;
};

struct _dbx_block_hdrstruct {
	int self;
	int nextaddressoffset;
	short int blocksize;
	char intcount;
	char unknown1;
	int nextaddress;
};

struct _dbx_folderstruct {
	int id;
	int parent;
	int unknown6;
	char unknown61;
	char length1;
	char unknown7;
	char unknown8;
};

struct _dbx_email_headerstruct {
  int self;
  int size;
  short int u1;
  unsigned char count;
  unsigned char u2;
};

/* Email types - values thereof
	 0x01 - buffer pointer to Flag (char?)
   0x02 -
   0x04 - buffer pointer to file offset of email data
   0x05 - buffer pointer to asciiz string containing the subject of email
   0x06 -
   0x07 - buffer pointer to asciiz message id of email
   0x08 - buffer pointer to asciiz another string containing the subject of email
   0x09 -
   0x0B -
   0x0A - buffer pointer to asciiz message ids of parent emails
   0x0C - buffer pointer to asciiz name of server where email was fetched from
   0x0D - buffer pointer to asciiz Name of sender
   0x0E - buffer pointer to asciiz Email address of sender
   0x11 -
   0x12 -
   0x13 - buffer pointer to asciiz Name of recipient
   0x14 - buffer pointer to asciiz Email address of recipient
   0x1A - buffer pointer to asciiz name of email account used to fetch email
   0x1B - buffer pointer to asciiz number of email account (e.g. "00000001")
   0x1C -
   0x80 -
   0x81 - email's flag (char?)
   0x84 - file offset to email data
   0x90 -
   0x91 -
*/

/* Folder types - values thereof
   0x02 - Descriptive Name
   0x03 - Filename
   0x80 - Folder ID
   0x81 - ID of parent
*/

struct _dbx_email_pointerstruct {
  unsigned char type;
  int val; //this is supposed to be a 3 byte int
};


int _dbx_getIndexes (FILE* fp, DBX *dbx) {
	int indexptr;
	int itemcount;

	//first table of indexes
	if (_dbx_getAtPos(fp, INDEX_POINTER, &indexptr, sizeof(indexptr))) {
		DEBUG_WARN("[%d] Failed to read Index Pointer\n", __LINE__);
		dbx_errno = DBX_INDEX_READ;
		return 2;
	}

	//count of items
	if (_dbx_getAtPos(fp, ITEM_COUNT, &itemcount, sizeof(itemcount))) {
		DEBUG_WARN("[%d] Failed to read itemcount\n", __LINE__);
		dbx_errno = DBX_ITEMCOUNT;
		return 1;
	}

	DEBUG_INDEX("[%d] ItemCount = %d\n",__LINE__, itemcount);

	dbx->indexes = (int*) malloc(itemcount*sizeof(int));
	dbx->indexCount = itemcount;

	if (_dbx_getindex(fp, indexptr, dbx)) {
		return 4;
	}

	if (dbx->indexCount != 0) {
		DEBUG_WARN("[%d] The indexcount specified that more indexes exist than have been read\n", __LINE__);
		dbx_errno = DBX_INDEX_UNDERREAD;
		return 3;
	}

	dbx->indexCount = itemcount; //reassign itemcount after call cause it should equal zero now
	return 0;
}

int _dbx_getindex(FILE* fp, int pos, DBX *dbx) {
	int x;
	struct _dbx_tableindexstruct tindex;
	struct _dbx_indexstruct index;

	DEBUG_INDEX("[%d] Reading index from %X\n", __LINE__, pos);

	RET_DERROR4(_dbx_getAtPos(fp, pos, &tindex, sizeof(tindex)), DBX_INDEX_READ,
				"[%d] Failed to read table index structure\n", __LINE__);

	DEBUG_INDEX("tindex.indexCount = %d\ntindex.anotherTablePtr = %d\n", tindex.indexCount, tindex.anotherTablePtr);

	if (tindex.indexCount > 0) {
		DEBUG_INDEX2("[%d] Recursing to get more indexes\n", __LINE__);
		_dbx_getindex (fp, tindex.anotherTablePtr, dbx);
	}

	pos += sizeof(struct _dbx_tableindexstruct);

	DEBUG_INDEX("[%d] ptrCount = %d\n", __LINE__, tindex.ptrCount);

	for (x = 1; x <= tindex.ptrCount; x++) {

		RET_DERROR(_dbx_getAtPos(fp, pos, &index, sizeof(struct _dbx_indexstruct)), DBX_INDEX_READ,
					"[%d] Failed to read index structure at pos %d\n", __LINE__, pos);

		RET_DERROR4(dbx->indexCount < 0, DBX_INDEX_OVERREAD,
						"[%d] Read too many indexes\n", __LINE__);

		dbx->indexes[--dbx->indexCount] = index.indexptr;
		DEBUG_INDEX4("[%d] Adding index pointer of %X at pos %d\n", __LINE__, index.indexptr, dbx->indexCount);

		pos += sizeof(struct _dbx_indexstruct);
		if (index.indexCount > 0)
			_dbx_getindex(fp, index.anotherTablePtr, dbx);
	}

	return 0;
}

#define STRING_TYPE 0
#define INT_TYPE 1
#define W32FT_TYPE 2
#define CHAR_TYPE 3

int _dbx_getitem (FILE *fp, int pos, void **item, int type, int flags) {
	int x;
	char *bufptr, *buffer, **bufx;
	int readtype=STRING_TYPE;

	DBXEMAIL *email = NULL;
	DBXFOLDER *folder = NULL;

	struct _dbx_email_headerstruct blockhdr;
	struct _dbx_email_pointerstruct blockp;

	int body = (flags&DBX_FLAG_BODY?1:0);

	if (type == DBX_TYPE_EMAIL) {
	  email = (DBXEMAIL*) malloc(sizeof(DBXEMAIL));
	  memset (email, 0, sizeof(DBXEMAIL));
	  email->type = DBX_TYPE_EMAIL;
	  *item = email;
	  email->email = NULL;
	} else {
	  folder = (DBXFOLDER*) malloc(sizeof(DBXFOLDER));
	  memset (folder, 0, sizeof(DBXFOLDER));
	  folder->type = DBX_TYPE_FOLDER;
	  *item = folder;
	}

	DEBUG_EMAIL("[%d] Reading from pos %#X\n", __LINE__, pos);

	RET_DERROR(_dbx_getAtPos(fp, pos, &blockhdr, sizeof(blockhdr)), DBX_INDEX_READ,
		   "[%d] Failed to read header of email block at pos %d\n", __LINE__, pos);

	//we will load all the block into memory as we will be accessing it byte by byte
	DEBUG_EMAIL("[%d] Creating block buffer of %d\n", __LINE__, blockhdr.size);
	buffer = (char*) malloc(blockhdr.size);

	RET_DERROR6(_dbx_get(fp, buffer, blockhdr.size), DBX_DATA_READ,
		   "[%d] Failed to read datablock of size %d from pos %d\n", __LINE__, blockhdr.size, pos+sizeof(struct _dbx_email_headerstruct));

	bufptr = buffer;
	if (email != NULL)
		email->data_offset = -1;

	for (x = 0; x < blockhdr.count; x++) {
	  blockp.val = 0;

	  memcpy(&(blockp.type), bufptr, 1); //this will copy the type
	  memcpy(&(blockp.val), bufptr+1, 3); //and the 3 byte int

	  DEBUG_EMAIL4("[%d] type = %#X value = %#X\n", __LINE__, blockp.type, blockp.val);
	  if (type == DBX_TYPE_EMAIL) {
	    switch (blockp.type) {
	    case 0x01: //pointer to flag
	    	email->flag = 0;
	    	//((int*)bufx) = &(email->flag);
                bufx = (char**)&(email->flag); // !!! Den !!!
	    	readtype = CHAR_TYPE;
	    	break;
	    case 0x04: //pointer to dataptr
	      //((int*)bufx) = &(email->data_offset);
              bufx = (char**)&(email->data_offset); // !!! Den !!!
	      readtype = INT_TYPE;
	      break;
	    case 0x05: //asciiz string of subject (without RE: or FWD: etc...)
	      bufx = &(email->psubject);
	      readtype = STRING_TYPE;
	      break;
	    case 0x07: //message id of email
	      bufx = &(email->messageid);
	      readtype = STRING_TYPE;
	      break;
	    case 0x08: //second copy of subject.Original text (with RE: and FWD: etc...)
	      bufx = &(email->subject);
	      readtype = STRING_TYPE;
	      break;
	    case 0x0A: //msg-id of parent(s)
	      bufx = &(email->parent_message_ids);
	      readtype = STRING_TYPE;
	      break;
	    case 0x0C: //name of server used to fetch email
	    	bufx = &(email->fetched_server);
	    	readtype = STRING_TYPE;
	    	break;
	    case 0x0D: //Sender's name
	      bufx = &(email->sender_name);
	      readtype = STRING_TYPE;
	      break;
	    case 0x0E: //Sender's email address
	      bufx = &(email->sender_address);
	      readtype = STRING_TYPE;
	      break;
	    case 0x12: //date - of what i'm not sure. It is in a win32 FILETIME structure. needs converting to something
#ifdef _WIN32
				//((FILETIME*)bufx) = &(email->date);
                                bufx = (char**)&(email->date);
#else
				((struct FILETIME*)bufx) = &(email->date);
#endif
				readtype = W32FT_TYPE;
				break;
	    case 0x13: //recipient's name
	      bufx = &(email->recip_name);
	      readtype = STRING_TYPE;
	      break;
	    case 0x14: //recipient's email address
	      bufx = &(email->recip_address);
	      readtype = STRING_TYPE;
	      break;
	    case 0x1A: //Name of Account used to fetch email
	    	bufx = &(email->oe_account_name);
	    	readtype = STRING_TYPE;
	    	break;
	    case 0x1B: //String version of account number used to fetch email (eg "00000001")
	    	bufx = &(email->oe_account_num);
	    	readtype = STRING_TYPE;
	    	break;
	    case 0x80: //email's ID
	      bufx = NULL;
	      email->id = blockp.val;
	      break;
	    case 0x81: //email's flag
	    	bufx=NULL;
	    	email->flag = blockp.val;
	    	break;
	    case 0x84: //direct offset of first email data block
	      email->data_offset = blockp.val;
	      bufx = NULL;
	      break;
	      /*	    case 0x02: //currently unknown
	    case 0x06: //currently unknown
	    case 0x09:
	    case 0x0B:
	    case 0x0C:
	    case 0x11: //currently unknown
	    case 0x1A:
	    case 0x1B:
	    case 0x1C: //currently unknown
	    case 0x81: //currently unknown
	    case 0x90: //currently unknown
	    case 0x91: //currently unknown
	      bufx = NULL;
	      break;*/
	    default:
	      DEBUG_WARN4("[%d] Come across unknown type %#X with val of %#X in blockp\n", __LINE__, blockp.type, blockp.val);
	      bufx = NULL;
	    }
	  } else {
	    switch(blockp.type) {
	    case 0x02: //descriptive name
	      bufx = &(folder->name);
	      readtype = STRING_TYPE;
	      break;
	    case 0x03: //filename
	      bufx = &(folder->fname);
	      readtype = STRING_TYPE;
	      break;
	    case 0x80: //current id
	      bufx = NULL;
	      folder->id = blockp.val;
	      break;
	    case 0x81: //parent id
	      bufx = NULL;
	      folder->parentid = blockp.val;
	      break;
	      /*	    case 0x86: //unknown
	    case 0x87: //unknown
	    case 0x88: //unknown
	    case 0x8A: //unknown
	    case 0x8B: //unknown
	      bufx = NULL;
	      break;*/
	    default:
	      DEBUG_WARN4("[%d] Come across unknown type %#X with val of %#X in blockp\n", __LINE__, blockp.type, blockp.val);
	      bufx = NULL;
	    }
	  }

	  if (bufx != NULL)
	    if (_dbx_get_from_buf(buffer, blockp.val + (blockhdr.count*4), (void**)bufx, readtype, blockhdr.size))
	      return 1; //an error occured

	  bufptr += 4; //size of data
	}
	free (buffer);
	/* if we are doing folder types, we have now finished */
	if (type == DBX_TYPE_FOLDER || body == 0)
	  return 0;

	RET_DERROR(email->data_offset == -1, DBX_DATA_READ,
		   "[%s:%d] Dataptr hasn't been set for current email\n", __FILE__, __LINE__);

	return _dbx_getBody(fp, &(email->email), email->data_offset);
}

int _dbx_getBody(FILE *fp, char** x, int ptr) {
	int bufsize = 0;
	struct _dbx_block_hdrstruct hdr;
	*x = NULL;
	while (ptr != 0) {

	RET_DERROR4(_dbx_getAtPos(fp, ptr, &hdr, sizeof(hdr)), DBX_DATA_READ,
		     "[%d] Failed to read datalength\n", __LINE__);
//		printf("Read header\n");
	  //this plus one will not be accumulative
	  //cause we don't add it to bufsize but we need it so we can terminate the buffer
	  *x = (char*)realloc(*x, bufsize + hdr.blocksize + 1);

	  RET_DERROR4(_dbx_get(fp, (*x)+bufsize, hdr.blocksize), DBX_DATA_READ,
		     "[%d] Failed to read data\n", __LINE__);

	  bufsize += hdr.blocksize;
	  ptr = hdr.nextaddress;
	}
	if (*x != NULL)
		(*x)[bufsize] = '\0'; //terminate the buffer

	return bufsize; //size of data read
}

int _dbx_getstruct(FILE *fp, int pos, DBXFOLDER* folder) {
	struct _dbx_folder_hdrstruct hdr;
	struct _dbx_folderstruct fol;

	char *buf, *fname;
	int msgoffset, blockpos=0;

	folder->name = NULL;
	DEBUG("[%d] Reading from pos %X\n", __LINE__, pos);

	RET_DERROR(_dbx_getAtPos(fp, pos, &hdr, sizeof(hdr)), DBX_DATA_READ,
					"[%d] Failed to read Folder Header at pos %d\n", __LINE__,pos);

	RET_DERROR(_dbx_get(fp, &fol, sizeof(fol)), DBX_DATA_READ,
					"[%d] Failed to read Folder Header at pos %d\n", __LINE__, pos);

	blockpos += sizeof(hdr);
	buf = (char*) malloc(fol.length1);
	msgoffset = hdr.intcount * sizeof(int);
	DEBUG("[%d] Msgoffset = %d\n", __LINE__, msgoffset);

	RET_DERROR(_dbx_getAtPos(fp, pos+blockpos+msgoffset, buf, fol.length1), DBX_DATA_READ,
					"[%d] Failed to read Folder name at pos %d\n", __LINE__, pos+msgoffset);

	if (strlen(buf) != fol.length1-1){
		WARN("libdbx: [%d] Length of read foldername not equal to expectation\n", __LINE__);
	}

	/*Allocate space big enough to hold remainder of block*/
	fname = (char*) malloc(hdr.blocksize-blockpos);
	if (fname == NULL) {
		WARN("libdbx: [%d] We must have read past the end of the blocksize\n", __LINE__);
		return -1;
	}

	RET_DERROR4(_dbx_get(fp, fname, hdr.blocksize-blockpos), DBX_DATA_READ,
					"[%d] Failed to read filename of folder\n", __LINE__);

	DEBUG4("[%d] name = %s\t fname = %s\n", __LINE__, buf, fname);
	folder->name = buf;
	folder->fname = fname;
	folder->id = fol.id;
	folder->parentid = fol.parent;
	dbx_errno = DBX_NOERROR;
	return strlen(buf);
}

int _dbx_getAtPos(FILE *fp, int pos, void* buf, unsigned int size) {
	if (fseek(fp, pos, SEEK_SET) == -1) {
		return 1;
	}

	if (fread(buf, 1, size, fp) < size) {
		return 2;
	}
	return 0;
}

int _dbx_get (FILE *fp, void *buf, unsigned int size) {
	if (fread(buf, 1,  size, fp) < size) {
		return 1;
	}
	return 0;
}

int _dbx_get_from_buf(char* buffer, int pos, void** dest, int type, int max) {
  int y;
  //copy data from buffer to string pointed to by bufx
  if (type == STRING_TYPE) {
    y = strlen(&buffer[pos])+1; //plus one for string terminator
    DEBUG_EMAIL4("[%d] copying values [%d bytes] from buffer starting from %d\n", __LINE__, y, pos);

    RET_DERROR6(y > max, DBX_DATA_READ,
	       "[%d] Length of string being read[%d] from data buffer is greater than buffer size[%d]", __LINE__, y, max);

    if (*dest == NULL)
    	*dest = (char*) malloc(y);
    strncpy( (char*)*dest, &buffer[pos], y );
    DEBUG_EMAIL("[%d] value copied was %s\n", __LINE__, (char*)*dest);
  } else if (type == INT_TYPE) {
    memcpy((int*)dest, &buffer[pos], 4);
    DEBUG_EMAIL("[%d] value copied was %#X\n", __LINE__, (int)*dest);
  } else if (type == W32FT_TYPE) {
		memcpy((FILETIME*)dest, &buffer[pos], 8);
//		DEBUG_EMAIL("[%d] value copied was %#X%X\n", __LINE__, (struct FILETIME*)dest->dwHighDateTime, (struct FILETIME*)dest->dwLowDateTime);
	} else if (type == CHAR_TYPE) {
		memcpy((unsigned char*)dest, &buffer[pos], 1);
		DEBUG_EMAIL("[%d] value copied was %i\n", __LINE__, (unsigned char)*((unsigned char*)dest));
	}

  return 0;
}
