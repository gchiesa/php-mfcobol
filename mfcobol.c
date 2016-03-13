/**
* Copyright 2007, 2008 - Giuseppe Chiesa
*
* This file is part of mfcobol.
*
* mfcobol is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* mfcobol is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with sp2html; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Created by: Giuseppe Chiesa - http://gchiesa.smos.org
*
**/

/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header,v 1.16.2.1.2.1 2007/01/01 19:32:09 iliaa Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_mfcobol.h"

#include "isinternal.h"  /* vbisam inclusion */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>


typedef struct mfc_db_resource {
    
    int     MFCVarLen; // flag che specifica se la  risorsa è aperta con record variabili
    int     MFCHandle; // handle al db     
} MFCDBRESOURCE;

typedef struct mfc_key_resource {
    
    struct keydesc keydesc;
} MFCKEYRESOURCE;


#define MFCDBRESOURCE_RES_NAME "MicroFocusCobol Database Resource"      // RISORSA PERSISTENTE
#define MFCKEYRESOURCE_RES_NAME "MicroFocusCobol Key Resource"          // RISORSA KEY DESCRIPTION


static int le_mfcdbresource;    /* identificativo risorsa persistente */
static int le_mfckeyresource;   /* identificativo risorsa key */


/* If you declare any globals in php_mfcobol.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(mfcobol)
*/




/* defines for utilities */
#define MFC_VARLEN  1   // used in mfcdb_insert
#define MFC_FIXLEN  2   // used in mfcdb_insert

#define MFC_COMP3_TYPE_C 0x43
#define MFC_COMP3_TYPE_D 0x44
#define MFC_COMP3_TYPE_F 0x46
#define MFC_FALSE   0
#define MFC_TRUE    1

/* declarations of utilities */
int     mfc_util_hex2dec(char *text, int size);
int     mfc_util_hex2dec_byte(char c);
char*   mfc_util_printhex(char *txt, int len);
char*   mfc_util_comp3decode(char *txt, int len);
char*   mfc_util_comp3encode(char *txt, int comp3_type, int byte_len);




/* {{{ mfcobol_functions[]
 *
 * Every user visible function must have an entry in mfcobol_functions[].
 */
zend_function_entry mfcobol_functions[] = {
	PHP_FE(confirm_mfcobol_compiled,	NULL)		/* For testing, remove later. */
    PHP_FE(mfcdb_key,           NULL)
    PHP_FE(mfcdb_create,        NULL)
	PHP_FE(mfcdb_open,	        NULL)
	PHP_FE(mfcdb_close,	        NULL)
	PHP_FE(mfcdb_insert,	    NULL)
	PHP_FE(mfcdb_exists,	    NULL)
	PHP_FE(mfcdb_fetch,	        NULL)
	PHP_FE(mfcdb_start,	        NULL)
    PHP_FE(mfcdb_curr,	        NULL)
	PHP_FE(mfcdb_next,	        NULL)
	PHP_FE(mfcdb_prev,	        NULL)
	PHP_FE(mfcdb_replace,	    NULL)
	PHP_FE(mfcdb_delete,	    NULL)
	PHP_FE(mfcdb_get_info,	    NULL)
    PHP_FE(mfcdb_get_recnum,    NULL)
    PHP_FE(mfcdb_comp3encode,   NULL)
    PHP_FE(mfcdb_comp3decode,   NULL)
    PHP_FE(mfcdb_error,         NULL)
	{NULL, NULL, NULL}	/* Must be the last line in mfcobol_functions[] */
};
/* }}} */

/* {{{ mfcobol_module_entry
 */
zend_module_entry mfcobol_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"mfcobol",
	mfcobol_functions,
	PHP_MINIT(mfcobol),
	PHP_MSHUTDOWN(mfcobol),
	PHP_RINIT(mfcobol),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(mfcobol),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(mfcobol),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MFCOBOL
ZEND_GET_MODULE(mfcobol)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("mfcobol.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_mfcobol_globals, mfcobol_globals)
    STD_PHP_INI_ENTRY("mfcobol.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_mfcobol_globals, mfcobol_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_mfcobol_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_mfcobol_init_globals(zend_mfcobol_globals *mfcobol_globals)
{
	mfcobol_globals->global_value = 0;
	mfcobol_globals->global_string = NULL;
}
*/
/* }}} */




/** 
 * le_mfcdbresource_dtor : destructor of MFCDBRESOURCE type
 */
void le_mfcdbresource_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    MFCDBRESOURCE *db;
    
    /* prendo il riferimento alla risorsa */
    db = (MFCDBRESOURCE*) rsrc->ptr;
    
    /* distruggo la risorsa */
    efree(db);
}




/** 
 * le_mfckeyresource_dtor : destructor of MFCKEYRESOURCE type
 */
void le_mfckeyresource_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    MFCKEYRESOURCE *key ;
    
    /* prendo il riferimento alla risorsa */
    key = (MFCKEYRESOURCE*) rsrc->ptr;
    
    /* libero la memoria */
    efree(key);
}




/**
 * PHP_MINIT_FUNCTION(mfcobol)
 */
PHP_MINIT_FUNCTION(mfcobol)
{
    /*
     * registro i distruttori delle risorse 
     */
    le_mfcdbresource = zend_register_list_destructors_ex(le_mfcdbresource_dtor, NULL, MFCDBRESOURCE_RES_NAME, module_number);
    le_mfckeyresource = zend_register_list_destructors_ex(le_mfckeyresource_dtor, NULL, MFCKEYRESOURCE_RES_NAME, module_number);
       
    
    /* REGISTRO LE COSTANTI */
    
    /* usate in comp-3 encode/decode */
    REGISTER_LONG_CONSTANT("MFC_COMP3_TYPE_C", (long) MFC_COMP3_TYPE_C, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_COMP3_TYPE_D", (long) MFC_COMP3_TYPE_D, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_COMP3_TYPE_F", (long) MFC_COMP3_TYPE_F, CONST_CS | CONST_PERSISTENT);
    
    /* usate in open */
    REGISTER_LONG_CONSTANT("MFC_VARLEN", (long) MFC_VARLEN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_FIXLEN", (long) MFC_FIXLEN, CONST_CS | CONST_PERSISTENT);
    
    /* usate in key */
    REGISTER_LONG_CONSTANT("MFC_CHARTYPE", (long) CHARTYPE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_INTTYPE", (long) INTTYPE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_LONGTYPE", (long) LONGTYPE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_FLOATTYPE", (long) FLOATTYPE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_QUADTYPE", (long) QUADTYPE, CONST_CS | CONST_PERSISTENT);
    
    REGISTER_LONG_CONSTANT("MFC_ISNODUPS", (long) ISNODUPS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_ISDUPS", (long) ISDUPS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_DCOMPRESS", (long) DCOMPRESS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_LCOMPRESS", (long) LCOMPRESS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_TCOMPRESS", (long) TCOMPRESS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_COMPRESS", (long) COMPRESS, CONST_CS | CONST_PERSISTENT);
    
    /* usate in start */
    REGISTER_LONG_CONSTANT("MFC_ISFIRST", (long) ISFIRST, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_ISLAST", (long) ISLAST, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_ISEQUAL", (long) ISEQUAL, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_ISGREAT", (long) ISGREAT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("MFC_ISGTEQ", (long) ISGTEQ, CONST_CS | CONST_PERSISTENT);
    
    return SUCCESS;
}




/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(mfcobol)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */




/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(mfcobol)
{

    
	return SUCCESS;
}
/* }}} */




/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(mfcobol)
{
	return SUCCESS;
}
/* }}} */




/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(mfcobol)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "mfcobol support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */




/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_mfcobol_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_mfcobol_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "mfcobol", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/




/**
 * PHP_FUNCTION(mfcdb_key) : create a key resource in order to create db or add keys
 */
/* {{{ proto resource mfcdb_key(int key_flags, int key_nparts, int kp_start, int kp_len, int kp_type, ...)
   create a key resource in order to create db or add keys */
PHP_FUNCTION(mfcdb_key)
{
    int argc = ZEND_NUM_ARGS();
    int kparts = 0;
    char buff[512];
    zval ***args;


    /*-------------------------------------------
    -- CHECK ARGS
    -------------------------------------------*/
    
    if(argc < 5) {
        
        php_error(E_WARNING, "mfcdb_key: this function requies at least 5 parameters");
        RETURN_FALSE;
    
    }

    if((argc - 2) % 3) {
        
        php_error(E_WARNING, "mfcdb_key: error in argument list... maybe you have forgot some key properties?");
        RETURN_FALSE;

    }
    
    args = (zval ***) safe_emalloc(sizeof(zval **), ZEND_NUM_ARGS(), 0);
    
    if(zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE) {
        
        efree(args);
        WRONG_PARAM_COUNT;
        
    }

    convert_to_long_ex(args[1]);
    kparts = Z_LVAL_P(*args[1]);

#ifdef MFC_DDEBUG
printf("mfcdb_key: keyparts da processare %d\n", kparts);
#endif 

    if( ((kparts * 3) + 2) != argc) {
        
        sprintf(buff, "mfcdb_key: you have declared %d  keyparts (total %d args) but passed only %d args", kparts, ((kparts * 3) + 2), argc);
        php_error(E_WARNING, buff);
        efree(args);
        RETURN_FALSE;
        
    }
    
    /*-------------------------------------------
    -- MY DECLARATIONS 
    -------------------------------------------*/
    MFCKEYRESOURCE *key;
    int i, j, key_len;

    
    /* alloco la risorsa per la key */
    key = (MFCKEYRESOURCE*) emalloc(sizeof(MFCKEYRESOURCE));
    memset(key, 0, sizeof(key));
    
    key_len = j = 0; 
    for(i = 2; i < ((kparts * 3) + 2) ; i++) {
        
        switch((i - 2) % 3) {
            
            case 0:         /* kp_start */
                convert_to_long_ex(args[i]);
                key->keydesc.k_part[j].kp_start = Z_LVAL_P(*args[i]);
                break;
            case 1:         /* kp_len */
                convert_to_long_ex(args[i]);
                key->keydesc.k_part[j].kp_leng = Z_LVAL_P(*args[i]);
                key_len += key->keydesc.k_part[j].kp_leng;
                break;
            case 2:
                convert_to_long_ex(args[i]);
                key->keydesc.k_part[j++].kp_type = Z_LVAL_P(*args[i]);
                break;
            default:
                php_error(E_ERROR, "mfcdb_key: unattended error processing key");
                RETURN_FALSE;
                
        } /* switch */
        
#ifdef MFC_DDEBUG
printf("mfcdb_key: scelta %d, key_len %d, keypart j %d, i %d, val  %d\n", (i - 2) % 3, key_len, j-1, i,   Z_LVAL_P(*args[i]));
#endif
        
    } /* for */
    
    
    /* setto le flag per la key */
    convert_to_long_ex(args[0]);
    key->keydesc.k_flags = Z_LVAL_P(*args[0]);
    
    /* setto la lunghezza della key */
    key->keydesc.k_len = key_len;
    
    /* setto il numero di key_parts */
    convert_to_long_ex(args[1]);
    key->keydesc.k_nparts = kparts;

#ifdef MFC_DDEBUG
printf("mfcdb_key: k_flags %d, k_len %d, k_nparts %d\n", key->keydesc.k_flags, key->keydesc.k_len, key->keydesc.k_nparts);
#endif
    
    efree(args);    /* array lista di argomenti */
    ZEND_REGISTER_RESOURCE(return_value, key, le_mfckeyresource);
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_create) : crea il database 
 */
/* {{{ proto resource mfcdb_create(string dbfile, int rec_len, resource primary_key, int db_flags)
   crate a new table */
PHP_FUNCTION(mfcdb_create)
{
	char *dbfile = NULL;
	int argc = ZEND_NUM_ARGS();
	int dbfile_len;
    int rec_len, db_flags = -1;
    zval *handle = NULL;

    if (zend_parse_parameters(argc TSRMLS_CC, "slr|l", &dbfile, &dbfile_len, &rec_len, &handle, &db_flags) == FAILURE) 
		return;
    
    /*-------------------------------------------
    -- CHECK ARGS / CODE 
    -------------------------------------------*/
    FILE *fi, *fd;
    MFCKEYRESOURCE *key;
    MFCDBRESOURCE *db;
    char buff[512];
    int result;
    struct keydesc kd;
    
    /* controllo se esistono i files */
    snprintf(buff, dbfile_len, "%s.dat", dbfile);
    fi = fopen(buff, "r");
    snprintf(buff, dbfile_len, "%s.idx", dbfile);
    fd = fopen(buff, "r");
    
    if((fi != NULL) || (fd != NULL)) {

        fclose(fi);
        fclose(fd);
        php_error(E_WARNING, "mfcdb_create: file dat or idx exists");
        RETURN_FALSE;
    
    }

    /* recupero la risorsa alla key */
    ZEND_FETCH_RESOURCE(key, MFCKEYRESOURCE*, &handle, -1, MFCKEYRESOURCE_RES_NAME, le_mfckeyresource);
    
    /* verifico che la primary key non abbia flag duplicata */ 
    if(key->keydesc.k_flags & ISDUPS) {
        
        php_error(E_WARNING, "mfcdb_create: can't create database with primary key containing DUPS flag");
        RETURN_FALSE;
    }
    
    /* verifico che la lunghezza key non sia maggiore della lunghezza record */
    if(key->keydesc.k_len > rec_len) {
        
        sprintf(buff, "mfcdb_create: error, key length (%d) is greater of reclen(%d)", key->keydesc.k_len, rec_len);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
    
    }
    
    /* verifico le db_flags */
    if(db_flags == -1) {
        
        db_flags = ISINOUT + ISNOLOG + ISFIXLEN + ISEXCLLOCK;
    
    }

#ifdef MFC_DDEBUG
printf("mfcdb_create: k_len %d, k_nparts %d, k_flags %d, kpart0_start %d, kpart0_len %d, kpart0_type %d, kpart1_start %d, kpart1_len %d, kpart1_type %d\n", key->keydesc.k_len,key->keydesc.k_nparts, key->keydesc.k_flags,key->keydesc.k_part[0].kp_start,key->keydesc.k_part[0].kp_leng,key->keydesc.k_part[0].kp_type, key->keydesc.k_part[1].kp_start, key->keydesc.k_part[1].kp_leng, key->keydesc.k_part[1].kp_type);
#endif


    snprintf(buff, dbfile_len, "%s", dbfile);
    result = isbuild(dbfile, rec_len, &key->keydesc, db_flags);
    
    if(result == -1) {
        
        sprintf(buff, "mfcdb_create: error creating db, errno is %d", iserrno);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
    
    }
    
    /* se qui, allora tutto ok alloco la risorsa per l'handle db */
    db = (MFCDBRESOURCE*) emalloc(sizeof(MFCDBRESOURCE));
    memset(db, 0, sizeof(db));
    
    db->MFCHandle = result;
    db->MFCVarLen = (db_flags & ISVARLEN)?1:0; /* -- TODO -- verificare nella gestione varlen */
    
    /* elimino la risorsa key che a questo punto non è più necessaria */
    zend_list_delete(Z_RESVAL(*handle));
    
    /* registro la risorsa */
    ZEND_REGISTER_RESOURCE(return_value, db, le_mfcdbresource);
}
/* }}} */

    
    

/**
 * PHP_FUNCTION(mfcdb_open) : apre il database
 * @return resource handle del database aperto
 */
/* {{{ proto resource mfcdb_open(string dbfile)
   open a database table */
PHP_FUNCTION(mfcdb_open)
{
	char *dbfile = NULL;
	int argc = ZEND_NUM_ARGS();
	int dbfile_len;

	if (zend_parse_parameters(argc TSRMLS_CC, "s", &dbfile, &dbfile_len) == FAILURE) 
		return;

    /*-------------------------------------------
    -- MY DECLARATIONS 
    -------------------------------------------*/
    MFCDBRESOURCE *db;
    int result;
    char buff[1024];

    
    db = (MFCDBRESOURCE*) emalloc(sizeof(MFCDBRESOURCE));
    memset(db, 0, sizeof(MFCDBRESOURCE));
    
    result = isopen(dbfile, ISINOUT + ISNOLOG + ISFIXLEN + ISAUTOLOCK);
    
#ifdef MFC_DEBUG
printf("mfcdb_open: result %d\n", result);
#endif

    if(result == -1) {
        sprintf(buff, "mfcdb_open: error opening database (%d). errno: %d\n", result, iserrno);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
    }
    
    /* salvo le varie struct nella risorsa del db */
    db->MFCHandle = result;
    db->MFCVarLen = 0; // -- TODO -- modificare quando si implementeranno i record variabili
    
    // mfc_save_resource(db);
    
    ZEND_REGISTER_RESOURCE(return_value, db, le_mfcdbresource);
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_close) : chiude la risorsa al db
 * @return TRUE | FALSE 
 */
/* {{{ proto bool mfcdb_close(resource handle)
   close the table of database */
PHP_FUNCTION(mfcdb_close)
{
	int argc = ZEND_NUM_ARGS();
	int handle_id = -1;
	zval *handle = NULL;

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &handle) == FAILURE) 
		return;

    /*-------------------------------------------
    -- MY DECLARATIONS/CODE 
    -------------------------------------------*/
    MFCDBRESOURCE *db;
    int result;
    long ref;
    char buff[1024];
    
    ZEND_FETCH_RESOURCE(db, MFCDBRESOURCE*, &handle, -1, MFCDBRESOURCE_RES_NAME, le_mfcdbresource);
    
    // mfc_load_resource(db);
    
    result = isclose(db->MFCHandle);
    
    if(result) {
        sprintf(buff, "mfcdb_close: error closing resource(%d) get value %d, errno %d\n", handle, result, iserrno);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
    }
    
    /* richiamo il distruttore della risorsa MFCDBRESOURCE */
    zend_list_delete(Z_RESVAL(*handle));
    
    
    RETURN_TRUE;
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_insert) : inserisco un record nella tabella
 */
/* {{{ proto int mfcdb_insert(resource handle, string record [, int is_varlen])
   insert a new record into table */
PHP_FUNCTION(mfcdb_insert)
{
    zval *handle = NULL;
	char *value = NULL;
	int argc = ZEND_NUM_ARGS();
	int value_len;
	long is_varlen;

	if (zend_parse_parameters(argc TSRMLS_CC, "rs|l", &handle, &value, &value_len, &is_varlen) == FAILURE) 
		return;

    /*-------------------------------------------
    -- CHECK ARGS
    -------------------------------------------*/
    

    if(argc < 2) {
    
        php_error(E_ERROR, "mfcdb_insert: this function requires 2 arguments");
        RETURN_FALSE;
    
    }
    
    if(argc == 2) {
        
        is_varlen = 0;
        
    } else {
        
        if(is_varlen == MFC_VARLEN) // lunghezza record variabile  
            is_varlen = 1;
    }
    
    
    /*-------------------------------------------
    -- MY DECLARATIONS/CODE
    -------------------------------------------*/
    MFCDBRESOURCE *db;
    char buff[1024];
    int result;

    /* recupero la risorsa */
    ZEND_FETCH_RESOURCE(db, MFCDBRESOURCE*, &handle, -1, MFCDBRESOURCE_RES_NAME, le_mfcdbresource);
    
    // mfc_load_resource(db);    
    
    /* verifico la lunghezza del record */
    if(!is_varlen) {
        
        if(psVBFile[db->MFCHandle]->iMinRowLength != psVBFile[db->MFCHandle]->iMaxRowLength ) {
            
            php_error(E_ERROR, "mfcdb_insert: unable to insert fixed length record in a variable length table");
            RETURN_FALSE;
        }
        
        if(value_len != psVBFile[db->MFCHandle]->iMinRowLength) {
            
            sprintf(buff, "mfcdb_insert: cant insert record of length %d in record of length %d", value_len, psVBFile[db->MFCHandle]->iMinRowLength);
            php_error(E_ERROR, buff);
            RETURN_FALSE;
        }
    
    } // if(!is_varlen)
    
    
    /* inserisco il record */
    result = iswrite(db->MFCHandle, value);
    
    if(result == -1) {
        
        if(iserrno == EDUPL) {  // CHIAVE GIA' ESISTENTE

            php_error(E_WARNING, "mfcdb_insert: error inserting data, EDUPL key\n");
            RETURN_FALSE;

        }

        sprintf(buff, "mfcdb_insert: error while inserting data, errno is %d", iserrno);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
        
    }
    
    // mfc_save_resource(db);
    
    RETURN_LONG(isrecnum);
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_exists) : verifica l'esistenza di un record nella tabella 
 * @return TRUE | FALSE
 */
/* {{{ proto bool mfcdb_exists(resource handle, string key [, int key_selected] )
   verify if a record exists in table */
PHP_FUNCTION(mfcdb_exists)
{
	char *key = NULL;
	int argc = ZEND_NUM_ARGS();
	int key_len;
    int key_selected, key_auto;
	int handle_id = -1;
	zval *handle = NULL;

	if (zend_parse_parameters(argc TSRMLS_CC, "rs|l", &handle, &key, &key_len, &key_selected) == FAILURE) 
		return;

    /*-------------------------------------------
    -- CHECK ARGS
    -------------------------------------------*/
    
    if(argc == 2) {
        
        key_selected = -1;
        key_auto = 1;
    
    } else {
    
        key_auto = 0;
    
    }
    
    
    if(argc < 2) {
        php_error(E_WARNING, "mfcdb_exists: wrong number of arguments (min 2 args)");
        RETURN_FALSE;
    }
    
    
    /*-------------------------------------------
    -- MY DECLARATIONS/CODE
    -------------------------------------------*/
    MFCDBRESOURCE *db;
    char buff[1024];
    char *buff_rec;
    int k, result;
    
    /* recupero la risorsa */
    ZEND_FETCH_RESOURCE(db, MFCDBRESOURCE*, &handle, -1, MFCDBRESOURCE_RES_NAME, le_mfcdbresource);
    
    // mfc_load_resource(db);
    
    /* verifico che la chiave scelta esista */
    if(!key_auto) {
        
        if(psVBFile[db->MFCHandle]->psKeydesc[key_selected]->k_len != key_len) {
            
            php_error(E_WARNING, "mfcdb_exists: key selected does not match key len in dictinfo");
            RETURN_FALSE;
            
        }
       
    } else {    /* se è key auto cerco la key della lunghezza necessaria */
        
        for(k=0;k<psVBFile[db->MFCHandle]->iNKeys;k++) {
            
            if(psVBFile[db->MFCHandle]->psKeydesc[k]->k_len == key_len) {
                
                key_selected = k;
            
            }
            
        }
    
    }
        
    if(key_selected == -1) {

        php_error(E_WARNING, "mfcdb_exists: key of length specified does not exists");
        RETURN_FALSE;

    }
    
    /* alloco la memoria per il buff_rec */
    buff_rec = (char*) emalloc((psVBFile[db->MFCHandle]->iMaxRowLength + 1) * sizeof(char));
    memset(buff_rec, 0, sizeof(buff_rec));
    
    /* cerco il record */
    snprintf(buff_rec, (key_len + 1), "%s", key);
#ifdef MFC_DDEBUG
fprintf(stdout, "mfcdb_exists: cerco chiave |%s| \n", buff_rec);
#endif


    if(key_selected > 0) {      /* se la chiave non è la primaria */
        
        result = isstart(db->MFCHandle, psVBFile[db->MFCHandle]->psKeydesc[key_selected], psVBFile[db->MFCHandle]->psKeydesc[key_selected]->k_len, buff_rec, ISEQUAL);
        
        if(result) {
            
            sprintf(buff, "mfcdb_fetch: error in isstart fetching secondary key, errno is %d\n", iserrno);
            php_error(E_WARNING, buff);
            RETURN_FALSE;
        }
        
        result = isread(db->MFCHandle, buff_rec, ISCURR);
        
    } else {                    /* ricerca su chiave primaria */
        
        result = isread(db->MFCHandle, buff_rec, ISEQUAL);
    }

#ifdef MFC_DDEBUG
fprintf(stdout, "mfcdb_exists: risultato isread iresult %d, isrecnum %d,  data %s \n", result, isrecnum, buff_rec);
#endif
    
    if(result) {
        
        if(iserrno == ENOREC) {
            
            RETURN_FALSE;
        
        }
        
        sprintf(buff, "mfcdb_exists: error reading record, errno is %d\n", iserrno);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
        
    }

    // mfc_save_resource(db);
    
    /* libero la memoria */
    efree(buff_rec);
    
    RETURN_TRUE;
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_fetch) : restituisce il valore corrispondente a una data chiave
 * @param resource risorsa del database aperto
 * @param string key chiave
 * @param int numkey (opzionale) per limitare la ricerca alla data chiave
 * @return string valore
 */
/* {{{ proto string mfcdb_fetch(resource handle, string key [, int key_selected])
   read a record identified by key from table and return it */
PHP_FUNCTION(mfcdb_fetch)
{
	char *key = NULL;
	int argc = ZEND_NUM_ARGS();
	int key_len;
    int key_selected, key_auto;
	int handle_id = -1;
	zval *handle = NULL;

	if (zend_parse_parameters(argc TSRMLS_CC, "rs|l", &handle,  &key, &key_len, &key_selected) == FAILURE) 
		return;
 
    /*-------------------------------------------
    -- CHECK ARGS
    -------------------------------------------*/
    
    if(argc == 2) {
        
        key_selected = -1;
        key_auto = 1;
    
    } else {
    
        key_auto = 0;
    
    }
    
    
    if(argc < 2) {
        php_error(E_WARNING, "mfcdb_fetch: wrong number of arguments (min 2 args)");
        RETURN_FALSE;
    }
    
    
    /*-------------------------------------------
    -- MY DECLARATIONS/CODE
    -------------------------------------------*/
    MFCDBRESOURCE *db;
    char buff[1024];
    char *buff_rec;
    int k, result;
    
    /* recupero la risorsa */
    ZEND_FETCH_RESOURCE(db, MFCDBRESOURCE*, &handle, -1, MFCDBRESOURCE_RES_NAME, le_mfcdbresource);
    
    //mfc_load_resource(db);
    
    /* verifico che la chiave scelta esista */
    if(!key_auto) {
        
        if(psVBFile[db->MFCHandle]->psKeydesc[key_selected]->k_len != key_len) {
            
            php_error(E_WARNING, "mfcdb_fetch: key selected does not match key len in dictinfo");
            RETURN_FALSE;
            
        }
       
     } else {    /* se è key auto cerco la key della lunghezza necessaria */
        
        for(k=0;k<psVBFile[db->MFCHandle]->iNKeys;k++) {
            
            if(psVBFile[db->MFCHandle]->psKeydesc[k]->k_len == key_len) {
                
                key_selected = k;
            
            }
            
        }
    
    }
        
    if(key_selected == -1) {

        php_error(E_WARNING, "mfcdb_fetch: key of length specified does not exists");
        RETURN_FALSE;

    }
    
    /* alloco la memoria per il buff_rec */
    buff_rec = (char*) emalloc((psVBFile[db->MFCHandle]->iMaxRowLength + 1) * sizeof(char));
    memset(buff_rec, 0, sizeof(buff_rec));
    
    /* cerco il record */
    snprintf(buff_rec, (key_len + 1), "%s", key);
#ifdef MFC_DDEBUG
fprintf(stdout, "mfcdb_fetch: cerco chiave %s,  index attivo: %d \n", buff_rec, psVBFile[db->MFCHandle]->iActiveKey);
#endif
    
    if(key_selected > 0) {      /* se la chiave non è la primaria */
        
        result = isstart(db->MFCHandle, psVBFile[db->MFCHandle]->psKeydesc[key_selected], psVBFile[db->MFCHandle]->psKeydesc[key_selected]->k_len, buff_rec, ISEQUAL);
        
        if(result) {
            
            sprintf(buff, "mfcdb_fetch: error in isstart fetching secondary key, errno is %d\n", iserrno);
            php_error(E_WARNING, buff);
            RETURN_FALSE;
        }
        
        result = isread(db->MFCHandle, buff_rec, ISCURR);
        
    } else {                    /* ricerca su chiave primaria */
        
        result = isread(db->MFCHandle, buff_rec, ISEQUAL);
    }

    if(result) {
        
        sprintf(buff, "mfcdb_fetch: error reading record, errno is %d\n", iserrno);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
        
    }
#ifdef MFC_DDEBUG
fprintf(stdout, "mfcdb_fetch: trovato record %d, data %s \n", isrecnum, buff_rec);
#endif

    // mfc_save_resource(db);

    RETURN_STRINGL(buff_rec, psVBFile[db->MFCHandle]->iMaxRowLength, 1);
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_start) : prepara un puntatore alla tabella per scansioni ripetute e consecutive
 * @param resource risorsa del database aperto
 * @param int key_selected chiave utilizzata per la ricerca
 * @param string key chiave o parte di essa
 * @param int type tipo di puntamento
 * @return TRUE | FALSE
 */
/* {{{ proto string mfcdb_start(resource handle, int key_selected, string key, int search_type)
   prepare a pointer to table in order to do sequential searches */
PHP_FUNCTION(mfcdb_start)
{
	char *key = NULL;
	int argc = ZEND_NUM_ARGS();
	int key_len;
    int key_selected, search_type;
    long search_by_isrecnum = (long) -1;
	int handle_id = -1;
	zval *handle = NULL;

	if (zend_parse_parameters(argc TSRMLS_CC, "rlsl|l", &handle,  &key_selected, &key, &key_len, &search_type, &search_by_isrecnum) == FAILURE) 
		return;
 
    /*-------------------------------------------
    -- CHECK ARGS
    -------------------------------------------*/
    
    if(argc < 4 ) {
       
       php_error(E_WARNING, "mfcdb_start: this function requires at least 4 arguments");
       RETURN_FALSE;
    
    }
    
    /*-------------------------------------------
    -- MY DECLARATIONS/CODE
    -------------------------------------------*/
    MFCDBRESOURCE *db;
    struct keydesc *key_desc;
    int result, old_k_nparts;
    char buff[1024], *cP;
    
    /* recupero la risorsa */
    ZEND_FETCH_RESOURCE(db, MFCDBRESOURCE*, &handle, -1, MFCDBRESOURCE_RES_NAME, le_mfcdbresource);
    
    if(key_selected < 0 || key_selected >= psVBFile[db->MFCHandle]->iNKeys ) {
       
       php_error(E_WARNING, "mfcdb_start: invalid key_selected");
       RETURN_FALSE;
       
    }
    
    if(key_len == 0) {
       
       php_error(E_WARNING, "mfcdb_start: invalid key length");
       RETURN_FALSE;
    
    }
    
    /* recupero la descrizione della chiave selezionata */
    key_desc = psVBFile[db->MFCHandle]->psKeydesc[key_selected];
    
    if(key_len > key_desc->k_len) {
       
       sprintf(buff, "mfcdb_start: key gived (%d bytes) is longest of max key length (%d bytes ) in key description", key_len, key_desc->k_len);
       php_error(E_WARNING, buff);
       RETURN_FALSE;
    
    }
    
    if(search_by_isrecnum < 1 && search_by_isrecnum != -1) {
       
       sprintf(buff, "mfcdb_start: isrecnum gived(%d) is not valid\n", search_by_isrecnum);
       php_error(E_WARNING, buff);
       RETURN_FALSE;
    
    }
    
    if(search_by_isrecnum >= 1) {
       
       old_k_nparts = key_desc->k_nparts;
       key_desc->k_nparts = 0;
       isrecnum = search_by_isrecnum;
       
    }
    
    /* effettuo il padding della key per essere compliant con isstart */
    cP = buff;
    memset(cP, 0, key_desc->k_part[0].kp_start);
    cP = &buff[key_desc->k_part[0].kp_start];
    memcpy(cP, key, key_len);
    buff[key_desc->k_part[0].kp_start + key_len] = '\0';
    
    result = isstart(db->MFCHandle, key_desc, key_len, buff, search_type);
       
    if(search_by_isrecnum >= 1) {
    
       key_desc->k_nparts = old_k_nparts;
       
    }
    
    
    if(result) {
          
      sprintf(buff, "mfcdb_start: error starting table. errno is %d\n", iserrno);
      php_error(E_WARNING, buff);
      RETURN_FALSE;
          
    }

#ifdef MFC_DDEBUG
fprintf(stdout, "mfcdb_start: index attivato: %d \n", psVBFile[db->MFCHandle]->iActiveKey);
#endif
       
    RETURN_TRUE;
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_curr) : return current record pointed by a mfcdb_start
 * @param resource resource of connection 
 * @return string record data
 */
/* {{{ proto bool mfcdb_curr(resource handle)
   return current record pointed by a mfcdb_start */
PHP_FUNCTION(mfcdb_curr)
{
	int argc = ZEND_NUM_ARGS();
	int handle_id = -1;
	zval *handle = NULL;

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &handle) == FAILURE) 
		return;

    /*-------------------------------------------
    -- MY DECLARATIONS/CODE
    -------------------------------------------*/
    MFCDBRESOURCE *db;
    int result;
    char *buff_rec;
    char buff[1024];
    
    /* recupero la risorsa */
    ZEND_FETCH_RESOURCE(db, MFCDBRESOURCE*, &handle, -1, MFCDBRESOURCE_RES_NAME, le_mfcdbresource);

    /* alloco la memoria per il buff_rec */
    buff_rec = (char*) emalloc((psVBFile[db->MFCHandle]->iMaxRowLength + 1) * sizeof(char));
    memset(buff_rec, 0, sizeof(buff_rec));

#ifdef MFC_DDEBUG
fprintf(stdout, "mfcdb_curr: index attivo: %d \n", psVBFile[db->MFCHandle]->iActiveKey);
#endif
    result = isread(db->MFCHandle, buff_rec, ISCURR);
    
    if(result) {
        
        sprintf(buff, "mfcdb_curr: error getting current record, errno is %d\n", iserrno);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
        
    }
    
    RETURN_STRINGL(buff_rec, psVBFile[db->MFCHandle]->iMaxRowLength, 1);
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_next) : return next record pointed by a mfcdb_start
 * @param resource resource of connection 
 * @return string record data
 */
/* {{{ proto bool mfcdb_next(resource handle)
   return next record pointed by a mfcdb_start */
PHP_FUNCTION(mfcdb_next)
{
	int argc = ZEND_NUM_ARGS();
	int handle_id = -1;
	zval *handle = NULL;

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &handle) == FAILURE) 
		return;

    /*-------------------------------------------
    -- MY DECLARATIONS/CODE
    -------------------------------------------*/
    MFCDBRESOURCE *db;
    int result;
    char *buff_rec;
    char buff[1024];
    
    /* recupero la risorsa */
    ZEND_FETCH_RESOURCE(db, MFCDBRESOURCE*, &handle, -1, MFCDBRESOURCE_RES_NAME, le_mfcdbresource);

    /* alloco la memoria per il buff_rec */
    buff_rec = (char*) emalloc((psVBFile[db->MFCHandle]->iMaxRowLength + 1) * sizeof(char));
    memset(buff_rec, 0, sizeof(buff_rec));
    
    result = isread(db->MFCHandle, buff_rec, ISNEXT);
    
    if(result) {
        
        sprintf(buff, "mfcdb_next: error getting next record, errno is %d\n", iserrno);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
        
    }
    
    RETURN_STRINGL(buff_rec, psVBFile[db->MFCHandle]->iMaxRowLength, 1);
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_prev) : return previous record pointed by a mfcdb_start
 * @param resource resource of connection 
 * @return string record data
 */
/* {{{ proto bool mfcdb_prev(resource handle)
   return previous record pointed by a mfcdb_start */
PHP_FUNCTION(mfcdb_prev)
{
	int argc = ZEND_NUM_ARGS();
	int handle_id = -1;
	zval *handle = NULL;

	if (zend_parse_parameters(argc TSRMLS_CC, "r", &handle) == FAILURE) 
		return;

    /*-------------------------------------------
    -- MY DECLARATIONS/CODE
    -------------------------------------------*/
    MFCDBRESOURCE *db;
    int result;
    char *buff_rec;
    char buff[1024];
    
    /* recupero la risorsa */
    ZEND_FETCH_RESOURCE(db, MFCDBRESOURCE*, &handle, -1, MFCDBRESOURCE_RES_NAME, le_mfcdbresource);

    /* alloco la memoria per il buff_rec */
    buff_rec = (char*) emalloc((psVBFile[db->MFCHandle]->iMaxRowLength + 1) * sizeof(char));
    memset(buff_rec, 0, sizeof(buff_rec));
    
    result = isread(db->MFCHandle, buff_rec, ISPREV);
    
    if(result) {
        
        sprintf(buff, "mfcdb_prev: error getting previous record, errno is %d\n", iserrno);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
        
    }
    
    RETURN_STRINGL(buff_rec, psVBFile[db->MFCHandle]->iMaxRowLength, 1);
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_replace) : replace record pointed by primarey key given as arguments with new record. It updates all keys.
 * @param resource resource of connection 
 * @param string primary key pointing record to replace
 * @param string data of new record
 * @return MFC_TRUE | MFC_FALSE 
 */
/* {{{ proto bool mfcdb_replace(string key, string value, resource handle)
   replace record pointed by primarey key given as arguments with new record. It updates all keys */
PHP_FUNCTION(mfcdb_replace)
{
	char *key = NULL;
	char *value = NULL;
	int argc = ZEND_NUM_ARGS();
	int key_len;
	int value_len;
	int handle_id = -1;
	zval *handle = NULL;

	if (zend_parse_parameters(argc TSRMLS_CC, "rss", &handle, &key, &key_len, &value, &value_len) == FAILURE) 
		return;

    /*-------------------------------------------
    -- CHECK ARGS
    -------------------------------------------*/

    if(argc < 3) {
        
        php_error(E_WARNING, "mfcdb_replace: this function requires atleast 3 arguments");
        RETURN_FALSE;
        
    }
    
    /*-------------------------------------------
    -- MY DECLARATIONS/CODE
    -------------------------------------------*/
    MFCDBRESOURCE *db;
    char buff[1024], *buff_rec; 
    int result;
    
    
    /* recupero la risorsa */
    ZEND_FETCH_RESOURCE(db, MFCDBRESOURCE*, &handle, -1, MFCDBRESOURCE_RES_NAME, le_mfcdbresource);
    
    /* verifico la lunghezza chiave */
    if(psVBFile[db->MFCHandle]->psKeydesc[0]->k_len != key_len) {
        
        sprintf(buff, "mfcdb_replace: error in key length. Primary key is %d bytes, your key is %d bytes",  psVBFile[db->MFCHandle]->psKeydesc[0]->k_len, key_len);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
    
    }
    
    /* alloco la memoria per il buff_rec */
    buff_rec = (char*) emalloc((psVBFile[db->MFCHandle]->iMaxRowLength + 1) * sizeof(char));
    memset(buff_rec, 0, sizeof(buff_rec));
    
    /* cerco il record da eliminare */
    snprintf(buff_rec, (key_len + 1), "%s", key);
#ifdef MFC_DDEBUG
fprintf(stdout, "mfcdb_replace: cerco chiave %s \n", buff_rec);
#endif
    result = isread(db->MFCHandle, buff_rec, ISEQUAL);
    
    if(result) {
        
        sprintf(buff, "mfcdb_replace: error reading record, errno is %d\n", iserrno);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
        
    }
    
    /* se record a lunghezza variabile, imposto isreclen */
    if(db->MFCVarLen) {
        
        isreclen = value_len;
        
    }
    
    /* aggiorno il record con il nuovo */
    result = isrewcurr(db->MFCHandle, value);
    
    if(result) {
        
        sprintf(buff, "mfcdb_replace: error updating/replacing record, errno is %d", iserrno);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
        
    }
    
    efree(buff_rec); /* risorsa per la ricerca record */
    RETURN_TRUE;
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_delete) : delete record pointed by primary key passed as argument
 * @param resource resource of connection
 * @param string primary key to search for deletion
 * @return MFC_TRUE | MFC_FALSE 
 */
/* {{{ proto bool mfcdb_delete(resource handle, string key)
   delete from table record pointed by key */
PHP_FUNCTION(mfcdb_delete)
{
	char *key = NULL;
	int argc = ZEND_NUM_ARGS();
	int key_len;
	int handle_id = -1;
	zval *handle = NULL;

	if (zend_parse_parameters(argc TSRMLS_CC, "rs", &handle, &key, &key_len) == FAILURE) 
		return;

    /*-------------------------------------------
    -- CHECK ARGS
    -------------------------------------------*/

    if(argc < 2) {
        
        php_error(E_WARNING, "mfcdb_delete: this function requires atleast 2 arguments");
        RETURN_FALSE;
        
    }
    
    /*-------------------------------------------
    -- MY DECLARATIONS/CODE
    -------------------------------------------*/
    MFCDBRESOURCE *db;
    char buff[1024], *buff_rec; 
    int result;
    
    
    /* recupero la risorsa */
    ZEND_FETCH_RESOURCE(db, MFCDBRESOURCE*, &handle, -1, MFCDBRESOURCE_RES_NAME, le_mfcdbresource);
    
    /* verifico la lunghezza chiave */
    if(psVBFile[db->MFCHandle]->psKeydesc[0]->k_len != key_len) {
        
        sprintf(buff, "mfcdb_delete: error in key length. Primary key is %d bytes, your key is %d bytes",  psVBFile[db->MFCHandle]->psKeydesc[0]->k_len, key_len);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
    
    }
    
    /* alloco la memoria per il buff_rec */
    buff_rec = (char*) emalloc((psVBFile[db->MFCHandle]->iMaxRowLength + 1) * sizeof(char));
    memset(buff_rec, 0, sizeof(buff_rec));
    
    /* cerco il record da eliminare */
    snprintf(buff_rec, (key_len + 1), "%s", key);
#ifdef MFC_DDEBUG
fprintf(stdout, "mfcdb_delete: cerco chiave %s \n", buff_rec);
#endif
    result = isread(db->MFCHandle, buff_rec, ISEQUAL);
    
    if(result) {
        
        sprintf(buff, "mfcdb_delete: error reading record, errno is %d\n", iserrno);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
        
    }
    
    /* se il record esiste lo elimino */
    result = isdelcurr(db->MFCHandle);
    
    if(result) {
        
        sprintf(buff, "mfcdb_delete: error deleting record, errno is %d\n", iserrno);
        php_error(E_WARNING, buff);
        RETURN_FALSE;
    
    }
    
    efree(buff_rec); /* risorsa per la ricerca record */
    RETURN_TRUE;
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_get_info) : restituisce un array informativo sulle tabelle aperte
 * @return array informazioni sulla tabella
 */
/* {{{ proto array mfcdb_get_info(resource handle) 
   return info about handle/table opened */
PHP_FUNCTION(mfcdb_get_info)
{
	int argc = ZEND_NUM_ARGS();
	zval *handle = NULL;
    
	if (zend_parse_parameters(argc TSRMLS_CC, "r", &handle) == FAILURE) 
		return;

    /*-------------------------------------------
    -- DECLARATIONS 
    -------------------------------------------*/
    MFCDBRESOURCE *db;
    zval *key_array[32], *key_part[64], *key_section; 
    int k, j, i, l;
    struct keydesc *key;
        
    ZEND_FETCH_RESOURCE(db, MFCDBRESOURCE*, &handle, -1, MFCDBRESOURCE_RES_NAME, le_mfcdbresource);
    
    // mfc_load_resource(db);
    
#ifdef MFC_DEBUG
printf("mfcdb_get_info: lettura informazioni su handle %d \n", db->MFCHandle);
#endif

    array_init(return_value); /* array dati da restituire */
    add_assoc_string(return_value, "tbl_filename", psVBFile[db->MFCHandle]->cFilename, 1);
    
    add_assoc_long(return_value, "tbl_num_keys", psVBFile[db->MFCHandle]->iNKeys);
    add_assoc_long(return_value, "tbl_active_key", psVBFile[db->MFCHandle]->iActiveKey);
    add_assoc_long(return_value, "tbl_node_size", psVBFile[db->MFCHandle]->iNodeSize);
    add_assoc_long(return_value, "tbl_min_row_length", psVBFile[db->MFCHandle]->iMinRowLength);
    add_assoc_long(return_value, "tbl_max_row_length", psVBFile[db->MFCHandle]->iMaxRowLength);
    add_assoc_long(return_value, "tbl_data_handle", psVBFile[db->MFCHandle]->iDataHandle);
    add_assoc_long(return_value, "tbl_index_handle", psVBFile[db->MFCHandle]->iIndexHandle);
    add_assoc_long(return_value, "tbl_is_open", psVBFile[db->MFCHandle]->iIsOpen);
    add_assoc_long(return_value, "tbl_open_mode", psVBFile[db->MFCHandle]->iOpenMode);
    add_assoc_long(return_value, "tbl_varlen_length", psVBFile[db->MFCHandle]->iVarlenLength);
    add_assoc_long(return_value, "tbl_varlen_slot", psVBFile[db->MFCHandle]->iVarlenSlot);
    add_assoc_long(return_value, "tbl_row_number", psVBFile[db->MFCHandle]->tRowNumber);
    add_assoc_long(return_value, "tbl_dup_number", psVBFile[db->MFCHandle]->tDupNumber);
    add_assoc_long(return_value, "tbl_row_start", psVBFile[db->MFCHandle]->tRowStart);
    add_assoc_long(return_value, "tbl_trans_last", psVBFile[db->MFCHandle]->tTransLast);
    add_assoc_long(return_value, "tbl_num_rows", psVBFile[db->MFCHandle]->tNRows);
    add_assoc_long(return_value, "tbl_varlen_node", psVBFile[db->MFCHandle]->tVarlenNode);
    
    MAKE_STD_ZVAL(key_section);
    array_init(key_section);
    
    i = l = 0; // indice key_array, key_parts 
    
    /* elaboro l'array delle chiavi */
    for(k=0;k<psVBFile[db->MFCHandle]->iNKeys;k++) {
        
        MAKE_STD_ZVAL(key_array[i]);
        array_init(key_array[i]);
        
#ifdef MFC_DDEBUG
printf("mfcdb_get_info: lettura key, k %d, l %d, i %d, j %d \n", k, l, i, j);
#endif
        key = psVBFile[db->MFCHandle]->psKeydesc[k];
        // if(key->k_len == 0)  break;
        
        add_assoc_long(key_array[i], "key_k_rootnode", key->k_rootnode);
        add_assoc_long(key_array[i], "key_k_len", key->k_len);
        add_assoc_long(key_array[i], "key_k_nparts", key->k_nparts);
        
        for(j=0;j<key->k_nparts;j++) {
            
#ifdef MFC_DDEBUG
printf("mfcdb_get_info: lettura key_part, j %d \n", j);
#endif
            MAKE_STD_ZVAL(key_part[l]);
            array_init(key_part[l]);
            
            add_assoc_long(key_part[l], "keypart_kp_start", key->k_part[j].kp_start);
            add_assoc_long(key_part[l], "keypart_kp_leng", key->k_part[j].kp_leng);
            add_assoc_long(key_part[l], "keypart_kp_type", key->k_part[j].kp_type);
            
            add_assoc_zval(key_array[i], "keypart_data", key_part[l++]);
            
            // efree(key_part);
            
        } // for(j=0;j<key->k_nparts;j++)
        
        add_next_index_zval(key_section, key_array[i++]);
        
        // efree(key_array);
    
    } // for(k=0;k<psVBFile[db->MFCHandle]->iNKeys;k++)
    
    add_assoc_zval(return_value, "tbl_key_section", key_section);
    
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_comp3encode) : return a string given in comp-3 encoding
 * @param string data to encode
 * @param int comp-3 type: MFC_COMP3_TYPE_C, MFC_COMP3_TYPE_D, MFC_COMP3_TYPE_F
 * @param int bytes length of return string 
 * @return string encoded string 
 */
/* {{{ proto string mfcdb_comp3encode(string text, int comp3_type [, int byte_len])
   encode the text given in comp3 mode */
PHP_FUNCTION(mfcdb_comp3encode)
{
    char *data = NULL;
    int argc = ZEND_NUM_ARGS();
    int data_len;
    long byte_len;
    long comp3type;
    
    int comp3type_int;
    int k=0, j=0;
    char *comp3 = NULL;
    
    if (zend_parse_parameters(argc TSRMLS_CC, "sl|l", &data, &data_len, &comp3type, &byte_len) == FAILURE) 
        return;

    /*-------------------------------------------
    -- CHECK ARGS
    -------------------------------------------*/
    
    if(argc < 3) {
        
        byte_len = 0;
    
    }
    
    if(argc < 2) {
        
        php_error(E_ERROR, "mfcdb_comp3encode: invalid number of arguments");
        RETURN_FALSE;
        
    }
    

    /*-------------------------------------------
    -- DECLARATIONS 
    -------------------------------------------*/
    
    comp3type_int = (int) comp3type;
    
    /* step 1 __ verifico che tutte le cifre siano numeriche */
    k=strlen(data)-1;
    j=0;
    while(k>=0) {
#ifdef MFC_DDEBUG
    printf("mfcdb_comp3encode: verifico numero su char %c\n", data[k]);
#endif 
        if(!isdigit(data[k--])) j = 1;
    }
    if(j) {
        php_error(E_WARNING, "mfcdb_comp3encode: argument must be a numeric string");
        return;
    }
    
    /* step 2 __ verifico che il tipo di comp3 richiesto sia corretto */
#ifdef MFC_DDEBUG
    printf("mfcdb_comp3encode: analizzo il typo di comp-3 : %c -> %x\n", comp3type_int, comp3type_int);
#endif
    switch(comp3type_int) {
        case MFC_COMP3_TYPE_C:
        case MFC_COMP3_TYPE_D:
        case MFC_COMP3_TYPE_F:
            break;
        default:
            php_error(E_WARNING, "mfcdb_comp3encode: comp3 type incorrect or not supported");
            return;
    }
    
    /* step 3 __ encoding data */
    comp3 = mfc_util_comp3encode(data, comp3type_int, (int) byte_len); 
    
    /* step 4 __ restituisco il dato encodato */
    if((data_len + 1) % 2) k = (data_len + 1 + 1) / 2;
    else k = (data_len + 1) / 2;
    
    if(byte_len) k = byte_len; // se specificata la lunghezza forzo il parametro
    
    RETURN_STRINGL(comp3, k, 1);
    EFREE(comp3);
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_comp3decode) : return a decoded version of a comp-3 string 
 * @param string comp-3 data to decode
 * @param int bytes length of binary data to decode
 * @return string decoded string
 */
/* {{{ proto string mfcdb_comp3decode(string data, int byte_len)
   return a decoded comp3 version of data */
PHP_FUNCTION(mfcdb_comp3decode)
{
    char *data = NULL;
    int argc = ZEND_NUM_ARGS();
    int data_len;
    long byte_len;
    
    char *comp3 = NULL;
    
    
    if (zend_parse_parameters(argc TSRMLS_CC, "sl", &data, &data_len, &byte_len) == FAILURE) 
        return;
    
    comp3 = mfc_util_comp3decode(data, byte_len);
    
    RETURN_STRING(comp3, 1);
    // EFREE(comp3);
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_error) : it returns last error message 
 * @return string error message 
 */
/* {{{ proto string mfcdb_error()
   return a string containing last error occurred */
PHP_FUNCTION(mfcdb_error)
{
    char buff[1024];
    
    switch(iserrno) {
        
        case EDUPL:
            sprintf(buff, "%-3d - ERROR: Duplicate row", iserrno);
            break;
        case ENOTOPEN:
            sprintf(buff, "%-3d - ERROR: File not open", iserrno);
            break;
        case EBADARG:
            sprintf(buff, "%-3d - ERROR: Illegal argument", iserrno);
            break;
        case EBADKEY:
            sprintf(buff, "%-3d - ERROR: Illegal key desc", iserrno);
            break;
        case ETOOMANY:
            sprintf(buff, "%-3d - ERROR: Too many files open", iserrno);
            break;
        case EBADFILE:
            sprintf(buff, "%-3d - ERROR: Bad isam file format", iserrno);
            break;
        case ENOTEXCL:
            sprintf(buff, "%-3d - ERROR: Non-exclusive access", iserrno);
            break;
        case ELOCKED:
            sprintf(buff, "%-3d - ERROR: Row locked", iserrno);
            break;
        case EKEXISTS:
            sprintf(buff, "%-3d - ERROR: Key already exists", iserrno);
            break;
        case EPRIMKEY:
            sprintf(buff, "%-3d - ERROR: Is primary key", iserrno);
            break;
        case EENDFILE:
            sprintf(buff, "%-3d - ERROR: End/begin of file", iserrno);
            break;
        case ENOREC:
            sprintf(buff, "%-3d - ERROR: No row found", iserrno);
            break;
        case ENOCURR:
            sprintf(buff, "%-3d - ERROR: No current row", iserrno);
            break;
        case EFLOCKED: 
            sprintf(buff, "%-3d - ERROR: File locked", iserrno);
            break;
        case EFNAME:
            sprintf(buff, "%-3d - ERROR: File name too long", iserrno);
            break;
        case EBADMEM:
            sprintf(buff, "%-3d - ERROR: Can't alloc memory", iserrno);
            break;
        case ELOGREAD:
            sprintf(buff, "%-3d - ERROR: Cannot read log rec", iserrno);
            break;
        case EBADLOG: 
            sprintf(buff, "%-3d - ERROR: Bad log row", iserrno);
            break;
        case ELOGOPEN:
            sprintf(buff, "%-3d - ERROR: Cannot open log file", iserrno);
            break;
        case ELOGWRIT:
            sprintf(buff, "%-3d - ERROR: Cannot write log rec", iserrno);
            break;
        case ENOTRANS:
            sprintf(buff, "%-3d - ERROR: No transaction", iserrno);
            break;
        case ENOBEGIN:
            sprintf(buff, "%-3d - ERROR: No begin work yet", iserrno);
            break;
        case ENOPRIM: 
            sprintf(buff, "%-3d - ERROR: No primary key", iserrno);
            break;
        case ENOLOG:
            sprintf(buff, "%-3d - ERROR: No logging", iserrno);
            break;
        case ENOFREE:
            sprintf(buff, "%-3d - ERROR: No free disk space", iserrno);
            break;
        case EROWSIZE:
            sprintf(buff, "%-3d - ERROR: Row size too short / long", iserrno);
            break;
        case EAUDIT:
            sprintf(buff, "%-3d - ERROR: Audit trail exists", iserrno);
            break;
        case ENOLOCKS:
            sprintf(buff, "%-3d - ERROR: No more locks", iserrno);
            break;
        case EDEADLOK:
            sprintf(buff, "%-3d - ERROR: Deadlock avoidance", iserrno);
            break;
        case ENOMANU:
            sprintf(buff, "%-3d - ERROR: Must be in ISMANULOCK mode", iserrno);
            break;
        case EINTERUPT:
            sprintf(buff, "%-3d - ERROR: Interrupted isam call", iserrno);
            break;
        case EBADFORMAT: 
            sprintf(buff, "%-3d - ERROR: Locking or NODESIZE change", iserrno);
            break;
        default:
            sprintf(buff, "%-3d - ERROR: unknow error, errocode is %d", iserrno, iserrno);
    }

    RETURN_STRINGL(buff, strlen(buff), 1);
}
/* }}} */




/**
 * PHP_FUNCTION(mfcdb_get_recnum) : returns record number of last call to db functions
 * @return int record number 
 */
/* {{{ proto int mfcdb_get_recnum()
   return last record number sets by commands in database */
PHP_FUNCTION(mfcdb_get_recnum)
{
    
    RETURN_LONG(isrecnum);
}
/* }}} */




/** ----------------------------------------------------------------------------
 * UTILITIES -------------------------------------------------------------------
 ** --------------------------------------------------------------------------*/

 
 

/** ---------------------------------------------
* mfc_util_hex2dec : convert hex text string to dec value 
* @param char* text binary data to convert
* @param int size size of data to convert
* @return int value converted
*/
int mfc_util_hex2dec(char *text, int size)
{
    int value = 0;
    int k=0, c = 0, pow = 0, dec = 0, pows[] = { 0x00, 0x100, 0x10000 };

    k=size-1;
    while(k>=0) {
        dec = mfc_util_hex2dec_byte(text[k]);
        value += (pows[c] * dec) + ((c==0)?dec:0) ;
        
#ifdef MFC_DDEBUG
    printf("mfc_util_hex2dec: converto %02X, pos k %d, c %d, pow %d, decimal %d, value att %d\n", text[k] & 0xFF, k, c, pows[c], dec, value);
#endif

        k--;
        c++;
    }

    return (int) value; 
}



/** ---------------------------------------------
* mfc_util_hex2dec_byte : convert a byte in dec value 
* @param char c byte to convert
* @return int value converted 
*/
int mfc_util_hex2dec_byte(char c)
{    
    int teens = 0, neg = MFC_FALSE;    
    
    if(c < 0) 
        neg = MFC_TRUE;
    
    while(c >= 10) {
        c -= 10;
        teens++;
    }
    
    return (10 * teens) + c + ( (neg)?256:0 );
}



/** ---------------------------------------------
* mfc_util_printhex : return a string hex rappresentation of binary data gived
* @param char* txt binary data to convert
* @param int len size of data to convert
* @return char* pointer at converted string
*/
char* mfc_util_printhex(char *txt, int len)
{
    int k = 0;
    char *buff = NULL;
    char tmp[3];
    
    buff = (char*) emalloc((len * 2) * sizeof(char)+1);
    memset(buff, 0, (len * 2) * sizeof(char)+1);
    
    
    while(k<len) {
#ifdef MFC_DDEBUG
        printf("mfc_util_printhex: ricevo %02x, accodo in buffer\n",  txt[k]);
#endif        
        sprintf(tmp, "%02X", txt[k] & 0xFF);
        strncat(buff, tmp, 2);
        k++;
    }
#ifdef MFC_DDEBUG
    printf("mfc_util_printhex: creata stringa %s\n", buff);
#endif

    return buff;
}



/** ---------------------------------------------
* mfc_util_comp3decode : return a string hex rappresentation of binary data gived
* @param char* txt binary data to convert
* @param int len size of data to convert
* @return char* pointer at converted string
*/
char* mfc_util_comp3decode(char *txt, int len)
{
    char *buff = NULL;
    char *buff2 = NULL;
    char sign, *p;
    int k = 0, j = 0, zero_jump = 0;

    buff = mfc_util_printhex(txt, len);
    
    buff2 = (char*) emalloc(strlen(buff) * sizeof(char) + 1);
    memset(buff2, 0, strlen(buff));
    
    /* analizzo il segno */
    sign = buff[strlen(buff)-1];
#ifdef MFC_DDEBUG
    printf("mfc_util_comp3decode: letto sign : %c\n", sign);
#endif 
    switch(sign)
    {
        case 'C':
            p = &buff2[1];
            buff2[0] = '0';         /* set to + to get sign also on positive numbers */
            memcpy(p, buff, (strlen(buff)-1));
            break;
        case 'D':
            p = &buff2[1];
            buff2[0] = '-';
            memcpy(p, buff, (strlen(buff)-1));
            break;
        default:
            memcpy(buff2, buff, (strlen(buff)-1));
    }

    buff2[strlen(buff)] = '\0';
    
#ifdef MFC_DDEBUG
    printf("mfc_util_comp3decode: restituisco stringa %s\n", buff2);
#endif 
    efree(buff);
    
    /* pulisco la stringa */
    buff = (char*) emalloc(strlen(buff2) * sizeof(char));
    memset(buff, 0, strlen(buff2));
    
    j = 0;
    zero_jump = 1;
    for(k=0;k<strlen(buff2);k++) {
        if(buff2[k] == '+' || buff2[k] == '-') {
            buff[j++] = buff2[k];
        }
        if(buff2[k] == '1' ||
           buff2[k] == '2' ||
           buff2[k] == '3' ||
           buff2[k] == '4' ||
           buff2[k] == '5' ||
           buff2[k] == '6' ||
           buff2[k] == '7' ||
           buff2[k] == '8' ||
           buff2[k] == '9') {
            zero_jump = 0;
            buff[j++] = buff2[k];
        }
        if(buff2[k] == '0' && !zero_jump) {
            buff[j++] = buff2[k];
        }
    }

    efree(buff2);
    
    return buff;
}



/** ---------------------------------------------
* mfc_util_comp3encode : return a string hex rappresentation of binary data gived
* @param char* txt binary data to convert
* @param int type of comp3 MFC_COMP3_TYPE_C | MFC_COMP3_TYPE_D | MFC_COMP3_TYPE_F
* @return char* pointer at converted string
*/
char* mfc_util_comp3encode(char *txt, int comp3_type, int byte_len)
{
    char *buff, *buff2, buff3[256];
    char a, z;
    int k, j, ia, iz, ibyte;
    
    buff = (char*) emalloc((strlen(txt) + 2) * sizeof(char));
    memset(buff, 0, strlen(txt) + 1);
    
    memcpy(buff, txt, strlen(txt) + 1);
    buff[strlen(txt)] = comp3_type;
    buff[strlen(txt)+1] = '\0';
    
#ifdef MFC_DDEBUG 
printf("mfc_util_comp3encode: txt |%s|, buff |%s|\n", txt, buff);
#endif
    
    
    k = (strlen(buff) % 2);
    if(k == 0) j = (strlen(buff) / 2);
    else j = ((strlen(buff) + 1) / 2 );
    
    if(byte_len > j) 
        j = (byte_len%2)?byte_len+1:byte_len;
#ifdef MFC_DDEBUG 
printf("mfc_util_comp3encode: byte_len %d j %d\n", byte_len, j);
#endif
    
    buff2 = (char*) emalloc(j * sizeof(char));
    memset(buff2, 0, j);
    
    k = 0;
    j--;
    
#ifdef MFC_DDEBUG 
printf("mfc_util_comp3encode: strlen buff %d \n", strlen(buff));
#endif
    
    for(k=(strlen(buff)-1);k>=0;k=k-2) {
        z = buff[k];
        
        if((k-1) >= 0) a = buff[k-1];
        else a = '0';

#ifdef MFC_DDEBUG 
printf("mfc_util_comp3encode: values k-> %d, j -> %d, a-> %c, z -> %c\n", k, j, a, z);
#endif
        
        switch(z) {
            case 'c':
            case 'C':
                iz = 0xC;
                break;
            case 'd':
            case 'D':
                iz = 0xD;
                break;
            case 'f':
            case 'F':
                iz = 0xF;
                break;
            default:
                sprintf(buff3, "%c", z);
                iz = atoi(buff3);
        }
        sprintf(buff3, "%c", a);
        ia = atoi(buff3);
        
        ibyte = ia * 16 | iz;

#ifdef MFC_DDEBUG 
printf("mfc_util_comp3encode: ia %d, iz %d, ibyte dec %d, ibyte hex %2x\n", ia, iz, ibyte, ibyte);
#endif 

        buff2[j--] = ibyte;
    }
    

#ifdef MFC_DDEBUG
    k = (strlen(buff) % 2);
    if(k == 0) j = (strlen(buff) / 2);
    else j = ((strlen(buff) + 1) / 2 );
    
    if(byte_len > j) 
        j = (byte_len%2)?byte_len+1:byte_len;
    
    printf("mfc_util_comp3encode: encoding stringa %s, restituisco stringa visualizzata hex %s\n", buff, mfc_util_printhex(buff2, j));
#endif 

    efree(buff);
    
    return(buff2);
}

/* EOF */
