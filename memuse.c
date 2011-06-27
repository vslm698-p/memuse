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
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/types.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>
#include <string.h>

#include "memuse.h"

GList *programs;
GList *programs_d;
FILE *dfile=NULL;
FILE *ofile=NULL;
int num=0;

void cleanup(void)
{
	if (dfile)
		fclose(dfile);
	if (ofile)
		fclose(ofile);
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
			{ "diff", 1, NULL, 'd' },
			{ "num", 1, NULL, 'n' },
			{ "help", 0, NULL, 'h' },
			{ 0, 0, NULL, 0 }
		};
		int index2 = 0, c;

		c = getopt_long(argc, argv, "o:d:n:h", opts, &index2);
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
			case 'd':
				dfile = fopen(optarg,"r");
				if ( !dfile){
					fprintf(stderr,_("Fail to create or open %s!"),optarg);
					return EXIT_FAILURE;
				}	
				break;

			case 'n':
				num= atoi(optarg);
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
	parse_proc();
	report_results();
	cleanup();
	return EXIT_SUCCESS;
}
