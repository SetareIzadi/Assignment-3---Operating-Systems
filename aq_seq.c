/**
 * @file   aq.c
 * @Author 02335 team
 * @date   October, 2024
 * @brief  Alarm queue skeleton implementation
 */

#include "aq.h"
#include <stdlib.h>

typedef struct normal_msg_node {
  void *msg;
  struct normal_msg_node *next;
  struct normal_msg_node *prev;
} normal_msg_node_t;

typedef struct alarm_queue {
  void *alarm_msg;
  normal_msg_node_t *normal_head;
  normal_msg_node_t *normal_tail;
} alarm_queue_t;




AlarmQueue aq_create( ) {
  alarm_queue_t *aq_internal = (alarm_queue_t *)malloc(sizeof(alarm_queue_t));
  if (aq_internal == NULL) {
    return NULL;
  }
  aq_internal->alarm_msg = NULL;
  aq_internal->normal_head = NULL;
  aq_internal->normal_tail = NULL;
  return (AlarmQueue)aq_internal;
}

int aq_send( AlarmQueue aq, void * msg, MsgKind k){
  if (aq == NULL) {
    return AQ_UNINIT;
  }
  if(msg == NULL) {
    return AQ_NULL_MSG;
  }
  alarm_queue_t *aq_internal = (alarm_queue_t *)aq;

  if(k == AQ_ALARM) {
    if(aq_internal->alarm_msg != NULL) {
      return AQ_NO_ROOM;
    }
    aq_internal->alarm_msg = msg;
    return 0;
  } else if (k == AQ_NORMAL) {
    normal_msg_node_t *node = (normal_msg_node_t *)malloc(sizeof(normal_msg_node_t));
    if(node == NULL) {
      return AQ_NO_ROOM;
    }
    node->msg = msg;
    node->next = NULL;
    node->prev = NULL;

    if(aq_internal->normal_tail == NULL) {
      aq_internal->normal_head = node;
      aq_internal->normal_tail = node;
    }
    else {
      node->prev = aq_internal->normal_tail;
      aq_internal->normal_tail->next = node;
      aq_internal->normal_tail = node;
    }
    return 0;
  }
  else {
    return AQ_NOT_IMPL;
  }
}

int aq_recv( AlarmQueue aq, void * * msg) {
  if (aq == NULL) {
    return AQ_UNINIT;
  }
  if(msg == NULL) {
    return AQ_NULL_MSG;
  }
  alarm_queue_t *aq_internal = (alarm_queue_t *)aq;

  if(aq_internal->alarm_msg != NULL) {
    *msg = aq_internal->alarm_msg;
    aq_internal->alarm_msg = NULL;
    return AQ_ALARM;
  } else if (aq_internal->normal_head != NULL) {
    normal_msg_node_t *node = aq_internal->normal_head;
    *msg = node->msg;
    aq_internal->normal_head = node->next;
    if(aq_internal->normal_head != NULL) {
      aq_internal->normal_head->prev = NULL;
    } else {
      aq_internal->normal_tail = NULL;
    }
    free(node);
    return AQ_NORMAL;
  } else {
    return AQ_NO_MSG;
  }
}

int aq_size( AlarmQueue aq) {
  if (aq == NULL) {
    return AQ_UNINIT;
  }
  alarm_queue_t *aq_internal = (alarm_queue_t *)aq;
  int size = 0;
  if (aq_internal->alarm_msg != NULL) {
    size += 1;
  }
  normal_msg_node_t *current = aq_internal->normal_head;
  while (current != NULL) {
    size += 1;
    current = current->next;
  }
  return size;
}

int aq_alarms( AlarmQueue aq) {
  if (aq == NULL) {
    return AQ_UNINIT;
  }
  alarm_queue_t *aq_internal = (alarm_queue_t *)aq;
  return (aq_internal->alarm_msg != NULL)? 1:0;
}