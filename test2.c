#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h> // For usleep

#include "aq.h"
#include "aux.h"

static AlarmQueue q;

void *producer(void *arg) {
    // Send normal messages first
    int ret = put_normal(q, 1);
    if (ret < 0) {
        printf("Producer: Error sending Normal1\n");
    } else {
        printf("Producer: Sent Normal1\n");
        fflush(stdout);
    }

    ret = put_normal(q, 2);
    if (ret < 0) {
        printf("Producer: Error sending Normal2\n");
    } else {
        printf("Producer: Sent Normal2\n");
        fflush(stdout);
    }

    msleep(500); // Delay before sending alarm

    // Send an alarm message
    ret = put_alarm(q, 3);
    if (ret < 0) {
        printf("Producer: Error sending Alarm1\n");
    } else {
        printf("Producer: Sent Alarm1\n");
        fflush(stdout);
    }

    return NULL;
}

void *consumer(void *arg) {
    msleep(1000); // Sleep to allow messages to be sent before receiving

    int msg;

    msg = get(q);
    if (msg >= 0) {
        printf("Consumer: Received %d\n", msg);
        fflush(stdout);
    } else {
        printf("Consumer: Error receiving message\n");
        fflush(stdout);
    }

    msg = get(q);
    if (msg >= 0) {
        printf("Consumer: Received %d\n", msg);
        fflush(stdout);
    } else {
        printf("Consumer: Error receiving message\n");
        fflush(stdout);
    }

    msg = get(q);
    if (msg >= 0) {
        printf("Consumer: Received %d\n", msg);
        fflush(stdout);
    } else {
        printf("Consumer: Error receiving message\n");
        fflush(stdout);
    }

    return NULL;
}

int main() {
    q = aq_create();
    if (q == NULL) {
        fprintf(stderr, "Failed to create alarm queue\n");
        exit(EXIT_FAILURE);
    }

    pthread_t producer_thread, consumer_thread;

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    return 0;
}
