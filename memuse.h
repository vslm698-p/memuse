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
 */

#ifndef __INCLUDE_GUARD_MEMUSE_H__
#define __INCLUDE_GUARD_MEMUSE_H__

#include <stdint.h>
#include <glib.h>
#include <locale.h>
#include <libintl.h>

#define PVR_MEMINFO_FILE "/proc/pvr/meminfo"

struct program {
	char		*name;
	uint64_t	kb;
};

struct library {
	char            *name;
	uint64_t        total_kb;
	int             count;

	double          cost;
	int             pid;
};

#define _(STRING)    gettext(STRING)

extern GList *libraries;
extern GList *programs;
extern GList *programs_d;
extern FILE *dfile;
extern FILE *ofile;
extern FILE *sfile;
extern int num;
extern int daem;

/* prototypes */

extern void parse_proc(void);
extern void report_results(void);
extern void parse_savedfile(void);
extern void add_library(char *name, uint64_t mem, int pid);
extern void report_library(void);
extern uint64_t get_pvr_total(void);

#endif
