/*
 * memuse - a tool to give a reasonable per application memory footprint estimate
 *
 * (C) Copyright 2008 Intel Corporation
 *
 * Authors: 
 *	Arjan van de Ven <arjan@linux.intel.com>
 *	Jing Wong <jing_j_wang@intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This file contains the reporting routines
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

#include "memuse.h"

static int comparef (gconstpointer a, gconstpointer b)
{
	struct program *A = (struct program *)a;
	struct program *B = (struct program *)b;

	return B->kb - A->kb;
}

struct program *find_program(char *name)
{
	GList *item;
	struct program *program;	

	item = g_list_first(programs_d);
	while (item) {
		program =  item->data;
		if (strncmp(program->name,name,strlen(name))==0)
			break;
		item = g_list_next(item);	
	}
	if (!item)
		program=NULL;
	return program;
}

void report_results(void)
{
	GList *item;
	uint64_t total = 0;
	int count=0;
	char buf[4096];

	struct program *program;

	if (daem == 0){
		programs = g_list_sort(programs, comparef);

		item = g_list_first(programs);
		while (item) {
			if ( num!=0 && count>=num)
				break;
			program = item->data;
			total += program->kb;
			struct program *program_d=NULL;
			if (dfile)
				program_d=find_program(program->name);

			if (program_d)
				sprintf(buf, _("%8lluKb(%+6lldK)\t%s \n"), program->kb,program->kb - program_d->kb,program->name);
			else
				sprintf(buf, _("%8lluKb\t\t%s \n"), program->kb, program->name);
			printf("%s", buf);
			if (ofile){
				sprintf(buf,_("%8lluKb\t\t%s \n"), program->kb, program->name);
				fputs(buf,ofile);
			}
			count++;
			item = g_list_next(item);
		}
		printf(_("%8lluKb\t\tsystem total\n"), total);
		report_library();
	}else {
		item = g_list_first(programs);
		while (item) {
			program = item->data;
			total += program->kb;
			item = g_list_next(item);
		}
	
		uint64_t prv_size = get_pvr_total() / 1024;

		if (sfile) {
			fseek(sfile, 0, SEEK_SET);
			sprintf(buf, _("%lluKb %lluKb\n"), total, prv_size); 
			fputs(buf, sfile);
			fflush(sfile);
		}
	}

}

