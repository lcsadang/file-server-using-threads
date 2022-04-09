#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

// PROTOTYPE FUNCTIONS
// misc functions
int randomizer(void);                           // returns the random amount of time the worker thread will sleep initially
int textupdate(char *path, char *buf);          // appends the command to the commands.txt
int getcmd(char *buf, int nbuf);                // gets the command from the user input

// command functions
void* writecmd(char *arg);                      // contains the process for the write command
void* readcmd(char *arg);                       // contains the process for the read command
void* emptycmd(char *arg);                      // contains the process for the empty command

int randomizer(){                               // returns the random amount of time the worker thread will sleep initially
	srand(time(0));
	int nap_time, r = rand()%100;           // returns a pseudo-random integer between 0 and 100

	if ((r % 5) == 0){                  // if the integer is divisible by 5 (there are 20 of these from 0 to 100), the thread sleeps for 6 seconds
		nap_time = 6;
	}
	else {
		nap_time = 1;                   // else the thread sleeps for 1 second
	}
	return nap_time;                        // returns the chosen time
}

int textupdate(char *path, char *buf){                  // appends the command to the commands.txt
        struct timeval current_time;                    // declares variables we need to access the current time
        time_t t;
        struct tm *full;

        gettimeofday(&current_time, NULL);              // gets the system's current time
        t = current_time.tv_sec;                        // gets the current time in seconds

        full = localtime(&t);                           // converts the time value we have to full date and time

        FILE *fp = fopen(path, "a");                    // opens our needed file (commands.txt)
        fprintf(fp, "%s %s", buf, asctime(full));       // appends the current command + current time
        fclose(fp);                                     // closes the currently open file
        return 0;
}

void* writecmd(char *arg){                                      // contains the process for the write command
        char* buf = arg;

	int i, nap = randomizer();                              // calls the randomizer to start initial sleep
	sleep(nap);

        char *path, *string;
        path = strchr(buf, ' ');                                // removes the command "write" from the input
        path++;
        string = strchr(path, ' ');                             // isolates the string
        string++;
        path = strtok(path, " ");                               // isolates the path

        int init_wt = open(path, O_RDWR | O_CREAT, 0666);       // if file does not exist, create one with rw permissions
        close(init_wt);                                      

        FILE *wt = fopen(path, "a");                            // opens the file with the given path

	for( i = 0; i < strlen(string); i++){                   // writes the characters to the file one by one and sleeps for 25ms after
		char char_to_write = string[i];
	        fwrite(&char_to_write, sizeof(char), 1, wt);
	        sleep(0.025);
	}

        fclose(wt);                                             // closes the open file

	free(arg);                                              // deallocates variables used

        return 0;
}

void* readcmd(char *arg){                                                       // contains the process for the read command
        char* buf = arg;

        int nap = randomizer();                                                 // calls the randomizer to start initial sleep
        sleep(nap);

        char *path = strchr(buf, ' ');                                          // isolates the path
	path++;

        FILE *to_read, *rt = fopen("read.txt", "a");                            // opens read.txt

        fprintf(rt, "%s : ", buf);                                              // appends the current command to read.txt

        if ((to_read = fopen(path, "r"))){                                      // opens the desired file, if it exists, do the following
                int size;
                fseek(to_read, 0, SEEK_END);                                    // moves the pointer to the end of the file
                size = ftell(to_read);                                          // takes the size of the file

		fclose(to_read);                                                // reopens the file so the pointer returns to the beginnning
                to_read = fopen(path, "r");

                char *rstring = (char*)malloc(size*sizeof(char) + 1);           // allocates space for the string to append

                fread(rstring, sizeof(char), size, to_read);                    // reads the contents of the file

                fprintf(rt, "%s\n", rstring);                                   // appends the contents of the file to read.txt

                fclose(rt);                                                     // closes the open files
                fclose(to_read);

		free(rstring);                                                  // deallocates variables used
                free(arg);

                return 0;
        }
        fprintf(rt, "FILE DNE\n");                                              // if the file does not exist, appends "FILE DNE" to read.txt
        fclose(rt);                                                             // closes read.txt

        free(arg);                                                              // deaallocates variables used

        return 0;
}

void* emptycmd(char *arg){                                                      // contains the process for the empty command
        char* buf = arg;

        int nap = randomizer();                                                 // calls the randomizer to start initial sleep
        sleep(nap);

        char *path = strchr(buf, ' ');                                          // isolates the path
        path++;

        FILE *to_empty, *et = fopen("empty.txt", "a");                          // opens empty.txt

	fprintf(et, "%s : ", buf);                                              // appends the current command to empty.txt

        if ((to_empty = fopen(path, "r"))){                                     // opens the desired file, if it exists, do the following
                int size;
                fseek(to_empty, 0, SEEK_END);                                   // moves the pointer to the end of the file
                size = ftell(to_empty);                                         // takes the size of the file

		fclose(to_empty);                                               // reopens the file so the pointer returns to the beginnning
                to_empty = fopen(path, "r");

                char *estring = (char*)malloc(size*sizeof(char) + 1);           // allocates space for the string to append

                fread(estring, sizeof(char), size, to_empty);                   // reads the contents of the file

                fprintf(et, "%s\n", estring);                                   // appends the contents of the file to empty.txt

                fclose(et);                                                     // closes empty.txt

                freopen(path, "w", to_empty);                                   // reopens the desired file with write permissions to clear it

		nap = (rand()%3) + 7;                                           // randomizes the duration of the process' sleep from 7 to 10
		sleep(nap);

                fclose(to_empty);                                               // closes the open files

		free(estring);                                                  // deaallocates variables used
                free(arg);

                return 0;
        }

        fprintf(et, "FILE DNE\n");                                              // if the file does not exist, appends "FILE DNE" to empty.txt
        fclose(et);                                                             // closes empty.txt

	free(arg);                                                              // deaallocates variables used

        return 0;
}

int getcmd(char *buf, int nbuf)                 // gets the command from the user input
{
        printf("$ ");
        memset(buf, 0, nbuf);                   // initializes buf 
        fgets(buf, nbuf, stdin);                // gets the input from stdin
        return 0;
}

int main(int argc, char *argv[]){
        char buf[110];

        while(getcmd(buf, sizeof(buf)) >= 0){
                if(buf[0] == 'w' && buf[1] == 'r' && buf[2] == 'i' && buf[3] == 't' && buf[4] == 'e' && buf[5] == ' '){         // checks if it is the write command
			buf[strlen(buf)-1] = 0;                                                                                 // chops \n
                        textupdate("commands.txt", buf);                                                                        // adds the command to commands.txt

                        char *arg = (char*)malloc(strlen(buf)*sizeof(char) + 1);                                                // allocates space for a variable specifically for this thread
                        strcpy(arg, buf);                                                                                       // copies the input to this variable

			pthread_t write_thread;                                                                                 // declares the thread

			pthread_create(&write_thread, NULL, (void*) writecmd, arg);                                             // dispatches worker thread

                        continue;
                }
                if(buf[0] == 'r' && buf[1] == 'e' && buf[2] == 'a' && buf[3] == 'd' && buf[4] == ' '){                          // checks if it is the read command
                        buf[strlen(buf)-1] = 0;                                                                                 // chops \n
                        textupdate("commands.txt", buf);                                                                        // adds the command to commands.txt

                        char *arg = (char*)malloc(strlen(buf)*sizeof(char) + 1);                                                // allocates space for a variable specifically for this thread
                        strcpy(arg, buf);                                                                                       // copies the input to this variable

			pthread_t read_thread;                                                                                  // declares the thread

                        pthread_create(&read_thread, NULL, (void*) readcmd, arg);                                               // dispatches worker thread

                        continue;
                }
                if(buf[0] == 'e' && buf[1] == 'm' && buf[2] == 'p' && buf[3] == 't' && buf[4] == 'y' && buf[5] == ' '){         // check if it is the empty command
                        buf[strlen(buf)-1] = 0;
                        textupdate("commands.txt", buf);                                                                        // adds the command to commands.txt

                        char *arg = (char*)malloc(strlen(buf)*sizeof(char) + 1);                                                // allocates space for a variable specifically for this thread
                        strcpy(arg, buf);                                                                                       // copies the input to this variable

			pthread_t empty_thread;                                                                                 // declares the thread

                        pthread_create(&empty_thread, NULL, (void*)emptycmd, arg);                                              // dispatches worker thread

                        continue;
                }
        }
}
