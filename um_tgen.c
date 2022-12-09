/* um_tgen.c - performance measurement tool.
 * See https://github.com/UltraMessaging/um_tgen */
/*
  Copyright (c) 2021-2022 Informatica Corporation
  Permission is granted to licensees to use or alter this software for any
  purpose, including commercial applications, according to the terms laid
  out in the Software License Agreement.

  This source code example is provided by Informatica for educational
  and evaluation purposes only.

  THE SOFTWARE IS PROVIDED "AS IS" AND INFORMATICA DISCLAIMS ALL WARRANTIES 
  EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY IMPLIED WARRANTIES OF 
  NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
  PURPOSE.  INFORMATICA DOES NOT WARRANT THAT USE OF THE SOFTWARE WILL BE 
  UNINTERRUPTED OR ERROR-FREE.  INFORMATICA SHALL NOT, UNDER ANY CIRCUMSTANCES,
  BE LIABLE TO LICENSEE FOR LOST PROFITS, CONSEQUENTIAL, INCIDENTAL, SPECIAL OR 
  INDIRECT DAMAGES ARISING OUT OF OR RELATED TO THIS AGREEMENT OR THE 
  TRANSACTIONS CONTEMPLATED HEREUNDER, EVEN IF INFORMATICA HAS BEEN APPRISED OF 
  THE LIKELIHOOD OF SUCH DAMAGES.
*/

#include "cprt.h"
#include <stdio.h>
#include <string.h>
#if ! defined(_WIN32)
  #include <stdlib.h>
  #include <unistd.h>
#endif

#include "lbm/lbm.h"
#include "cprt.h"  /* See https://github.com/fordsfords/cprt */
#include "tgen.h"  /* See https://github.com/fordsfords/tgen */


#define E(lbm_funct_call_) do { \
  int e_ = (lbm_funct_call_); \
  if (e_ == LBM_FAILURE) { \
    fprintf(stderr, "ERROR (%s:%d): %s failed: '%s'\n", \
       CPRT_BASENAME(__FILE__), __LINE__, #lbm_funct_call_, lbm_errmsg()); \
    fflush(stderr); \
    CPRT_ERR_EXIT; \
  } \
} while (0)  /* E */


struct my_data_s {
  lbm_context_t *ctx;
  lbm_src_topic_attr_t *src_attr;
  lbm_topic_t *topic_obj;
  lbm_src_t *src;
#ifdef UM_SSRC
  lbm_ssrc_t *ssrc;
#endif
  char *buffer;
};
typedef struct my_data_s my_data_t;


/* Command-line options and their defaults. String defaults are set
 * in "get_my_opts()".
 */
static int o_affinity_cpu = -1;
static char *o_config = NULL;
#ifdef UM_SSRC
static int o_generic_src = 0;
#else
static int o_generic_src = 1;  /* If no smart source, force generic flag. */
#endif
static char *o_persist = NULL;
static char *o_script_str = NULL;
static char *o_topic_str = NULL;
static char *o_xml_config = NULL;


char usage_str[] = "Usage: um_tgen [-h] [-a affinity_cpu] [-c config] [-g] [-p persist_mode] -s script_string [-x xml_config]\n";

void usage(char *msg)
{
  if (msg) fprintf(stderr, "%s\n", msg);
  printf("%s\n", usage_str);
  CPRT_NET_CLEANUP;
  exit(1);
}  /* usage */


void help()
{
  fprintf(stderr, "%s\n", usage_str);
  fprintf(stderr, "where:\n"
      "  -h : print help\n"
      "  -a affinity_cpu : bitmap for CPU affinity for send thread [%d]\n"
      "  -c config : configuration file; can be repeated [%s]\n"
      "  -g : generic source [%d]\n"
      "  -p ''|r|s : persist mode (empty=streaming, r=RPP, s=SPP) [%s]\n"
      "  -s 'script' : test script (required)\n"
      "  -x xml_config : XML configuration file [%s]\n"
      , o_affinity_cpu, o_config, o_generic_src, o_persist, o_xml_config
  );
  CPRT_NET_CLEANUP;
  exit(0);
}


void get_my_options(int argc, char **argv, tgen_t *tgen)
{
  char *app_name;
  int opt;

  /* Set defaults for string options. */
  o_config = CPRT_STRDUP("");
  o_persist = CPRT_STRDUP("");
  o_script_str = CPRT_STRDUP("");
  o_topic_str = CPRT_STRDUP("");
  o_xml_config = CPRT_STRDUP("");

  while ((opt = cprt_getopt(argc, argv, "ha:c:gp:s:t:x:")) != EOF) {
    switch (opt) {
      case 'h': help(); break;
      case 'a': CPRT_ATOI(cprt_optarg, o_affinity_cpu); break;
      /* Allow -c to be repeated, loading each config file in succession. */
      case 'c': free(o_config);
                o_config = CPRT_STRDUP(cprt_optarg);
                E(lbm_config(o_config));
                break;
      case 'g': o_generic_src = 1; break;
      case 'p': free(o_persist); o_persist = CPRT_STRDUP(cprt_optarg); break;
      /* Allow -s to be repeated, loading each config file in succession. */
      case 's': free(o_script_str);
                o_script_str = CPRT_STRDUP(cprt_optarg);
                tgen_add_multi_steps(tgen, o_script_str);
                break;
      case 't': free(o_topic_str); o_topic_str = CPRT_STRDUP(cprt_optarg); break;
      case 'x': free(o_xml_config); o_xml_config = CPRT_STRDUP(cprt_optarg); break;
      default: usage(NULL);
    }  /* switch */
  }  /* while */

  if (strlen(o_persist) == 0) {
    app_name = "um_tgen";
  }
  else if (strcmp(o_persist, "r") == 0) {
    app_name = "um_tgen_rpp";
  }
  else if (strcmp(o_persist, "s") == 0) {
    app_name = "um_tgen_spp";
  }
  else {
    usage("Error, -p value must be '', 'r', or 's'\n");
  }

  if (o_script_str == NULL) {
    usage("Error, must supply script (-s)");
  }

#ifndef UM_SSRC
  if (! o_generic_src) {
    fprintf(stderr, "Smart source not available, must use '-g' for generic source.\n");
    exit(1);
  }
#endif

  if (strlen(o_xml_config) > 0) {
    /* Unlike lbm_config(), you can't load more than one XML file.
     * If user supplied -x more than once, only load last one. */
    E(lbm_config_xml_file(o_xml_config, app_name));
  }

  if (cprt_optind != argc) { usage("Unexpected positional parameter(s)"); }
}  /* get_my_options */


void create_source(my_data_t *my_data)
{
  lbm_context_t *ctx = my_data->ctx;
  lbm_topic_t *topic_obj;

  E(lbm_src_topic_alloc(&topic_obj, ctx, o_topic_str, NULL));

  if (o_generic_src) {
    E(lbm_src_create(&my_data->src, ctx, topic_obj, NULL, NULL, NULL));
    CPRT_ENULL(my_data->buffer = (char *)malloc(65536));
  }
#ifdef UM_SSRC
  else {
    E(lbm_ssrc_create(&my_data->ssrc, ctx, topic_obj, NULL, NULL, NULL));
    E(lbm_ssrc_buff_get(my_data->ssrc, &my_data->buffer, 0));
  }
#endif
}  /* create_source */


void delete_source(my_data_t *my_data)
{
  if (o_generic_src) {  /* If using smart src API */
    E(lbm_src_delete(my_data->src));
    free(my_data->buffer);
  }
#ifdef UM_SSRC
  else {
    E(lbm_ssrc_buff_put(my_data->ssrc, my_data->buffer));
    E(lbm_ssrc_delete(my_data->ssrc));
  }
#endif
}  /* delete_source */


void my_send(tgen_t *tgen, int len)
{
  my_data_t *my_data = (my_data_t *)tgen_user_data_get(tgen);

  if (o_generic_src) {  /* If using smart src API */
    E(lbm_src_send(my_data->src, my_data->buffer, len, 0));
  }
#ifdef UM_SSRC
  else {
    E(lbm_ssrc_send_ex(my_data->ssrc, my_data->buffer, len, 0, NULL));
  }
#endif
}  /* my_send */


void my_variable_change(tgen_t *tgen, char var_id, int value)
{
  switch (var_id) {
    /* Variable "l" controls LBT-RM loss rate. */
    case 'l': lbm_set_lbtrm_src_loss_rate(tgen_variable_get(tgen, var_id)); break;
  }
}


int main(int argc, char **argv)
{
  uint64_t cpuset;
  my_data_t my_data;
  tgen_t *tgen;
  CPRT_NET_START;

  tgen = tgen_create(0, &my_data);

  get_my_options(argc, argv, tgen);

  E(lbm_context_create(&my_data.ctx, NULL, NULL, NULL));

  /* Pin time-critical thread (sending thread) to requested CPU core. */
  if (o_affinity_cpu > -1) {
    CPRT_CPU_ZERO(&cpuset);
    CPRT_CPU_SET(o_affinity_cpu, &cpuset);
    cprt_set_affinity(cpuset);
  }

  create_source(&my_data);

  tgen_run(tgen);

  delete_source(&my_data);

  tgen_delete(tgen);

  return 0;
}  /* main */
