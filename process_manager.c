/*
*
* process_manager.c
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"

typedef struct {
   int id;
   int items_to_produce;
   Queue *belt;
   int belt_size;
} ProcessManagerArgs;

static void* producer(void* arg) {
   ProcessManagerArgs* args = (ProcessManagerArgs*)arg;
   for (int i = 0; i < args->items_to_produce; i++) {
	   struct element* ele = malloc(sizeof(struct element));
	   ele->num_edition = i;
	   ele->id_belt = args->id;
      if (i==args->items_to_produce-1){
         ele->last = 1;
      }
      Queue *queue = args->belt;
	   queue_put(queue, ele);
	   printf("[OK][queue] Introduced element with id %d in belt %d.\n", i, args->id);
   }
   pthread_exit(NULL);
}

static void* consumer(void* arg) {
   ProcessManagerArgs* args = (ProcessManagerArgs*)arg;
   Queue *queue = args->belt;
   while (1) {
	   struct element* ele = queue_get(queue);
	   if (ele->last) {
		   printf("[OK][queue] Obtained element with id %d in belt %d.\n", ele->num_edition, args->id);
		   free(ele);
		   break;
	   }
	   printf("[OK][queue] Obtained element with id %d in belt %d.\n", ele->num_edition, args->id);
	   free(ele);
   }
   pthread_exit(NULL);
}

int process_manager(int id, int belt_size, int items_to_produce) {
   // Wait for factory_manager's signal to start
   printf("[OK][process_manager] Process_manager with id %d waiting to produce %d elements.\n", id, items_to_produce);

   // Initialize the belt (queue)
   int feedback;
   Queue queue = queue_create(belt_size, &feedback);
   if (feedback) {
	   printf(stderr, "[ERROR][process_manager] Belt initialization failed for id %d.\n", id);
	   return -1;
   }
   printf("[OK][process_manager] Belt with id %d has been created with a maximum of %d elements.\n", id, belt_size);

   // Create producer and consumer threads
   pthread_t producer_thread, consumer_thread;
   ProcessManagerArgs args = {id, items_to_produce, &queue, belt_size};

   if (pthread_create(&producer_thread, NULL, producer, &args) != 0) {
	   printf(stderr, "[ERROR][process_manager] Failed to create producer thread.\n");
	   queue_destroy(&queue);
	   return -1;
   }

   if (pthread_create(&consumer_thread, NULL, consumer, &args) != 0) {
	   printf(stderr, "[ERROR][process_manager] Failed to create consumer thread.\n");
	   pthread_cancel(producer_thread);
	   queue_destroy(&queue);
	   return -1;
   }

   // Wait for threads to finish
   pthread_join(producer_thread, NULL);
   pthread_join(consumer_thread, NULL);

   // Cleanup
   if (queue_destroy(&queue)) {
	   printf(stderr, "[ERROR][process_manager] Belt destruction failed for id %d.\n", id);
	   return -1;
   }

   printf("[OK][process_manager] Process_manager with id %d has produced %d elements.\n", id, items_to_produce);
   return 0;
}