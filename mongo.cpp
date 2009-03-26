// mongo.cpp
/**
 *  Copyright 2009 10gen, Inc.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *  http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <vector>

#include <php.h>
#include <php_ini.h>
#include <sys/socket.h>
#include <ext/standard/info.h>
#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>

#include "mongo.h"
#include "mongo_types.h"
#include "bson.h"
#include "gridfs.h"

/** Classes */
zend_class_entry *mongo_id_class, 
  *mongo_code_class, 
  *mongo_date_class, 
  *mongo_regex_class, 
  *mongo_bindata_class;

/** Resources */
int le_connection, le_pconnection, le_db_cursor, le_gridfs, le_gridfile;

ZEND_DECLARE_MODULE_GLOBALS(mongo)
static PHP_GINIT_FUNCTION(mongo);

static function_entry mongo_functions[] = {
  PHP_FE( mongo_connect , NULL )
  PHP_FE( mongo_close , NULL )
  PHP_FE( mongo_remove , NULL )
  PHP_FE( mongo_query , NULL )
  PHP_FE( mongo_find_one , NULL )
  PHP_FE( mongo_insert , NULL )
  PHP_FE( mongo_batch_insert , NULL )
  PHP_FE( mongo_update , NULL )
  PHP_FE( mongo_has_next , NULL )
  PHP_FE( mongo_next , NULL )
  PHP_FE( mongo_gridfs_init , NULL )
  PHP_FE( mongo_gridfs_list , NULL )
  PHP_FE( mongo_gridfs_store , NULL )
  PHP_FE( mongo_gridfs_find , NULL )
  PHP_FE( mongo_gridfile_filename , NULL )
  PHP_FE( mongo_gridfile_size , NULL )
  PHP_FE( mongo_gridfile_write , NULL )
  {NULL, NULL, NULL}
};

static function_entry mongo_id_functions[] = {
  PHP_NAMED_FE( __construct, PHP_FN( mongo_id___construct ), NULL )
  PHP_NAMED_FE( __toString, PHP_FN( mongo_id___toString ), NULL )
  { NULL, NULL, NULL }
};


static function_entry mongo_code_functions[] = {
  PHP_NAMED_FE( __construct, PHP_FN( mongo_code___construct ), NULL )
  PHP_NAMED_FE( __toString, PHP_FN( mongo_code___toString ), NULL )
  { NULL, NULL, NULL }
};


static function_entry mongo_date_functions[] = {
  PHP_NAMED_FE( __construct, PHP_FN( mongo_date___construct ), NULL )
  PHP_NAMED_FE( __toString, PHP_FN( mongo_date___toString ), NULL )
  { NULL, NULL, NULL }
};


static function_entry mongo_regex_functions[] = {
  PHP_NAMED_FE( __construct, PHP_FN( mongo_regex___construct ), NULL )
  PHP_NAMED_FE( __toString, PHP_FN( mongo_regex___toString ), NULL )
  { NULL, NULL, NULL }
};

static function_entry mongo_bindata_functions[] = {
  PHP_NAMED_FE( __construct, PHP_FN( mongo_bindata___construct ), NULL )
  PHP_NAMED_FE( __toString, PHP_FN( mongo_bindata___toString ), NULL )
  { NULL, NULL, NULL }
};


/* {{{ mongo_module_entry 
 */
zend_module_entry mongo_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
  STANDARD_MODULE_HEADER,
#endif
  PHP_MONGO_EXTNAME,
  mongo_functions,
  PHP_MINIT(mongo),
  PHP_MSHUTDOWN(mongo),
  PHP_RINIT(mongo),
  NULL,
  PHP_MINFO(mongo),
  PHP_MONGO_VERSION,
  PHP_MODULE_GLOBALS(mongo),
  PHP_GINIT(mongo),
  NULL,
  NULL,
  STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_MONGO
ZEND_GET_MODULE(mongo)
#endif


/* {{{ PHP_INI */ 
PHP_INI_BEGIN()
STD_PHP_INI_BOOLEAN("mongo.auto_reconnect", "0", PHP_INI_SYSTEM, OnUpdateLong, auto_reconnect, zend_mongo_globals, mongo_globals) 
STD_PHP_INI_BOOLEAN("mongo.allow_persistent", "1", PHP_INI_SYSTEM, OnUpdateLong, allow_persistent, zend_mongo_globals, mongo_globals) 
STD_PHP_INI_ENTRY_EX("mongo.max_persistent", "-1", PHP_INI_SYSTEM, OnUpdateLong, max_persistent, zend_mongo_globals, mongo_globals, display_link_numbers) 
STD_PHP_INI_ENTRY_EX("mongo.max_connections", "-1", PHP_INI_SYSTEM, OnUpdateLong, max_links, zend_mongo_globals, mongo_globals, display_link_numbers) 
STD_PHP_INI_ENTRY("mongo.default_host", NULL, PHP_INI_ALL, OnUpdateString, default_host, zend_mongo_globals, mongo_globals) 
STD_PHP_INI_ENTRY("mongo.default_port", "27017", PHP_INI_ALL, OnUpdateLong, default_port, zend_mongo_globals, mongo_globals) 
PHP_INI_END()
/* }}} */


/* {{{ PHP_GINIT_FUNCTION
 */
static PHP_GINIT_FUNCTION(mongo){
  mongo_globals->num_persistent = 0; 
  mongo_globals->num_links = 0; 
  mongo_globals->auto_reconnect = false; 
  mongo_globals->default_host = "localhost"; 
  mongo_globals->default_port = 27017; 
  mongo_globals->request_id = 3;
}
/* }}} */

static void php_connection_dtor( zend_rsrc_list_entry *rsrc TSRMLS_DC ) {
  mongo_link *conn = (mongo_link*)rsrc->ptr;
  if( conn ) {
    close(conn->socket);
    efree(conn);
    if (rsrc->type == le_pconnection) {
      MonGlo(num_persistent)--;
    }
    MonGlo(num_links)--;
  }
}

static void php_gridfs_dtor( zend_rsrc_list_entry *rsrc TSRMLS_DC ) {
  mongo::GridFS *fs = (mongo::GridFS*)rsrc->ptr;
  if( fs )
    delete fs;
}


static void php_gridfile_dtor( zend_rsrc_list_entry *rsrc TSRMLS_DC ) {
  mongo::GridFile *file = (mongo::GridFile*)rsrc->ptr;
  if( file )
    delete file;
}

static void php_cursor_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) {
  mongo_cursor *cursor = (mongo_cursor*)rsrc->ptr;
  if (cursor) {
    if (cursor->buf) {
      efree(cursor->buf);
    }
    efree(cursor);
  }
}


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(mongo) {

  REGISTER_INI_ENTRIES();

  le_connection = zend_register_list_destructors_ex(php_connection_dtor, NULL, PHP_CONNECTION_RES_NAME, module_number);
  le_pconnection = zend_register_list_destructors_ex(NULL, php_connection_dtor, PHP_CONNECTION_RES_NAME, module_number);
  le_db_cursor = zend_register_list_destructors_ex(php_cursor_dtor, NULL, PHP_DB_CURSOR_RES_NAME, module_number);
  le_gridfs = zend_register_list_destructors_ex(php_gridfs_dtor, NULL, PHP_GRIDFS_RES_NAME, module_number);
  le_gridfile = zend_register_list_destructors_ex(php_gridfile_dtor, NULL, PHP_GRIDFILE_RES_NAME, module_number);

  zend_class_entry bindata; 
  INIT_CLASS_ENTRY(bindata, "MongoBinData", mongo_bindata_functions); 
  mongo_bindata_class = zend_register_internal_class(&bindata TSRMLS_CC); 

  zend_class_entry code; 
  INIT_CLASS_ENTRY(code, "MongoCode", mongo_code_functions); 
  mongo_code_class = zend_register_internal_class(&code TSRMLS_CC); 

  zend_class_entry date; 
  INIT_CLASS_ENTRY(date, "MongoDate", mongo_date_functions); 
  mongo_date_class = zend_register_internal_class(&date TSRMLS_CC); 

  zend_class_entry id; 
  INIT_CLASS_ENTRY(id, "MongoId", mongo_id_functions); 
  mongo_id_class = zend_register_internal_class(&id TSRMLS_CC); 

  zend_class_entry regex; 
  INIT_CLASS_ENTRY(regex, "MongoRegex", mongo_regex_functions); 
  mongo_regex_class = zend_register_internal_class(&regex TSRMLS_CC); 


  return SUCCESS;
}
/* }}} */


/* {{{ PHP_MSHUTDOWN_FUNCTION
 */ 
PHP_MSHUTDOWN_FUNCTION(mongo) {
  UNREGISTER_INI_ENTRIES();

  return SUCCESS;
}
/* }}} */


/* {{{ PHP_RINIT_FUNCTION
 */ 
PHP_RINIT_FUNCTION(mongo) {
  MonGlo(num_links) = MonGlo(num_persistent); 
}
/* }}} */


/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(mongo) {
  char buf[32]; 

  php_info_print_table_start();

  php_info_print_table_header(2, "MongoDB Support", "enabled");
  snprintf(buf, sizeof(buf), "%ld", MonGlo(num_persistent));
  php_info_print_table_row(2, "Active Persistent Connections", buf);
  snprintf(buf, sizeof(buf), "%ld", MonGlo(num_links));
  php_info_print_table_row(2, "Active Connections", buf);

  php_info_print_table_end(); 

  DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ mongo_connect
 */
PHP_FUNCTION(mongo_connect) {
  php_mongo_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */


/* {{{ mongo_close() 
 */
PHP_FUNCTION(mongo_close) {
  zval *zconn;
 
  if (ZEND_NUM_ARGS() != 1 ) {
     zend_error( E_WARNING, "expected 1 parameter, got %d parameters", ZEND_NUM_ARGS() );
     RETURN_FALSE;
  }
  else if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zconn) == FAILURE) {
     zend_error( E_WARNING, "incorrect parameter types, expected mongo_close( connection )" );
     RETURN_FALSE;
  }

  zend_list_delete(Z_LVAL_P(zconn));
  RETURN_TRUE;
}
/* }}} */


/* {{{ mongo_query() 
 */
PHP_FUNCTION(mongo_query) {
  mongo_link *link;
  zval *zconn, *zquery, *zsort, *zfields, *zhint;
  char *collection;
  int limit, skip, collection_len, flags = 0, pos = 0;
  char *buf = (char*)emalloc(1024);
  
  if( ZEND_NUM_ARGS() != 8 ) {
      zend_error( E_WARNING, "expected 8 parameters, got %d parameters", ZEND_NUM_ARGS() );
      RETURN_FALSE;
  }
  else if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsallaaa", &zconn, &collection, &collection_len, &zquery, &skip, &limit, &zsort, &zfields, &zhint) == FAILURE ) {
      zend_error( E_WARNING, "incorrect parameter types, expected mongo_query( connection, string, array, int, int, array, array, array )" );
      RETURN_FALSE;
  }
  
  ZEND_FETCH_RESOURCE2(link, mongo_link*, &zconn, -1, PHP_CONNECTION_RES_NAME, le_connection, le_pconnection); 

  // creates an insert-style header
  CREATE_HEADER(OP_QUERY);
  APPEND_HEADER_NS(buf, pos, collection, collection_len);

  serialize_int(buf, &pos, skip);
  serialize_int(buf, &pos, limit);
  zval_to_bson(buf, &pos, Z_ARRVAL_P(zquery) TSRMLS_CC);

  serialize_size(buf, 0, pos);

  // sends
  int sent = send(link->socket, buf, pos, flags)+1;
  efree(buf);
  if (sent == -1) {
    RETURN_FALSE;
  }

  mongo_cursor *cursor = (mongo_cursor*)emalloc(sizeof(mongo_cursor));
  GET_RESPONSE_NS(link, cursor, collection, collection_len);
  cursor->limit = limit;

  ZEND_REGISTER_RESOURCE(return_value, cursor, le_db_cursor);
}
/* }}} */


static int get_reply(mongo_link *link, mongo_cursor *cursor) {
  int flags = 0;
  if (recv(link->socket, &cursor->header.length, INT_32, flags) == -1) {
    perror("recv");
    return FAILURE;
  }

  cursor->header.length -= INT_32;
  char *response = (char*)emalloc(cursor->header.length);
  if (recv(link->socket, response, cursor->header.length, flags) == -1) {
    perror("recv");
    return FAILURE;
  }
  char *r = response;

  memcpy(&cursor->header.request_id, r, INT_32);
  r += INT_32;
  memcpy(&cursor->header.response_to, r, INT_32);
  r += INT_32;
  memcpy(&cursor->header.op, r, INT_32);
  r += INT_32;

  memcpy(&cursor->flag, r, INT_32);
  r += INT_32;
  memcpy(&cursor->cursor_id, r, INT_64);
  r += INT_64;
  memcpy(&cursor->start, r, INT_32);
  r += INT_32;
  memcpy(&cursor->num, r, INT_32);
  r += INT_32;

  cursor->buf_size = cursor->header.length-(REPLY_HEADER_SIZE-INT_32);
  if(cursor->buf) {
    efree(cursor->buf);
  }
  cursor->buf = (char*)emalloc(cursor->buf_size);
  memcpy(cursor->buf, r, cursor->buf_size);
  /*
  php_printf("size: %d\n", cursor->buf_size);
  char *temp = cursor->buf;
  int i;
  for(i=0;i<cursor->buf_size; i++) {
    php_printf("%d\n", *temp++);
  }
  */
  efree(response);

  cursor->pos = 0;
  cursor->at = 0;

  return SUCCESS;
}

/* {{{ proto array mongo_find_one(resource connection, string ns, array query) 
   Query the database for one record */
PHP_FUNCTION(mongo_find_one) {
  zval *zconn, *zquery;
  char *collection;
  int collection_len;
  mongo::BSONObjBuilder bquery;
  mongo::DBClientBase *conn_ptr;

  /*  int i;
  char *temp = r;
  for(i=0;i<cursor->header.length-REPLY_HEADER_SIZE; i++) {
    php_printf("%d\n", *temp++);
    }*/

  if( ZEND_NUM_ARGS() != 3 ) {
    zend_error( E_WARNING, "expected 3 parameters, got %d parameters", ZEND_NUM_ARGS() );
    RETURN_FALSE;
  }
  else if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsa", &zconn, &collection, &collection_len, &zquery) == FAILURE) {
    zend_error( E_WARNING, "incorrect parameter types, expected mongo_find_one( connection, string, array )" );
    RETURN_FALSE;
  }

  ZEND_FETCH_RESOURCE2(conn_ptr, mongo::DBClientBase*, &zconn, -1, PHP_CONNECTION_RES_NAME, le_connection, le_pconnection); 

  php_array_to_bson(&bquery, Z_ARRVAL_P(zquery) TSRMLS_CC);

  mongo::BSONObj obj = conn_ptr->findOne((const char*)collection, bquery.done());

  array_init(return_value);
  bson_to_php_array(&obj, return_value TSRMLS_CC);
}
/* }}} */


/* {{{ proto bool mongo_remove(resource connection, string ns, array query) 
   Remove records from the database */
PHP_FUNCTION(mongo_remove) {
  zval *zconn, *zarray;
  char *collection;
  int collection_len;
  zend_bool justOne = 0;
  mongo::DBClientBase *conn_ptr;
  mongo::BSONObjBuilder rarray; 

  if( ZEND_NUM_ARGS() != 4 ) {
    zend_error( E_WARNING, "expected 4 parameters, got %d parameters", ZEND_NUM_ARGS() );
    RETURN_FALSE;
  }
  else if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsab", &zconn, &collection, &collection_len, &zarray, &justOne) == FAILURE) {
    zend_error( E_WARNING, "incorrect parameter types, expected mongo_remove( connection, string, array, bool )" );
    RETURN_FALSE;
  }

  ZEND_FETCH_RESOURCE2(conn_ptr, mongo::DBClientBase*, &zconn, -1, PHP_CONNECTION_RES_NAME, le_connection, le_pconnection); 

  php_array_to_bson(&rarray, Z_ARRVAL_P(zarray) TSRMLS_CC);
  conn_ptr->remove(collection, rarray.done(), justOne);

  RETURN_TRUE;
}
/* }}} */


/* {{{ proto bool mongo_insert(resource connection, string ns, array obj) 
   Insert a record to the database */
PHP_FUNCTION(mongo_insert) {
  zval *zconn, *zarray;
  char *collection;
  int collection_len;
  mongo_link *link;
  char *buf = (char*)emalloc(1024);
  int pos = 0, flags = 0;

  if (ZEND_NUM_ARGS() != 3 ) {
    zend_error( E_WARNING, "expected 3 parameters, got %d parameters", ZEND_NUM_ARGS() );
    RETURN_FALSE;
  }
  else if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsa", &zconn, &collection, &collection_len, &zarray) == FAILURE) {
    zend_error( E_WARNING, "incorrect parameter types, expected mongo_insert( connection, string, array )" );
    RETURN_FALSE;
  }

  ZEND_FETCH_RESOURCE2(link, mongo_link*, &zconn, -1, PHP_CONNECTION_RES_NAME, le_connection, le_pconnection); 

  // creates an insert-style header
  CREATE_HEADER(OP_INSERT);

  // appends it to buf
  APPEND_HEADER_NS(buf, pos, collection, collection_len);

  // adds data
  zval_to_bson(buf, &pos, Z_ARRVAL_P(zarray) TSRMLS_CC);
  serialize_size(buf, 0, pos);

  // sends
  RETVAL_BOOL(send(link->socket, buf, pos, flags)+1);
  efree(buf);
}
/* }}} */

PHP_FUNCTION(mongo_batch_insert) {
  mongo_link *link;
  HashPosition pointer;
  zval *zconn, *zarray, **data;
  char *collection;
  int collection_len, pos = 0, flags = 0;
  char *buf = (char*)emalloc(1024);

  if (ZEND_NUM_ARGS() != 3 ) {
    zend_error( E_WARNING, "expected 3 parameters, got %d parameters", ZEND_NUM_ARGS() );
    RETURN_FALSE;
  }
  else if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsa", &zconn, &collection, &collection_len, &zarray) == FAILURE) {
    zend_error( E_WARNING, "incorrect parameter types, expected mongo_batch_insert( connection, string, array )" );
    RETURN_FALSE;
  }

  ZEND_FETCH_RESOURCE2(link, mongo_link*, &zconn, -1, PHP_CONNECTION_RES_NAME, le_connection, le_pconnection); 

  // creates an insert-style header
  CREATE_HEADER(OP_INSERT);
  APPEND_HEADER_NS(buf, pos, collection, collection_len);

  HashTable *php_array = Z_ARRVAL_P(zarray);
  for(zend_hash_internal_pointer_reset_ex(php_array, &pointer); 
      zend_hash_get_current_data_ex(php_array, (void**) &data, &pointer) == SUCCESS; 
      zend_hash_move_forward_ex(php_array, &pointer)) {

    int start = pos;
    zval_to_bson(buf, &pos, Z_ARRVAL_PP(data) TSRMLS_CC);
    serialize_size(buf, start, pos);
  }

  RETVAL_BOOL(send(link->socket, buf, pos, flags)+1);
  efree(buf);
}


/* {{{ proto bool mongo_update(resource connection, string ns, array query, array replacement, bool upsert) 
   Update a record in the database */
PHP_FUNCTION(mongo_update) {
  zval *zconn, *zquery, *zobj;
  char *collection;
  int collection_len;
  zend_bool zupsert = 0;
  mongo::DBClientBase *conn_ptr;
  mongo::BSONObjBuilder bquery, bfields;
  int argc = ZEND_NUM_ARGS();

  if ( argc != 5 ) {
    zend_error( E_WARNING, "expected 5 parameters, got %d parameters", argc );
    RETURN_FALSE;
  }
  else if(zend_parse_parameters(argc TSRMLS_CC, "rsaab", &zconn, &collection, &collection_len, &zquery, &zobj, &zupsert) == FAILURE) {
    zend_error( E_WARNING, "incorrect parameter types, expected mongo_update( connection, string, array, array, bool )");
    RETURN_FALSE;
  }

  ZEND_FETCH_RESOURCE2(conn_ptr, mongo::DBClientBase*, &zconn, -1, PHP_CONNECTION_RES_NAME, le_connection, le_pconnection); 

  php_array_to_bson(&bquery, Z_ARRVAL_P(zquery) TSRMLS_CC);
  php_array_to_bson(&bfields, Z_ARRVAL_P(zobj) TSRMLS_CC);
  conn_ptr->update(collection, bquery.done(), bfields.done(), (int)zupsert);

  RETURN_TRUE;
}
/* }}} */


/* {{{ proto bool mongo_has_next(resource cursor) 
   Check if a cursor has another record. */
PHP_FUNCTION( mongo_has_next ) {
  zval *zcursor;
  mongo_cursor *cursor;
  int argc = ZEND_NUM_ARGS(), flags = 0;

  if (argc != 1 ) {
    zend_error( E_WARNING, "expected 1 parameters, got %d parameters", argc );
    RETURN_FALSE;
  }
  else if( zend_parse_parameters(argc TSRMLS_CC, "r", &zcursor) == FAILURE) {
    zend_error( E_WARNING, "incorrect parameter types, expected mongo_has_next( cursor ), got %d parameters", argc );
    RETURN_FALSE;
  }

  cursor = (mongo_cursor*)zend_fetch_resource(&zcursor TSRMLS_CC, -1, PHP_DB_CURSOR_RES_NAME, NULL, 1, le_db_cursor);

  if (cursor->at <= cursor->num) {
    RETURN_TRUE;
  }

  char *buf = (char*)emalloc(128);
  int pos = 0;
  CREATE_RESPONSE_HEADER(cursor->header.request_id, OP_REPLY);
  APPEND_HEADER_NS(buf, pos, cursor->ns, cursor->ns_len);
  serialize_int(buf, &pos, cursor->limit);
  serialize_long(buf, &pos, cursor->cursor_id);
  serialize_size(buf, 0, pos);
  if (send(cursor->link.socket, buf, pos, flags) == -1) {
    php_printf("unable to fetch more");
    efree(buf);
    RETURN_FALSE;
  }
  efree(buf);

  GET_RESPONSE(&cursor->link, cursor);

  if (cursor->num > 0) {
    RETURN_TRUE;
  }

  RETURN_FALSE;
}
/* }}} */


/* {{{ mongo_next
 */
PHP_FUNCTION( mongo_next ) {
  zval *zcursor;
  int argc;
  mongo_cursor *cursor;

  argc = ZEND_NUM_ARGS();
  if (argc != 1 ) {
    zend_error( E_WARNING, "expected 1 parameter, got %d parameters", argc );
    RETURN_FALSE;
  }
  else if(zend_parse_parameters(argc TSRMLS_CC, "r", &zcursor) == FAILURE) {
    zend_error( E_WARNING, "incorrect parameter type, expected mongo_next( cursor )" );
    RETURN_FALSE;
  }

  cursor = (mongo_cursor*)zend_fetch_resource(&zcursor TSRMLS_CC, -1, PHP_DB_CURSOR_RES_NAME, NULL, 1, le_db_cursor);

  if (cursor->at >= cursor->num) {
    efree(cursor->buf);
    GET_RESPONSE(&cursor->link, cursor);
  }

  if (cursor->at < cursor->num) {
    char *temp = cursor->buf+cursor->pos;
    zval *elem;
    ALLOC_INIT_ZVAL(elem);
    array_init(elem);
    temp = bson_to_zval(temp, elem TSRMLS_CC);

    // increment cursor position
    cursor->pos = temp - cursor->buf;
    cursor->at++;
    RETURN_ZVAL(elem, 0, 1);
  }
  RETURN_NULL();
}
/* }}} */


/* {{{ php_mongo_do_connect
 */
static void php_mongo_do_connect(INTERNAL_FUNCTION_PARAMETERS) {
  mongo_link *conn;
  char *server, *uname, *pass, *key;
  zend_bool persistent, paired, lazy;
  int server_len, uname_len, pass_len, key_len;
  zend_rsrc_list_entry *le;
  string error;
  struct sockaddr_in name;
  struct hostent *hostinfo;
  
  int argc = ZEND_NUM_ARGS();
  if (argc != 6) {
    zend_error( E_WARNING, "expected 6 parameters, got %d parameters", argc );
    RETURN_FALSE;
  }
  else if (zend_parse_parameters(argc TSRMLS_CC, "sssbbb", &server, &server_len, &uname, &uname_len, &pass, &pass_len, &persistent, &paired, &lazy) == FAILURE) {
    zend_error( E_WARNING, "incorrect parameter types, expected: mongo_pconnect( string, string, string, bool, bool, bool )" );
    RETURN_FALSE;
  }

  /* make sure that there aren't too many links already */
  if (MonGlo(max_links) > -1 &&
      MonGlo(max_links) <= MonGlo(num_links)) {
    RETURN_FALSE;
  }
  /* if persistent links aren't allowed, just create a normal link */
  if (!MonGlo(allow_persistent)) {
    persistent = 0;
  }
  /* make sure that there aren't too many persistent links already */
  if (persistent &&
      MonGlo(max_persistent) > -1 &&
      MonGlo(max_persistent) <= MonGlo(num_persistent)) {
    RETURN_FALSE;
  }

  /*  if (persistent) {
    key_len = spprintf(&key, 0, "%s_%s_%s", server, uname, pass);
    // if a connection is found, return it 
    if (zend_hash_find(&EG(persistent_list), key, key_len+1, (void**)&le) == SUCCESS) {
      conn = (mongo::DBClientBase*)le->ptr;
      ZEND_REGISTER_RESOURCE(return_value, conn, le_pconnection);
      efree(key);
      return;
    }
    // if lazy and no connection was found, return 
    else if(lazy) {
      efree(key);
      RETURN_NULL();
    }
    efree(key);
  }*/

  if (server_len == 0) {
    zend_error( E_WARNING, "invalid host" );
    RETURN_FALSE;
  }

  name.sin_family = AF_INET;
  name.sin_port = htons (27017);
  hostinfo = gethostbyname(server);
  if (hostinfo == NULL) {
    zend_error (E_WARNING, "unknown host %s", server);
    RETURN_FALSE;
  }
  name.sin_addr = *(struct in_addr*)hostinfo->h_addr;

  int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0) {
    zend_error (E_WARNING, "socket");
    RETURN_FALSE;
  }

  int connected = connect(sock, (struct sockaddr*) &name, sizeof(name));
  if (connected != 0) {
    zend_error(E_WARNING, "error connecting");
    RETURN_FALSE;
  }
  conn = (mongo_link*)emalloc(sizeof(mongo_link));
  conn->socket = sock;

  /*  if (paired) {
    conn = new mongo::DBClientPaired();
    if (!((mongo::DBClientPaired*)conn)->connect(server)) {
      zend_error(E_WARNING, "error connecting to pair");
      RETURN_FALSE;
    }
  } else {
    conn = new mongo::DBClientConnection(MonGlo(auto_reconnect));
    if (!((mongo::DBClientConnection*)conn)->connect(server, error)){
      zend_error(E_WARNING, "%s", error.c_str());
      RETURN_FALSE;
    }
    }*/
  
  // store a reference in the persistence list
  if (persistent) {
    zend_rsrc_list_entry new_le; 

    key_len = spprintf(&key, 0, "%s_%s_%s", server, uname, pass);
    Z_TYPE(new_le) = le_pconnection;
    new_le.ptr = conn;

    if (zend_hash_update(&EG(persistent_list), key, key_len+1, (void*)&new_le, sizeof(zend_rsrc_list_entry), NULL)==FAILURE) { 
      delete conn;
      efree(key);
      RETURN_FALSE;
    }
    efree(key);

    ZEND_REGISTER_RESOURCE(return_value, conn, le_pconnection);
    MonGlo(num_persistent)++;
  }
  // otherwise, just return the connection
  else {
    ZEND_REGISTER_RESOURCE(return_value, conn, le_connection);    
  }
  MonGlo(num_links)++;
}
/* }}} */
