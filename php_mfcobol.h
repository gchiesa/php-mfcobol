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

#ifndef PHP_MFCOBOL_H
#define PHP_MFCOBOL_H

extern zend_module_entry mfcobol_module_entry;
#define phpext_mfcobol_ptr &mfcobol_module_entry

#ifdef PHP_WIN32
#define PHP_MFCOBOL_API __declspec(dllexport)
#else
#define PHP_MFCOBOL_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(mfcobol);
PHP_MSHUTDOWN_FUNCTION(mfcobol);
PHP_RINIT_FUNCTION(mfcobol);
PHP_RSHUTDOWN_FUNCTION(mfcobol);
PHP_MINFO_FUNCTION(mfcobol);

PHP_FUNCTION(confirm_mfcobol_compiled);	/* For testing, remove later. */
PHP_FUNCTION(mfcdb_key);
PHP_FUNCTION(mfcdb_create);
PHP_FUNCTION(mfcdb_open);
PHP_FUNCTION(mfcdb_close);
PHP_FUNCTION(mfcdb_insert);
PHP_FUNCTION(mfcdb_exists);
PHP_FUNCTION(mfcdb_fetch);
PHP_FUNCTION(mfcdb_start);
PHP_FUNCTION(mfcdb_curr);
PHP_FUNCTION(mfcdb_next);
PHP_FUNCTION(mfcdb_prev);
PHP_FUNCTION(mfcdb_replace);
PHP_FUNCTION(mfcdb_delete);
PHP_FUNCTION(mfcdb_get_info);
PHP_FUNCTION(mfcdb_get_recnum);
PHP_FUNCTION(mfcdb_comp3encode);
PHP_FUNCTION(mfcdb_comp3decode);
PHP_FUNCTION(mfcdb_error);


/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(mfcobol)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(mfcobol)
*/


/* In every utility function you add that needs to use variables 
   in php_mfcobol_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as MFCOBOL_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define MFCOBOL_G(v) TSRMG(mfcobol_globals_id, zend_mfcobol_globals *, v)
#else
#define MFCOBOL_G(v) (mfcobol_globals.v)
#endif

#endif	/* PHP_MFCOBOL_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
