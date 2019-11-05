//******************************************************************************
// Copyright (c) 2018, The Regents of the University of California (Regents).
// All Rights Reserved. See LICENSE for license details.
//------------------------------------------------------------------------------
#include "eapp_utils.h"
#include "string.h"
#include "syscall.h"
#include "malloc.h"
#include "edge_wrapper.h"

int atoi(char *str) {
  int result = 0;
  for (int i = 0; i < strlen(str); i++) {
	result = result * 10 + str[i] - '0';	
  } 
  return result;
}


void EAPP_ENTRY eapp_entry(){

  edge_init();

  struct edge_data pkgstr;
  ocall_get_string(&pkgstr);

  void* host_str = malloc(pkgstr.size);
  copy_from_shared(host_str, pkgstr.offset, pkgstr.size);


//  int val = 0;
//  if (((char *) host_str)[0] == 'a') 
//	 val = 1000000000;
  int val = atoi(host_str);
  ocall_print_value(val);
  for (int i = 0; i < val; i++);


  EAPP_RETURN(3);
}
