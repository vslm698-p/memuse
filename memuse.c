/*
 * memuse - a tool to give a reasonable per application memory footprint estimate
 *
 * (C) Copyright 2008 Intel Corporation
 *
 * Authors:
 *	Arjan van de Ven <arjan@linux.intel.com>
 *	Jing Wang <jing.j.wang@intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This file contains the main routines and program flow
 */


#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>
#include <string.h>

#include "memuse.h"

GList *programs = NULL;
GList *programs_d = NULL;
FILE *dfile = NULL;
FILE *ofile = NULL;
FILE *sfile = NULL;
int daem = 0;
int tt = 0;
int num = 0;

void free_node(gpointer data, gpointer user_data)
{
	struct program *p = (struct program *)data;
	if (p)
		free(p);
}

void free_list(void)
{
	g_list_foreach(programs, free_node, NULL);
	g_list_foreach(programs_d, free_node, NULL);
	g_list_free(programs);
	g_list_free(programs_d);
	programs = NULL;
	programs_d = NULL;
}

void cleanup(void)
{
	free_list();
	if (dfile)
		fclose(dfile);
	if (ofile)
		fclose(ofile);
	if (sfile)
		fclose(sfile);
}


void usage()
{
	printf(_("Usage: memuse [OPTION...]\n"));
	printf(_("  -o, --output=File            output the memory use list into specified file\n"));
	printf(_("  -d, --diff=File              diff the prevous memory use case with current\n"));
	printf(_("  -n, --num=Integer            Show top how many processes by memory use\n"));
	printf(_("  -h, --help            Show this help message\n"));
	exit(0);
}

int main(int argc, char **argv)
{
	setlocale (LC_ALL, "");
	bindtextdomain ("memuse", "/usr/share/locale");
	textdomain ("memuse");

	if (argc < 0 && argv[0] == NULL)
		return EXIT_FAILURE; /* shut up compiler warning */

	while (1) {
		static struct option opts[] = {
			{ "output", 1, NULL, 'o' },
			{ "diff", 1, NULL, 'i' },
			{ "num", 1, NULL, 'n' },
			{ "time", 1, NULL, 't'},
			{ "file", 1, NULL, 'f'},
			{ "daemon", 0, NULL, 'd'},
			{ "help", 0, NULL, 'h' },
			{ 0, 0, NULL, 0 }
		};
		int index2 = 0, c;

		c = getopt_long(argc, argv, "o:i:n:t:f:dh", opts, &index2);
		if (c == -1)
			break;
		switch (c) {
			case 'o':
				ofile = fopen(optarg,"w+");
				if ( !ofile){
					fprintf(stderr,_("Fail to create or open %s!"),optarg);
					return EXIT_FAILURE;
				}
				break;

			case 'i':
				dfile = fopen(optarg,"r");
				if ( !dfile){
					fprintf(stderr,_("Fail to create or open %s!"),optarg);
					return EXIT_FAILURE;
				}
				break;

			case 'n':
				num = atoi(optarg);
				break;

			case 't':
				tt = atoi(optarg);
				break;

			case 'f':
				sfile = fopen(optarg, "w+");
				break;

			case 'd':
				printf("Use daemon mode\n");
				daem = 1;
				break;

			case 'h':
				usage();
				break;

			default:
				;
		}
	}

	if (dfile)
		parse_savedfile();

	if (daem) {
		pid_t pid, sid;
		pid = fork();
		if (pid > 0)
			exit(0);
		else {
			if ((sid=setsid())<0)
				exit(1);
			umask(0);
			while (1) {
				parse_proc();
				report_results();
				free_list();
				sleep(tt);
			}
		}
	}else{
		parse_proc();
		report_results();
	}
	cleanup();
	return 0;
}
