// Author: Andrea Tongsak
// Class: CS344 OPERATING SYSTEMS I (CS_344_001_F2021)
// Date: October 17, 2021
/* Exploration: Data Types, Modifiers, Qualifers & Conversion */
/* Exploration: Directories */
/* Exploration: Permissions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define PREFIX "movies_"

/**
 * Movie information stored in struct
 */
struct movie {
    char *title;
    int year;
    char *languages;
    double rating_value;
    struct movie *next;
};

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
struct movie *processFile(char *filePath) {
    // Open the specified file for reading only
    FILE *movieFile = fopen(filePath, "r");

    char *currLine = NULL;
    size_t len = 0;
    ssize_t nread;

    // The head of the linked list
    struct movie *head = NULL;
    // The tail of the linked list
    struct movie *tail = NULL;

    // Read the file line by line
    while ((nread = getline(&currLine, &len, movieFile)) != -1)
    {
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
    free(currLine);
    fclose(movieFile);
    return head;
}

/**
 * Helper: creates a directory with a random number generated at the end
 */
char createDirectory(char* dirname) {
    // generates random number and seed
    srand(time(NULL));
    int random = (rand() % (999999-0+1));
    sprintf(dirname, "tongsaka.movies.%d", random);
    // The permission of the directory must be set to rwxr-x--- (u=rwx, g=r-x, o=---) or 0750
    mkdir(dirname, 0750);
    return *dirname;
}

/**
 * Helper: creates a file for every year in which at least one movie was released
 */
void createFile(struct movie* list, char* filename) {
    int file;
    // go through each movie, and set permissions
    while (list != NULL) {
        // creates YYYY.txt
        sprintf(filename, "%d.txt", list->year);
        
        if (strcmp(filename, "0.txt") != 0) {
            // open the file name for writing only (0_WRONLY)
            //      | create an empty file if not there (O_CREAT)
            //      | append new content to the end of file (0_APPEND)
            // the permission on these files must be rw-r----- (u=rw-, g=r--, o=---) or 0640
            file = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0640);
            // inserts the movie title into file
            write(file, list->title, strlen(list->title));
            write(file, "\n", strlen("\n"));
            close(file);
        }
        list = list->next;
    }
}

/**
 *  Process Movie .csv File
 *  Algorithm
 *  0. Look for the largest file in the directory
 *  1. Create directory with name onid.movies.random
 *  2. The permission of the directory must be set to rwxr-x--- (u=rwx, g=r-x, o=---)
 *  3. Parse data in the chosen file to find out the movies released in each year
 *  4. In the new directory, create one file for each year in which at least one movie was released
 *     The permission on these files must be rw-r----- (u=rw-, g=r--, o=---)
 *     The file must be named YYYY.txt where YYYY is the 4 digit integer value of the year.
 *  5. Within the file for a year, write the titles of all the movies released in that year, one for each line
 *     E.g., if two movies Avengers: Infinity War and Mary Queen of Scots where released in 2018, then
 *          the file 2018.txt will have two lines with each of the two titles on one line each.
 */
void processMovieFile(int choice) {
    DIR* currentDir = opendir(".");
    struct dirent *aDir;
    struct stat dirStat;
    off_t currSize = 0;
    int i = 0;
    char userChoice[256];
    char *ptr;
    int ch = '.';
    bool found = false;
    char dir_name[21];
    char *filePath = calloc(256, sizeof(char));
    
    // handle the cases when user choice is to process the largest .csv file or the smallest .csv file
    if (choice == 1 || choice == 2) {
        // Go through all the entries and compare movie file sizes
        while ((aDir = readdir(currentDir)) != NULL) {
            // points to the end of the directory name
            ptr = strrchr(aDir->d_name, ch);
            if (ptr != NULL) {
                // IMPORTANT: we are comparing the file and ensuring there is a csv extension
                if (strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0 && strcmp(".csv", ptr) == 0) {
                    found = true;
                    // get metadata for the current entry
                    stat(aDir->d_name, &dirStat);
                    
                    // locate the largest movies .csv file
                    if (choice == 1) {
                        // IMPORTANT line: the directory size is smaller than the current size
                        if (i == 0 || dirStat.st_size > currSize) {
                            currSize = dirStat.st_size;
                            memset(userChoice, '\0', sizeof(userChoice));
                            strcpy(userChoice, aDir->d_name);
                        }
                    // locate the smallest movies .csv file
                    } else if (choice == 2) {
                        // IMPORTANT line: the directory size is larger than the current size
                        if (i == 0 || dirStat.st_size < currSize) {
                            currSize = dirStat.st_size;
                            memset(userChoice, '\0', sizeof(userChoice));
                            strcpy(userChoice, aDir->d_name);
                        }
                    }
                    i++;
                }
            }
        }
        // close current directory and free directory entries
        closedir(currentDir);
        free(aDir);
    }
    
    // handle the cases when user choice is to process a specified .csv file
    if (choice == 3) {
        // create the directory from the filePath
        printf("Enter the complete file name: ");
        scanf("%s", filePath);
        printf("\n");
      
        while ((aDir = readdir(currentDir)) != NULL) {
            if (strcmp(aDir->d_name, filePath) == 0) {
                found = true;
            }
        }
        closedir(currentDir);
        free(aDir);
    }
    
    // we now process the file
    if (found) {
        if (choice == 1 || choice == 2) {
            printf("Now processing the chosen file named %s\n", userChoice);
        } else if (choice == 3) {
            printf("Now processing the chosen file named %s\n", filePath);
        }
        createDirectory(dir_name);
        printf("Created a directory with file %s\n", dir_name);
        // the movie name is processed and stored in list
        if (choice == 1 || choice == 2) {
            struct movie *list = processFile(userChoice);
            char filename[9];
            currentDir = opendir(".");
            
            // Go through all the entry
            while ((aDir = readdir(currentDir)) != NULL) {
                
                if (strcmp(dir_name, aDir->d_name), strlen(dir_name)) {
                    // enter the new directory
                    chdir(dir_name);
                    break;
                }
            }
            // create the file within the directory with XXX.txt
            createFile(list, filename);
            // move out of the directory and exit
            chdir("..");
            closedir(currentDir);
            
        } else if (choice == 3) {
            struct movie *list = processFile(filePath);
            char filename[25];
            currentDir = opendir(".");
            
            // the directory is still existing
            while ((aDir = readdir(currentDir)) != NULL) {
                
                if ((strncmp(dir_name, aDir->d_name, strlen(dir_name)) && strlen(dir_name)) == 0) {
                    // enter the directory
                    chdir(dir_name);
                    break;
                }
            }

            createFile(list, filename);
            // moves out of directory, exits
            chdir("..");
            closedir(currentDir);
            free(filePath);
        }
    } else {
        printf("The file %s was not found. Try again.\n\n", filePath);
    }
}

/**
 * Process the CSV file provided as an argument to create linked lists
 * Compile using:
 *  gcc --std=gnu99 -o students main.c
 */
int main () {
    
    int choice;
    int file2process;
    bool filenotselected = true;
    do {
        printf("1. Select file to process\n");
        printf("2. Exit the program\n\n");
        printf("Enter a choice 1 or 2: ");
        scanf("%d", &choice);

        if (choice < 1 || choice > 2) {
            printf("You entered an incorrect choice. Try again.\n");
        } else {
            // File processing option
            if (choice == 1) {
                do {
                    printf("Which file you want to process?\n");
                    printf("Enter 1 to pick the largest file\n");
                    printf("Enter 2 to pick the smallest file\n");
                    printf("Enter 3 to specify the name of a file\n\n");
                    printf("Enter a choice from 1 to 3: ");
                    scanf("%d", &file2process);
                    printf("\n");
                    
                    // Handle user input for choice of file
                    if (file2process < 1 || file2process > 3) {
                        printf("You entered an incorrect choice. Try again.\n");
                    } else {
                        filenotselected = false;
                        if (file2process == 1) {
                            printf("\nGoing to process the largest file\n\n");
                            processMovieFile(1);
                            
                        } else if (file2process == 2) {
                            printf("\nGoing to process the smallest file\n\n");
                            processMovieFile(2);
                            
                        } else if (file2process == 3) {
                            printf("\nGoing to process the file that has been specified\n\n");
                            processMovieFile(3);
                        }
                    }
                } while (filenotselected);
            }
        }
    } while (choice != 2);

    return EXIT_SUCCESS;
};
