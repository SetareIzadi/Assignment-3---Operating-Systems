/**
 * @file   aq_tsafe.c
 * @Author Your Name
 * @date   October, 2024
 * @brief  Thread-safe Alarm Queue implementation
 */

#include "aq.h"
#include <stdlib.h>
#include <pthread.h>

// Node for normal messages (doubly linked list)
typedef struct normal_msg_node {
    void *msg;
    struct normal_msg_node *next;
    struct normal_msg_node *prev;
} normal_msg_node_t;

// Structure for the alarm queue
typedef struct alarm_queue {
    void *alarm_msg;                    // Pointer to the alarm message (NULL if none)
    normal_msg_node_t *normal_head;     // Head of normal messages linked list
    normal_msg_node_t *normal_tail;     // Tail of normal messages linked list
    pthread_mutex_t mutex;              // Mutex for thread safety
    pthread_cond_t not_empty;           // Condition variable for consumers
    pthread_cond_t no_alarm;            // Condition variable for alarm producers
} alarm_queue_t;

AlarmQueue aq_create() {
    alarm_queue_t *aq_internal = (alarm_queue_t *)malloc(sizeof(alarm_queue_t));
    if (aq_internal == NULL) {
        return NULL; // Allocation failed
    }
    // Initialize the queue
    aq_internal->alarm_msg = NULL;
    aq_internal->normal_head = NULL;
    aq_internal->normal_tail = NULL;
    // Initialize mutex and condition variables
    pthread_mutex_init(&aq_internal->mutex, NULL);
    pthread_cond_init(&aq_internal->not_empty, NULL);
    pthread_cond_init(&aq_internal->no_alarm, NULL);
    return (AlarmQueue)aq_internal;
}

int aq_send(AlarmQueue aq, void *msg, MsgKind k) {
    if (aq == NULL) {
        return AQ_UNINIT; // Queue has not been initialized
    }
    if (msg == NULL) {
        return AQ_NULL_MSG; // Sent message is NULL
    }
    alarm_queue_t *aq_internal = (alarm_queue_t *)aq;

    pthread_mutex_lock(&aq_internal->mutex); // Lock the mutex

    if (k == AQ_ALARM) {
        // Wait until there is no alarm message
        while (aq_internal->alarm_msg != NULL) {
            pthread_cond_wait(&aq_internal->no_alarm, &aq_internal->mutex);
        }
        // Insert the alarm message
        aq_internal->alarm_msg = msg;
    } else if (k == AQ_NORMAL) {
        // Create a new node for the normal message
        normal_msg_node_t *node = (normal_msg_node_t *)malloc(sizeof(normal_msg_node_t));
        if (node == NULL) {
            pthread_mutex_unlock(&aq_internal->mutex); // Unlock before returning
            return AQ_NO_ROOM; // Memory allocation failed
        }
        node->msg = msg;
        node->next = NULL;
        node->prev = NULL;

        // Insert the node into the doubly linked list
        if (aq_internal->normal_tail == NULL) {
            // The list is empty
            aq_internal->normal_head = node;
            aq_internal->normal_tail = node;
        } else {
            // Append to the end of the list
            node->prev = aq_internal->normal_tail;
            aq_internal->normal_tail->next = node;
            aq_internal->normal_tail = node;
        }
    } else {
        pthread_mutex_unlock(&aq_internal->mutex); // Unlock before returning
        return AQ_NOT_IMPL;
    }

    // Signal that the queue is not empty
    pthread_cond_signal(&aq_internal->not_empty);
    pthread_mutex_unlock(&aq_internal->mutex); // Unlock the mutex
    return 0; // Success
}

int aq_recv(AlarmQueue aq, void **msg) {
    if (aq == NULL) {
        return AQ_UNINIT; // Queue has not been initialized
    }
    if (msg == NULL) {
        return AQ_NULL_MSG; // Null pointer provided for message output
    }
    alarm_queue_t *aq_internal = (alarm_queue_t *)aq;

    pthread_mutex_lock(&aq_internal->mutex); // Lock the mutex

    // Wait until the queue is not empty
    while (aq_internal->alarm_msg == NULL && aq_internal->normal_head == NULL) {
        pthread_cond_wait(&aq_internal->not_empty, &aq_internal->mutex);
    }

    int ret_kind;
    if (aq_internal->alarm_msg != NULL) {
        // Retrieve the alarm message
        *msg = aq_internal->alarm_msg;
        aq_internal->alarm_msg = NULL;
        ret_kind = AQ_ALARM;
        // Signal that there is now no alarm message
        pthread_cond_signal(&aq_internal->no_alarm);
    } else {
        // Retrieve the normal message from the head
        normal_msg_node_t *node = aq_internal->normal_head;
        *msg = node->msg;
        aq_internal->normal_head = node->next;
        if (aq_internal->normal_head != NULL) {
            // There are more nodes in the list
            aq_internal->normal_head->prev = NULL;
        } else {
            // The list is now empty
            aq_internal->normal_tail = NULL;
        }
        free(node);
        ret_kind = AQ_NORMAL;
    }

    pthread_mutex_unlock(&aq_internal->mutex); // Unlock the mutex
    return ret_kind; // Return the kind of message received
}

int aq_size(AlarmQueue aq) {
    if (aq == NULL) {
        return AQ_UNINIT;
    }
    alarm_queue_t *aq_internal = (alarm_queue_t *)aq;

    pthread_mutex_lock(&aq_internal->mutex); // Lock the mutex
    int size = 0;
    if (aq_internal->alarm_msg != NULL) {
        size += 1;
    }
    normal_msg_node_t *current = aq_internal->normal_head;
    while (current != NULL) {
        size += 1;
        current = current->next;
    }
    pthread_mutex_unlock(&aq_internal->mutex); // Unlock the mutex
    return size;
}

int aq_alarms(AlarmQueue aq) {
    if (aq == NULL) {
        return AQ_UNINIT;
    }
    alarm_queue_t *aq_internal = (alarm_queue_t *)aq;

    pthread_mutex_lock(&aq_internal->mutex); // Lock the mutex
    int alarms = (aq_internal->alarm_msg != NULL) ? 1 : 0;
    pthread_mutex_unlock(&aq_internal->mutex); // Unlock the mutex
    return alarms;
}

// Optional: Implement aq_destroy if needed
void aq_destroy(AlarmQueue aq) {
    if (aq == NULL) {
        return;
    }
    alarm_queue_t *aq_internal = (alarm_queue_t *)aq;

    // Lock the mutex to prevent other threads from accessing the queue
    pthread_mutex_lock(&aq_internal->mutex);

    // Free any remaining messages
    if (aq_internal->alarm_msg != NULL) {
        free(aq_internal->alarm_msg);
    }
    normal_msg_node_t *current = aq_internal->normal_head;
    while (current != NULL) {
        normal_msg_node_t *next = current->next;
        free(current->msg);
        free(current);
        current = next;
    }

    // Unlock the mutex
    pthread_mutex_unlock(&aq_internal->mutex);

    // Destroy mutex and condition variables
    pthread_mutex_destroy(&aq_internal->mutex);
    pthread_cond_destroy(&aq_internal->not_empty);
    pthread_cond_destroy(&aq_internal->no_alarm);

    // Free the queue structure
    free(aq_internal);
}
