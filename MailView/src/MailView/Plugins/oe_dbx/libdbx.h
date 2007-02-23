/***************************************************************************
                          libdbx.h  -  Header file for DBX handling Library
                             -------------------
    begin                : April 2001
    copyright            : (C) 2001 by David Smith
    email                : Dave.S@Earthcorp.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
	libdbx - read dbx files as used by Outlook Express 5.0

*/

#ifndef _LIBDBX_H_
#define _LIBDBX_H_

#include <stdio.h>

#define LIBDBX_VERSION "1.0.2"

#ifndef _WIN32
#ifndef FILETIME_DEFINED
#define FILETIME_DEFINED
/*Win32 Filetime struct - copied from WINE*/
typedef struct {
	unsigned int dwLowDateTime;
  unsigned int dwHighDateTime;
} FILETIME;
#endif
#else
#ifndef _FILETIME_
#include <windows.h>
#endif
#define FILETIME_DEFINED
#endif // !defined(_WIN32)

/* Control Structure */
struct dbxcontrolstruct {
	FILE *fd; //file descriptor of the dbx file
	int indexCount; //number of elements in the following array
	int * indexes; //array of indexes
	int type; //type of DBX file
};

typedef struct dbxcontrolstruct DBX;

/* Folder Entity - Extracted from folders.dbx */
struct dbxfolderstruct {
  int  num; //index number of folder
  char type; //is folder or email
	char *name; //name of folder
	char *fname; //filename of the folder
	int id; //numeric id of the folder
	int parentid; //numeric id of the parent folder
};

typedef struct dbxfolderstruct DBXFOLDER;

/* Email Entity - Extracted from a mail type .dbx file */
struct dbxemailstruct {
	int num; //index number of item
  char type; //is folder or email
  char *email; //email contents
  char *psubject; //Processed subject line (without RE: etc...)
  char *subject; //original subject line
  char *messageid; //message's mail id
  char *parent_message_ids; //ids of parents
  char *sender_name;
  char *sender_address;
  char *recip_name;
  char *recip_address;
  FILETIME date; //of what I am unsure
  int  id; //dbx id
  int  data_offset; //offset of the body portion in the dbx file
  int  flag; //flags of email
  char *oe_account_name; //name of the account that accepted this email
  char *oe_account_num; //string representation of the account number (e.g. "00000001")
  char *fetched_server; //name of POP server message came from
};

typedef struct dbxemailstruct DBXEMAIL;

/* Non-Entity - just the header type */
struct dbxnonstruct {
	int num; //not really important
	char type; //used to determine the type of this object
};

typedef struct dbxnonstruct DBXNON;

/* Global Variables */
extern int dbx_errno;

/* DBX Errors */
/*0 - No error. i.e. Success */
#define DBX_NOERROR 0
/*1 - dbx file operation failed (open or close)*/
#define DBX_BADFILE 1
/*2 - Reading of Item Count from dbx file failed */
#define DBX_ITEMCOUNT 2
/*3 - Reading of Index Pointer from dbx file failed */
#define DBX_INDEX_READ 3
/*4 - Number of indexes read from dbx file is less than expected*/
#define DBX_INDEX_UNDERREAD 4
/*5 - Number of indexes read from dbx file is greater than expected*/
#define DBX_INDEX_OVERREAD 5
/*6 - Request was made for index reference greater than exists (subscript out of range)*/
#define DBX_INDEXCOUNT 6
/*7 - Reading of data from dbx file failed */
#define DBX_DATA_READ 7
/*8 - Item is a news item not an email*/
#define DBX_NEWS_ITEM 8

/* Prototypes */
#ifdef __cplusplus
extern "C" {
#endif
DBX *dbx_open(const char*);
DBX *dbx_open_stream(FILE *fp);
int dbx_close(DBX*);
int dbx_free(DBX *dbx, void *item);
int dbx_free_item(void *item);
void* dbx_get(DBX*, int, int);
int dbx_perror(const char*);
int dbx_free(DBX* handle, void *item);
int dbx_get_body(DBX* dbx, int start, char** ptr);
int dbx_get_email_body(DBX *dbx, DBXEMAIL* email);
int dbx_free_email_body(DBXEMAIL* email);
const char* dbx_strerror(int err);

#ifdef __cplusplus
}
#endif

/*Types of DBX file*/
/*0 - Contains emails*/
#define DBX_TYPE_EMAIL 0
/*1 - Contains news group items*/
#define DBX_TYPE_NEWS 1
/*2 - Contains the folder structure of Outlook*/
#define DBX_TYPE_FOLDER 2
/*3 - Contains a none entity structure*/
#define DBX_TYPE_VOID 3

/*Fetch flags for dbx_get*/
/*1<<0 - Fetch the body*/
#define DBX_FLAG_BODY 1<<0


/*Flag defines for email->flag*/
/* 0x80 - Is Seen (10000000) */
#define DBX_EMAIL_FLAG_ISSEEN 0x80

#endif //_LIBDBX_H_
