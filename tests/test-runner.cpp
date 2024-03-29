//******************************************************************************
// Copyright (c) 2018, The Regents of the University of California (Regents).
// All Rights Reserved. See LICENSE for license details.
//------------------------------------------------------------------------------
#include <getopt.h>
#include <iostream>
#include <cstdio>
#include "keystone.h"
#include "edge_wrapper.h"
#include "report.h"
#include "test_dev_key.h"

char* longstr = "";

const unsigned int num_tests = 6;

char* strarr[num_tests] = {"0", "100", "10000", "100000", "1000000", "100000000"};

unsigned long print_buffer(char* str){
  printf("Enclave said: %s",str);
  return strlen(str);
}

void print_value(unsigned long val){
  printf("Enclave said value: %u\n",val);
  return;
}

const char* get_host_string(){
  return longstr;
}

static struct report_t report;

void print_hex(void* buffer, size_t len)
{
  int i;
  for(i = 0; i < len; i+=sizeof(uintptr_t))
  {
    printf("%.16lx ", *((uintptr_t*) ((uintptr_t)buffer + i)));
  }
  printf("\n");
}

void copy_report(void* buffer)
{
  Report report;

  report.fromBytes((unsigned char*)buffer);

  if (report.checkSignaturesOnly(_sanctum_dev_public_key))
  {
    printf("Attestation report SIGNATURE is valid\n");
  }
  else
  {
    printf("Attestation report is invalid\n");
  }
}

unsigned long run_and_time_enclave(char *eapp_file, char *rt_file, int self_timing, size_t freemem_size, size_t untrusted_size, uintptr_t utm_ptr) {
	
  Keystone enclave;
  Params params;
  unsigned long cycles1,cycles2;

  params.setFreeMemSize(freemem_size);
  params.setUntrustedMem(utm_ptr, untrusted_size);

  enclave.init(eapp_file, rt_file , params);
  edge_init(&enclave);

  if( self_timing ){
    asm volatile ("rdcycle %0" : "=r" (cycles1));
  }

  enclave.run();

  if( self_timing ){
    asm volatile ("rdcycle %0" : "=r" (cycles2));
  }


  if( self_timing ){
    return cycles2-cycles1;
  }

}

int main(int argc, char** argv)
{
  if(argc < 3 || argc > 8)
  {
    printf("Usage: %s <eapp> <runtime> [--utm-size SIZE(K)] [--freemem-size SIZE(K)] [--time] [--load-only] [--utm-ptr 0xPTR]\n", argv[0]);
    return 0;
  }


  int self_timing = 0;
  int load_only = 0;

  size_t untrusted_size = 2*1024*1024;
  size_t freemem_size = 48*1024*1024;
  uintptr_t utm_ptr = (uintptr_t)DEFAULT_UNTRUSTED_PTR;

  static struct option long_options[] =
    {
      {"time",         no_argument,       &self_timing, 1},
      {"load-only",    no_argument,       &load_only, 1},
      {"utm-size",     required_argument, 0, 'u'},
      {"utm-ptr",      required_argument, 0, 'p'},
      {"freemem-size", required_argument, 0, 'f'},
      {0, 0, 0, 0}
    };


  char* eapp_file = argv[1];
  char* rt_file = argv[2];

  int c;
  int opt_index = 3;
  while (1){

    c = getopt_long (argc, argv, "u:p:f:",
                     long_options, &opt_index);

    if (c == -1)
      break;

    switch (c){
    case 0:
      break;
    case 'u':
      untrusted_size = atoi(optarg)*1024;
      break;
    case 'p':
      utm_ptr = strtoll(optarg, NULL, 16);
      break;
    case 'f':
      freemem_size = atoi(optarg)*1024;
      break;
    }
  }

//  Keystone enclave;
//  Params params;
//  unsigned long cycles1,cycles2,cycles3,cycles4;
//
//  params.setFreeMemSize(freemem_size);
//  params.setUntrustedMem(utm_ptr, untrusted_size);
//
//
//  if( self_timing ){
//    asm volatile ("rdcycle %0" : "=r" (cycles1));
//  }
//
//  enclave.init(eapp_file, rt_file , params);
//
//  if( self_timing ){
//    asm volatile ("rdcycle %0" : "=r" (cycles2));
//  }
//
//  edge_init(&enclave);
//
//  if( self_timing ){
//    asm volatile ("rdcycle %0" : "=r" (cycles3));
//  }
//
//  if( !load_only )
//    enclave.run();
//
//  if( self_timing ){
//    asm volatile ("rdcycle %0" : "=r" (cycles4));
//    printf("[keystone-test] Init: %lu cycles\r\n", cycles2-cycles1);
//    printf("[keystone-test] Runtime: %lu cycles\r\n", cycles4-cycles3);
//  }
  
  run_and_time_enclave(eapp_file, rt_file, self_timing, freemem_size, untrusted_size, utm_ptr); 
 
  long curr_rts[num_tests];
  long diffs[num_tests-1] = {0, 0, 0, 0};
  unsigned int iters = 5;
  if(self_timing) {
	for (int i = 0; i < iters; i++) {
	  for (int j = 0; j < num_tests; j++) {
	    longstr = strarr[j];
	 	curr_rts[j] = run_and_time_enclave(eapp_file, rt_file, self_timing, freemem_size, untrusted_size, utm_ptr);
		printf("Curr runtime: %lu\n", curr_rts[j]);
	  }
	  for (int k = 1; k < num_tests; k++) {
	    diffs[k-1] += curr_rts[k] - curr_rts[0];
		printf("Curr diff %d: %li\n", k, curr_rts[k]-curr_rts[0]);
	  }
	}
	for (int l = 0; l < num_tests-1; l++) {
	  printf("Avg Difference in Runtime between %s and 0 iterations: %li\n", strarr[l+1], diffs[l]/iters);
	}
  }
  return 0;
}
