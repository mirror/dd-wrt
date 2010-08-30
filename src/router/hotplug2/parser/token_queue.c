#include "token_queue.h"

void token_queue_init(struct token_queue_t *queue) {
	queue->size = 16;
	queue->queue = malloc(sizeof(struct token_t*) * queue->size);
	queue->top = -1;
}

void token_queue_push(struct token_queue_t *queue, struct token_t *token) {
	queue->queue[++queue->top] = token;
	if (queue->top + 1 >= queue->size) {
		queue->size *= 2;
		queue->queue = realloc(queue->queue, sizeof(struct token_t*) * queue->size);
	}
}

struct token_t *token_queue_pop(struct token_queue_t *queue) {
	struct token_t *token;
	if (queue->top == -1)
		return NULL;
	
	token = queue->queue[queue->top];
	queue->queue[queue->top--] = NULL;
	
	return token;
}

struct token_t *token_queue_dequeue(struct token_queue_t *queue) {
	struct token_t *token;
	if (queue->top == -1)
		return NULL;
	
	token = queue->queue[0];
	memmove(queue->queue, &queue->queue[1], queue->top-- * sizeof(struct token_t*));
	return token;
}

char **token_queue_strings(struct token_queue_t *queue) {
	char **strings;
	int i;

	strings = malloc(sizeof(char *) * (token_queue_size(queue) + 1));
	strings[token_queue_size(queue)] = NULL;
	for (i = 0; i <= queue->top; i++) {
		strings[i] = strdup(queue->queue[i]->token);
	}

	return strings;
}

int token_queue_size(struct token_queue_t *queue) {
	return queue->top+1;
}

void token_queue_inspect(struct token_queue_t *queue) {
	int i;
	
	printf("---queue---\n");
	if (queue->top >= 0) {
		for (i = queue->top; i >= 0; i--) {
			printf("[%2d/%2d]: (%p) %s (%d)\n", i, queue->top, (void*)queue->queue[i], queue->queue[i]->token, queue->queue[i]->type);
		}
	}
	printf("---/queue--\n\n");
}

void token_queue_empty(struct token_queue_t *queue) {
	int i;

	if (queue->top >= 0) {
		for (i = queue->top; i >= 0; i--) {
			token_free(queue->queue[i]);
		}
	}
	queue->top = -1;
}

void token_queue_free(struct token_queue_t *queue) {
	token_queue_empty(queue);
	free(queue->queue);
}
