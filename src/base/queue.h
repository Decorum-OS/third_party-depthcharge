/*
 * Copyright 2016 Google Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __BASE_QUEUE_H__
#define __BASE_QUEUE_H__

#include <stddef.h>
#include <stdint.h>

#include "base/container_of.h"

typedef struct QueueNode {
	struct QueueNode *next;
} QueueNode;

typedef struct Queue {
	QueueNode *head;
	QueueNode *tail;
} Queue;

// Remove and return the QueueNode from the front of the Queue queue.
QueueNode *queue_pop(Queue *queue);

// Return the first node in the Queue queue without removing it.
static inline QueueNode *queue_peek(Queue *queue)
{
	return queue->head;
}

// Check whether a queue is empty
static inline int queue_empty(Queue *queue)
{
	return queue_peek(queue) == NULL;
}

// Add a QueueNode node to the Queue queue.
void queue_push(QueueNode *node, Queue *queue);

#endif /* __BASE_QUEUE_H__ */
