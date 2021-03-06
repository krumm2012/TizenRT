/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
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
/**
 * @defgroup TTRACE_LIBC TTRACE
 * @ingroup KERNEL
 *
 * @{
 */

///@file ttrace_internal.h
///@brief ttrace internal APIs

#ifndef __INCLUDE_TINYARA_TTRACE_INTERNAL_H
#define __INCLUDE_TINYARA_TTRACE_INTERNAL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <debug.h>
#include <time.h>
#include <sys/types.h>

/****************************************************************************
 * Public Type Declarations
 ****************************************************************************/
#define TTRACE_START               's'
#define TTRACE_FINISH              'f'
#define TTRACE_INFO                'i'
#define TTRACE_SELECTED_TAG        't'
#define TTRACE_FUNC_TAG            'g'
#define TTRACE_USED_BUFSIZE        'u'
#define TTRACE_BUFFER              'b'
#define TTRACE_DUMP                'd'
#define TTRACE_PRINT               'p'

#define TTRACE_CODE_VARIABLE        0
#define TTRACE_CODE_UNIQUE         (1 << 7)

#define TTRACE_EVENT_TYPE_BEGIN    'b'
#define TTRACE_EVENT_TYPE_END      'e'

#define TTRACE_MSG_BYTES            32
#define TTRACE_COMM_BYTES           12
#define TTRACE_BYTE_ALIGN           4

#define TTRACE_INVALID             -1
#define TTRACE_VALID                0

/****************************************************************************
 * Public Variables
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#if defined(__cplusplus)
extern "C"
{
#endif

struct tag_list {
	const char *name;
	const char *longname;
	const int tags;
};

static const struct tag_list ttrace_tags[] = {
	{"none",    "None",          TTRACE_TAG_OFF},
	{"apps",    "Applications",  TTRACE_TAG_APPS},
	{"libs",    "Libraries",     TTRACE_TAG_LIBS},
	{"lock",    "Lock",          TTRACE_TAG_LOCK},
	{"task",    "TASK",          TTRACE_TAG_TASK},
	{"ipc",     "IPC",           TTRACE_TAG_IPC},
};

struct sched_message {          // total 32B
	pid_t prev_pid;                     // 2B
	uint8_t prev_prio;                  // 1B
	uint8_t prev_state;                 // 1B
	char prev_comm[TTRACE_COMM_BYTES];  // 12B
	pid_t next_pid;                     // 2B
	uint8_t next_prio;                  // 1B
	int8_t pad;                         // 1B
	char next_comm[TTRACE_COMM_BYTES];  // 12B
};

union trace_message {              // total 32B
	char message[TTRACE_MSG_BYTES];  // 32B, message(256b)
	struct sched_message sched_msg;  // 32B
};

struct trace_packet {        // total 44 byte(message), 12byte(uid)
	struct timeval ts;         // 8B, time_t(32b) + long(32b)
	pid_t pid;                 // 2B, int16_t(16b)
	char event_type;           // 1B, char(8b)
	int8_t codelen;            // 1B, code(1b) + variable length(7b) or uid(7b)
	int32_t pad;
	union trace_message msg;   // 32B
};

static int show_packet(struct trace_packet *packet)
{
	int uid = (packet->codelen & TTRACE_CODE_UNIQUE) >> 7;
	int msg_len = (packet->codelen & ~TTRACE_CODE_UNIQUE);
	int pad = 0;
	ttdbg("time: %06d.%06d\r\n", packet->ts.tv_sec, packet->ts.tv_usec);
	ttdbg("event_type: %c, %d\r\n", packet->event_type, packet->event_type);
	ttdbg("pid: %d\r\n", packet->pid);
	ttdbg("codelen: %d\r\n", packet->codelen);
	ttdbg("unique code? %d\r\n", uid);
	if (uid == TRUE) {
		ttdbg("uid: %d\r\n", (packet->codelen & ~TTRACE_CODE_UNIQUE));
		pad = TTRACE_MSG_BYTES;
	} else {
		ttdbg("message: %s\r\n", packet->msg.message);
		pad = 0;
	}

	return sizeof(struct trace_packet) - pad;
}

static int show_sched_packet(struct trace_packet *packet)
{
	ttdbg("[%06d:%06d] %03d: %c|prev_comm=%s prev_pid=%u prev_prio=%u prev_state=%u ==> next_comm=%s next_pid=%u next_prio=%u\r\n",
		  packet->ts.tv_sec, packet->ts.tv_usec,
		  packet->pid,
		  (char)packet->event_type,
		  packet->msg.sched_msg.prev_comm,
		  packet->msg.sched_msg.prev_pid,
		  packet->msg.sched_msg.prev_prio,
		  packet->msg.sched_msg.prev_state,
		  packet->msg.sched_msg.next_comm,
		  packet->msg.sched_msg.next_pid,
		  packet->msg.sched_msg.next_prio);
	return sizeof(struct trace_packet);
}

#if defined(__cplusplus)
}
#endif

#endif /* __INCLUDE_TINYARA_TTRACE_INTERNAL_H */
/** @} */
