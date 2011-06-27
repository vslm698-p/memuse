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
 * This file contains the /proc parsing routines
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
#include <fcntl.h>

#include "memuse.h"

static void parse_process(int pid)
{
	FILE *file;
	FILE *cmd;
	int len;

	char filename[PATH_MAX];
	char cmdfile[PATH_MAX];
	char program_name[PATH_MAX];
	char line[PATH_MAX];
	struct program *program;
	uint64_t pss = 0;
	uint64_t this_pss = 0;
	char lib[4096];
	int ret;
	
	memset(program_name, 0, sizeof(program_name));
/*
	sprintf(filename, "/proc/%i/exe", pid);
	ret = readlink(filename, program_name, sizeof(program_name)-1);
	if (ret < 0)
		return;
*/
	sprintf(cmdfile,"/proc/%i/cmdline",pid);
	cmd = fopen(cmdfile, "r");	
	if (!cmd)
		return;

	len=fread(program_name,1,sizeof(program_name),cmd);	
	if (len <= 0){
		fclose(cmd);
		return;
	}

	int i=0;
	while (i++<len){
		if (program_name[i]==0)
			program_name[i]=32;	
	}
	program_name[len]='\0';
		
	program = malloc(sizeof(struct program));
	assert(program != NULL);
	program->name = strdup(program_name);
	programs = g_list_append(programs, program);	
	
	sprintf(filename, "/proc/%i/smaps", pid);
	file = fopen(filename, "r");
	if (!file){
		fclose(cmd);
		return;
	}
	while (!feof(file)) {
		char *c;
		memset(line, 0, sizeof(line));
		if (!fgets(line, sizeof(line)-1, file))
			break;

		if (line[0] < 'A'|| line[0] > 'Z') {
			c = strchr(line, '/');
			if (c)
				strcpy(lib,c);
		}

		if (strstr(line, "Pss:")) {
			c = line;
			c+=5;
			pss += strtoull(c, NULL, 10);		
			this_pss += strtoull(c, NULL, 10);
		}

		if (strstr(line, "Swap:")) {
			add_library(lib, this_pss, pid);
			this_pss = 0;
			lib[0] = 0;
                }
		
	}
	program->kb = pss;
	fclose(file);		
	fclose(cmd);
}

void parse_proc(void)
{
	DIR *dir;
	struct dirent *ent;

	dir = opendir("/proc");
	if (!dir) {
		fprintf(stderr, _("Failed to open /proc!\n"));
		return;
	}
	do {
		uint64_t tmp;
		ent = readdir(dir);
		if (!ent)
			break;
		tmp = strtoull(ent->d_name, NULL, 10);
		if (tmp > 0)
			parse_process(tmp);
	} while (1);
	
	closedir(dir);
}

void parse_savedfile(void)
{
	char program_name[PATH_MAX];
	char line[PATH_MAX];
	struct program *program;
	uint64_t pss = 0;
	
	memset(program_name, 0, sizeof(program_name));
	
	if (!dfile)
		return;
	while (!feof(dfile)) {
		char *c;
		memset(line, 0, sizeof(line));
		if (!fgets(line,sizeof(line)-1,dfile))
			break;
		c=strstr(line, "Kb");
		if (!c)
			continue;
		*c='\0';
		pss=strtoull(line,NULL,0);
		c+=2;
		while (*c==' ' || *c=='\t')
			c++;
		strcpy(program_name,c);
		program_name[strlen(c)-1]='\0';
		program = malloc(sizeof(struct program));
		assert(program != NULL);
		program->name = strdup(program_name);
		program->kb=pss;
		programs_d = g_list_append(programs_d, program);	
	}
}
