/*
 * memuse - a tool to give a reasonable per application memory footprint estimate
 *
 * (C) Copyright 2008 Intel Corporation
 *
 * Authors: 
 *	Arjan van de Ven <arjan@linux.intel.com>
 *	Jing Wang <jing_j_wang@intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 *
 * This file contains the per-library routines
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

GList *libraries;

void add_library(char *name, uint64_t pss, int pid)
{
	GList *item;
	struct library *library;

	if (strlen(name)==0)
		return;

	item = g_list_first(libraries);
	while (item) {
		library = item->data;
		item = g_list_next(item);
		if (strcmp(name, library->name)==0) {
			if (library->pid != pid)
				library->count++;
			library->total_kb += pss;
			library->pid = pid;
			return;
		}
	}
	library = malloc(sizeof(struct library));
	assert(library != NULL);
	memset(library, 0, sizeof(struct library));
	library->name = strdup(name);
	library->count = 1;
	library->total_kb = pss;
	library->pid = pid;

	libraries = g_list_append(libraries, library);
}


static int comparef (gconstpointer a, gconstpointer b)
{
	struct library *A = (struct library *)a;
	struct library *B = (struct library *)b;

	return B->total_kb - A->total_kb;
}

void report_library(void)
{
	int i = 0;
	GList *item;
	struct library *library;

	item = g_list_first(libraries);
	while (item) {
		char *c;
		library = item->data;
		item = g_list_next(item);
		c = strchr(library->name, '\n');
		if (c) *c = 0;
		
		library->cost = 1.0 * library->total_kb / library->count;
	}

	libraries = g_list_sort(libraries, comparef);

	item = g_list_first(libraries);
			printf("%40s\t%s\t%s\t%s\n",
			"Library", "Users", "Avg Memory", "Total Memory");
	while (item) {
		library = item->data;
		item = g_list_next(item);
		if (i<10 && library->count > 0) {
			i++;
			printf("%40s\t%i\t%lluKb\t\t%lluKb\n",
			library->name,
			library->count,
			library->total_kb / library->count,
			library->total_kb);
		}		
	}

}

