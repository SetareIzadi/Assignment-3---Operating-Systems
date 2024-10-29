#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h> // For usleep

#include "aq.h"
#include "aux.h"

static AlarmQueue q;

void *alarm_producer(void *arg) {
    int ret = put_alarm(q, 1);
    if (ret < 0) {
        printf("Alarm Producer: Error sending Alarm1\n");
    } else {
        printf("Alarm Producer: Sent Alarm1\n");
        fflush(stdout);
    }

    ret = put_alarm(q, 2); // Should block until Alarm1 is received
    if (ret < 0) {
        printf("Alarm Producer: Error sending Alarm2\n");
    } else {
        printf("Alarm Producer: Sent Alarm2\n");
        fflush(stdout);
    }
    return NULL;
}

void *normal_producer(void *arg) {
    msleep(500); // Sleep to ensure the alarm producer runs first
    int ret = put_normal(q, 3);
    if (ret < 0) {
        printf("Normal Producer: Error sending Normal1\n");
    } else {
        printf("Normal Producer: Sent Normal1\n");
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

    pthread_t alarm_thread, normal_thread, consumer_thread;

    pthread_create(&alarm_thread, NULL, alarm_producer, NULL);
    pthread_create(&normal_thread, NULL, normal_producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(alarm_thread, NULL);
    pthread_join(normal_thread, NULL);
    pthread_join(consumer_thread, NULL);

    return 0;
}
