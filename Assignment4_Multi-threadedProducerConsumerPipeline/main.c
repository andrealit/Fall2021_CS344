// Assignment 4: Multi-threaded Producer Consumer Pipeline
// Andrea Tongsak
// CS344 Fall 2021

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

/*
 * Constants for buffers
 */
#define SIZE 1000
#define LINES 50

#define STOP_SYMBOL "STOP\n"
#define LINE_BREAK '\n'
#define SPACE ' '
//#define PLUS_PLUS_SYMBOL "++"
//#define CARROT '^'

// Buffer 1: shared resource between input thread and line separator thread
char buffer_1[LINES][SIZE];
// Initialize the mutex 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;
// number of items in buffer 1
int count_1 = 0;
// index where the input thread will put the next item
int prod_idx_1 = 0;
// index where the thread will pick up the next item
int con_idx_1 = 0;

// Buffer 2: shared resource between line separator thread and plus sign thread
char buffer_2[LINES][SIZE];
// Initialize the mutex 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;
// number of items in buffer 2
int count_2 = 0;
// index where the input thread will put the next item
int prod_idx_2 = 0;
// index where the thread will pick up the next item
int con_idx_2 = 0;

// Buffer 3: shared resource between plus sign thread and output thread
char buffer_3[LINES][SIZE];
// Initialize the mutex 3
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;
// number of items in buffer 2
int count_3 = 0;
// index where the input thread will put the next item
int prod_idx_3 = 0;
// index where the thread will pick up the next item
int con_idx_3 = 0;

/*
 get user input
 */
char * get_user_input() {
    char * input = NULL;
    size_t len = 0;
    getline(&input, &len, stdin);
    return input;
}

// THREAD HANDLING
/*
 Put in data into buffer 1
 */
void put_buff_1(char * line) {
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_1);
    // Put the item in the buffer
    strcpy(buffer_1[prod_idx_1], line);
    // Increment the index where the next item will be put.
    prod_idx_1 = prod_idx_1 + 1;
    count_1++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_1);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);
}

// NOTE: may need to create a private buffer that then moves into get_buff_1
// This is because the lock/unlock mechanism in a buffer may not correctly work unless
// The buffer is placed above the lock line.

/*
 * Function that the input thread will run.
 * Put the item in the buffer shared with the line_separator thread.
*/
void *get_input(void *args)
{
   for (int i = 0; i < LINES; i++){
      // Get input from the user
      char *input = get_user_input();
      // If stop processing line received, stop processing inputs
      if (strcmp(input, STOP_SYMBOL) == 0) {
         put_buff_1(input);
         return NULL;
      }
      put_buff_1(input);
      free(input);
   }
   return NULL;
}

/*
 Get the next item from buffer 1
 */
char * get_buff_1() {
    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_1);
    while (count_1 == 0) {
      // Buffer is empty. Wait for the producer to signal that the buffer has data
      pthread_cond_wait(&full_1, &mutex_1);
    }
    char * line = buffer_1[con_idx_1];
    // Increment the index from which the item will be picked up
    con_idx_1 = con_idx_1 + 1;
    count_1--;
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);
    // Return the item
    return line;
}

/*
 Put in data into buffer 2
 */
void put_buff_2(char * line) {
    // Lock Mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_2);
    // Fill buffer - put the item in the buffer
    strcpy(buffer_2[prod_idx_2], line);
    // Increment buffer index where the next item will be put.
    prod_idx_2 = prod_idx_2 + 1;
    count_2++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_2);
    // Unlock Mutex
    pthread_mutex_unlock(&mutex_2);
}

/*
 replace_line_separator(): finds \n, change it to ' '
 */
void * replace_line_separator(void * args) {
    char *input;
    for (int i = 0; i < LINES; i++) {
       input = get_buff_1();
       // If stop processing line received, stop processing inputs
       if (strcmp(input, STOP_SYMBOL) == 0) {
          put_buff_2(input);
          return NULL;
       }
       // Loop through entire input to check for line separator
       for (int j = 0; input[j] != '\0'; j++) {
          if (input[j] == LINE_BREAK) {
             input[j] = SPACE;
          }
       }
       put_buff_2(input);
    }
    return NULL;
}

/*
 Get the data from buffer 2
 */
char * get_buff_2() {
    // Lock mutex
    pthread_mutex_lock(&mutex_2);
    // if count_2 is 0 then buffer is empty.
    // Then wait for the producer to signal that the buffer has data
    while (count_2 == 0) {
        pthread_cond_wait(&full_2, &mutex_2);
    }
    // Copy the buffer string into item
    char *line = buffer_2[con_idx_2];
    // Increment consumer index from which the item will be picked up and decrement count_2
    con_idx_2 = con_idx_2 + 1;
    count_2--;
    // Unlock mutex
    pthread_mutex_unlock(&mutex_2);
    // Return the item
    return line;
}

/*
 Put in data into buffer 3
 */
void put_buff_3(char * line) {
    // Lock Mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_3);
    // Fill buffer - put the item in the buffer
    strcpy(buffer_3[prod_idx_3], line);
    // Increment buffer index where the next item will be put.
    prod_idx_3 = prod_idx_3 + 1;
    count_3++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_3);
    // Unlock Mutex
    pthread_mutex_unlock(&mutex_3);
}

/*
 replace_plus_carrot(): finds every ++ sign pair, changes it for the ^ symbol
 */
void * replace_plus_carrot(void * args) {
    char * input;
    for (int i = 0; i < LINES; i++) {
        input = get_buff_2();
        // if stop processing is found, stop processing the inputs
        if (strcmp(input, STOP_SYMBOL) == 0) {
           put_buff_3(input);
           return NULL;
        }
        // loop through pair of plus signs while not NULL
        for (int j = 0; input[j] != '\0'; j++) {
            // found a input that has two plus characters
            if (input[j] == '+' && input[j+1] == '+') {
                input[j] = '^';
                for (int k = j+1; k < SIZE; k++) {
                    input[k] = input[k+1];
                }
            }
        }
        put_buff_3(input);
    }
    return NULL;
}


/*
 Get the data from buffer 3
 */
char * get_buff_3() {
    // Lock mutex
    pthread_mutex_lock(&mutex_3);
    // if count_3 is 0 then buffer is empty.
    // Then wait for the producer to signal that the buffer has data
    while (count_3 == 0) {
        pthread_cond_wait(&full_3, &mutex_3);
    }
    // Copy the buffer string into item
    char *line = buffer_3[con_idx_3];
    // Increment consumer index from which the item will be picked up and decrement count_2
    con_idx_3 = con_idx_3 + 1;
    count_3--;
    // Unlock mutex
    pthread_mutex_unlock(&mutex_3);
    // Return the item
    return line;
}

/*
 write_output(): write this processed data to standard output, needs to be exactly 80 characters
 at the third buffer
 */
void * write_output(void * args) {
    char **char_array;
    char_array = calloc(650, sizeof(char));
    for (int i = 0; i < 650; i++) {
       char_array[i] = calloc(81, sizeof(char));
    }
    char *input;
    int input_idx;        // index of char in user input
    int char_idx = 0;     // Current index of char in string in char_array
    int output_idx = 0;   // Current index of string in char_array

    for (int i = 0; i < LINES; i++) {
       input_idx = 0;
       input = get_buff_3();
       if (strcmp(input, STOP_SYMBOL) == 0) {
          return NULL;
       }
       // Loop until end of current input
       while (input[input_idx]) {
          // Store char, from input, in char_array
          char_array[output_idx][char_idx] = input[input_idx];
          // If 80 chars in string reached, print to standard output
          if (char_idx == 79) {
             char_array[output_idx][80] = '\0';
             printf("%s\n", char_array[output_idx]);
             // Increment 'string' index to start storing next 80 chars
             output_idx++;
             // Reset char index in string to 0.
             char_idx = 0;
             fflush(stdout);
          }
          else {
             // If current string does not have 80 chars, increment char_index
             char_idx++;
          }
          // Increment to next char in user input
          input_idx++;
       }
    }
    return NULL;
}

/*
 main function handles the threads
 */
int main()
{
    // Define threads
    pthread_t input_thread;
    pthread_t line_separator_thread;
    pthread_t plus_sign_thread;
    pthread_t output_thread;

    // Create the threads
    pthread_create(&input_thread, NULL, get_input, NULL);
    pthread_create(&line_separator_thread, NULL, replace_line_separator, NULL);
    pthread_create(&plus_sign_thread, NULL, replace_plus_carrot, NULL);
    pthread_create(&output_thread, NULL, write_output, NULL);

    // Wait for the threads to terminate
    pthread_join(input_thread, NULL);
    pthread_join(line_separator_thread, NULL);
    pthread_join(plus_sign_thread, NULL);
    pthread_join(output_thread, NULL);

    return EXIT_SUCCESS;
}
