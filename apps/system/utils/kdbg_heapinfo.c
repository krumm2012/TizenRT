/****************************************************************************
 *
 * Copyright 2016-2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <tinyara/sched.h>
#include <tinyara/config.h>
#include <tinyara/mm/mm.h>

#ifdef CONFIG_DEBUG_MM_HEAPINFO
static void kdbg_heapinfo_init(FAR struct tcb_s *tcb, FAR void *arg)
{

	tcb->peak_alloc_size = 0;

	printf("PID %d, peak allocated heap information is cleared\n", tcb->pid);
}

static void kdbg_heapinfo_task(FAR struct tcb_s *tcb, FAR void *arg)
{
	if (tcb->pid == 0) {
		tcb->adj_stack_size = CONFIG_IDLETHREAD_STACKSIZE;
	}
	printf("%3d | ", tcb->pid);
#if defined(CONFIG_SCHED_HAVE_PARENT) && !defined(HAVE_GROUP_MEMBERS)
	printf("%5d | ", tcb->ppid);
#endif
	printf("%5d | %9d | %9d | ", tcb->adj_stack_size, tcb->curr_alloc_size, tcb->peak_alloc_size);

	/* Show task name and arguments */
#if CONFIG_TASK_NAME_SIZE > 0
	printf("%s(", tcb->name);
#else
	printf("<noname>(");
#endif
	printf(")\n");
}
#endif
int kdbg_heapinfo(int argc, char **args)
{
#ifdef CONFIG_DEBUG_MM_HEAPINFO
	int option;
	int mode = HEAPINFO_SIMPLE;
	int pid = HEAPINFO_PID_NOTNEEDED;

	if (argc >= 2 && !strcmp(args[1], "--help")) {
		goto usage;
	}

	struct mm_heap_s *user_heap = mm_get_heap_info();
	while ((option = getopt(argc, args, "iap:f")) != ERROR) {
		switch (option) {
		case 'i':
			sched_foreach(kdbg_heapinfo_init, NULL);
			printf("Peak allocated memory size is cleared\n");
			return OK;
		case 'a':
			mode = HEAPINFO_DETAIL_ALL;
			pid = HEAPINFO_PID_NOTNEEDED;
			break;
		case 'p':
			mode = HEAPINFO_DETAIL_PID;
			pid = atoi(optarg);
			break;
		case 'f':
			mode = HEAPINFO_DETAIL_FREE;
			pid = HEAPINFO_PID_NOTNEEDED;
			break;
		case '?':
		default:
			printf("Invalid option\n");
			goto usage;
		}
	}
	heapinfo_parse(user_heap, mode, pid);

	printf("\n%3s | ", "PID");
#if defined(CONFIG_SCHED_HAVE_PARENT) && !defined(HAVE_GROUP_MEMBERS)
	printf("%5s | ", "PPID");
#endif
	printf("%5s | %9s | %9s | %s\n", "STACK", "CURR_HEAP", "PEAK_HEAP", "NAME");
	printf("----|");
#if defined(CONFIG_SCHED_HAVE_PARENT) && !defined(HAVE_GROUP_MEMBERS)
	printf("-------|");
#endif
	printf("-------|-----------|-----------|----------\n");
	sched_foreach(kdbg_heapinfo_task, NULL);

	return OK;

usage:
	printf("\nUsage: heapinfo [OPTIONS]\n");
	printf("Display information of heap memory\n");
	printf("\nOptions:\n");
	printf(" -i           Initialize the heapinfo\n");
	printf(" -a           Show the all allocation details\n");
	printf(" -p PID       Show the specific PID allocation details \n");
	printf(" -f           Show the free list \n");
#endif
	return ERROR;
}
