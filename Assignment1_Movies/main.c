// Works Cited:
// Andrea Tongsak
// CS344 OPERATING SYSTEMS I (CS_344_001_F2021)
// October 10, 2021
/* Exploration - System Calls, and Reading Writing Files in C */
/* Exploration - Variable & Data Types, Input & Output in C */
/* Exploration: From C Programs to Machine Code */
// students.c example on the Assignment 1 page

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* struct for movie information */
struct movie {
    char *title;
    int year;
    char *languages;
    double rating_value;
    struct movie *next;
};

/**
 * Helper function to add a movie node into a linked list
 */
void addMovie(struct movie **head, char* title, int year, char* languages, double rating_value) {
        
    // allocate new movie node
    struct movie* newNode = (struct movie*)malloc(sizeof(struct movie));
    newNode->title = calloc(strlen(title) + 1, sizeof(char));
    strcpy(newNode->title, title);
    newNode->languages = calloc(strlen(languages) + 1, sizeof(char));
    strcpy(newNode->languages, languages);
    newNode->year = year;
    newNode->rating_value = rating_value;
    newNode->next = NULL;
    
    // if head is NULL, it is an empty list
    if (*head == NULL)
       *head = newNode;
    // otherwise, find the last node and add the newNode
    else {
       struct movie *lastnode = *head;
       while (lastnode->next != NULL) {
          lastnode = lastnode->next;
        }
        // add the newNode at the end of the linked list
        lastnode->next = newNode;
    }
}

/**
 * Allocate memory for each line and movie value
 */
struct movie *createMovie(char *currLine) {
    struct movie *currMovie = malloc(sizeof(struct movie));

    // using strtok_r
    char *saveptr;

    // The first token is title
    char *token_title = strtok_r(currLine, ",", &saveptr);
    currMovie->title = calloc(strlen(token_title) + 1, sizeof(char));
    strcpy(currMovie->title, token_title);

    // The second token is the year
    int token_year = atoi(strtok_r(NULL, ",", &saveptr));
    currMovie->year = token_year;

    // The third token is the language
    char *token_language = strtok_r(NULL, ",", &saveptr);
    currMovie->languages = calloc(strlen(token_language) + 1, sizeof(char));
    strcpy(currMovie->languages, token_language);

    // The fourth token is the rating
    double token_rating = strtod(saveptr, &saveptr);
    currMovie->rating_value = token_rating;

    // Set the next node to NULL in the newly created movie entry
    currMovie->next = NULL;

    return currMovie;
};

/**
 * Return linked list of movies by parsing data from each line of the specified file.
 */
struct movie *processFile(char *filePath, int *movieCount) {
    // Open the specified file for reading only
    FILE *movieFile = fopen(filePath, "r");

    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;
    // char *token;
    int lineCnt = 0;

    // The head of the linked list
    struct movie *head = NULL;
    // The tail of the linked list
    struct movie *tail = NULL;

    // Read the file line by line
    while ((nread = getline(&currLine, &len, movieFile)) != -1)
    {
        if (lineCnt > 0) {
           // Get a new student node corresponding to the current line
           struct movie *newNode = createMovie(currLine);

           // Is this the first node in the linked list?
           if (head == NULL)
           {
               // This is the first node in the linked link
               // Set the head and the tail to this node
               head = newNode;
               tail = newNode;
           }
           else
           {
               // This is not the first node.
               // Add this node to the list and advance the tail
               tail->next = newNode;
               tail = newNode;
           }
        }
        // Track how many movies were entered (after the first row)
        lineCnt = lineCnt + 1;
    }
    *movieCount = lineCnt-1;
    free(currLine);
    fclose(movieFile);
    return head;
}

/* (1) Show movies and their specified year */
void movieYear(struct movie *list, int userYear) {
    bool match = false;

    // check the linked list for years that match
    while (list) {
        if (userYear == list->year) {
            match = true;
            printf("%s\n", list->title);
        }
        list = list->next;
    }
    if (!match) {
        printf("No data about movies released in the year %i\n", userYear);
    }
}

/**
 * HELPER Function: Print the linked lists of movies using pointer
 */
void printList(struct movie* list) {
    struct movie* ptr = list;
    while (ptr) {
        printf("%d %.1f %s\n", ptr->year, ptr->rating_value, ptr->title);
        ptr = ptr->next;
    }
}
/**
 * (2) Show the highest rated movie for each year
 */
void highestRatedMovie(struct movie *list) {

    // create a new linkedlist called ranked
    struct movie *ranked = malloc(sizeof(struct movie));
    // uss ptr to traverse ranked linkedlist
    struct movie *ptr = malloc(sizeof(struct movie));
    // use movies to traverse the given movie linkedlist
    struct movie *movies = malloc(sizeof(struct movie));
    // have a flag when found the movie that matching the year
    bool found = false;
    
    // set ptr to point to the head of ranked linkedlist
    // *ptr = *ranked;
    
    // first occurance so just copy the 1st node over
    ranked->title = calloc(strlen(list->title)+1, sizeof(char));
    strcpy(ranked->title, list->title);
    ranked->year = list->year;
    ranked->languages = calloc(strlen(list->languages)+1, sizeof(char));
    strcpy(ranked->languages, list->languages);
    ranked->rating_value = list->rating_value;
    ranked->next = NULL;

    // move to the 2nd mode of the main movies linkedlist
    movies = list->next;
    
    // cycle through original list
    while (movies != NULL) {
        // set ptr to points to the 1st node of ranked linkedlist
        ptr = ranked;
        found = false;
        while (ptr != NULL) {
            // if they are in the same year
            if (movies->year == ptr->year) {
                // the movie is found
                found = true;
                // if the rating of node from movies linkedlist is higher
                if (movies->rating_value > ptr->rating_value) {
                    // overwrite with the higher rating information
                    ptr->title = calloc(strlen(movies->title)+1, sizeof(char));
                    strcpy(ptr->title, movies->title);
                    ptr->rating_value = movies->rating_value;
                    ptr->languages = calloc(strlen(movies->languages)+1, sizeof(char));
                    strcpy(ptr->languages, movies->languages);
                }
            }
            // move to next node in the ranked linkedlist
            ptr = ptr->next;
        }
        // since no more movie to compare between the two linkedlists
        // add a new node since current year doesn't exist and movie is still not found
        if ((ptr == NULL) && (found == false)) {
            addMovie(&ranked, movies->title, movies->year, movies->languages, movies->rating_value);
        }
        // move to the next node in the main movies linkedlist
        movies = movies->next;
     }

    // print out the final ranked linked list
    printList(ranked);
    // free the ranked linkedlist (since it is printed out)
    free(ranked);
    free(ptr);
    free(movies);
}

/* (3) Show movies and their year of release for a language */
void movieLanguage(struct movie *list, char *language) {
    char *word;
    char *temp;
    bool match = false;

    // Iterate the linked list
    while (list) {
        // Within the line, search for language match
        // https://www.tutorialspoint.com/c_standard_library/c_function_strstr.htm
        word = strstr(list->languages, language);
        if (word != NULL) {
            // remove extra ; and ]
            temp = calloc(strlen(word)+1, sizeof(char));
            strcpy(temp, word);
            char *token = strtok(temp, ";]");
            // Compare the token with the language entered
            // https://www.tutorialspoint.com/c_standard_library/c_function_strstr.htm
            if (strcmp(token, language) == 0) {
                match = true;
                printf("%d %s\n", list->year, list->title);
            }
        }
        list = list->next;
    }

    if (!match) {
        printf("No data about movies released in %s\n", language);
    }
}

/**
 * Process the CSV file provided as an argument to create linked lists
 * Compile using:
 *  gcc --std=gnu99 -o students main.c
 */
int main (int argc, char* argv[]) {
    if (argc < 2) {
        printf("Please provide the name of the file to process\n");
        printf("Example usage: ./movies movies_sample_1.csv\n");
        return EXIT_FAILURE;
    }

    // screen the file
    FILE* f = fopen(argv[1], "rw");
    if (f == NULL) {
        printf("Please provide a valid file name to process\n");
        printf("Example usage: ./movies movies_sample_1.csv\n");
        return EXIT_FAILURE;
    }

    int movieCount = 0;
    struct movie *list = processFile(argv[1], &movieCount);
    //struct movie *rankedList = NULL;

    printf("\nProcessed file %s and parsed data for %d movies.\n\n", argv[1], movieCount);

    // User interface
    printf("1. Show movies released in the specified year\n");
    printf("2. Show highest rated movie for each year\n");
    printf("3. Show the title and year of release of all movies in a specific language\n");
    printf("4. Exit from the program\n");

    int choice;
    do {
        printf("\nEnter a choice from 1 to 4: ");
        scanf("%d", &choice);

        if (choice < 1 || choice > 4) {
            printf("You entered an incorrect choice. Try again.\n");
        } else {
            // Displays movies from a certain year
            if (choice == 1) {
                int year;
                printf("Enter the year for which you want to see movies: ");
                scanf("%d", &year);
                movieYear(list, year);
            }
            // Displays the movies with the highest rating that year
            else if (choice == 2) {
                highestRatedMovie(list);
            }
            // Show movies and their year of release for a specific language
            else if (choice == 3) {
                char *language = calloc(20, sizeof(char));
                printf("Enter the language which you want to see the movies: ");
                scanf("%s", language);
                movieLanguage(list, language);
            }
        }
    } while (choice != 4);

    // deallocate memory
    free(list);

    return EXIT_SUCCESS;
};
