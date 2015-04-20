//===========================================================================
// FILE: sim.c
//===========================================================================
// Implementation file for library simulation.
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
// This file handles the process of getting the simulation up and running
//---------------------------------------------------------------------------



// Standard C Headers
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// ds library Headers
#include "list.h"
#include "clist.h"
#include "queue_any.h"

// Project Headers
#include "smalloc.h"
#include "trace.h"
#include "terminal.h"
#include "shutdown.h"
#include "library.h"
#include "librarian.h"
#include "borrower.h"
#include "sim.h"


#define DEFAULT_BORROWERS_QTY        20
#define DEFAULT_BORROWERS_DELAY    1000
#define DEFAULT_BORROWERS_OFFSET    500
#define DEFAULT_BORROWER_RQSTSIZE     5
#define DEFAULT_LIBRARIAN_DELAY    5000
#define DEFAULT_LIBRARIAN_RQSTSIZE    5
#define DEFAULT_LIBRARY_BOOKRANGE    20
#define DEFAULT_LIBRARY_NUMBOOKS     60

// global (ugly but practical)
// with more time would be worth creating accessor functions
config_t config;


#define SAFE_CREATE_THREAD(t,a,f,d)                                    \
	{                                                                  \
        int error = pthread_create(t,a,f,d);                           \
        if (error != 0)                                                \
        {                                                              \
        	terminal_settextred(1);                                    \
            printf("ERROR: pthread_create() failed, code %i\n",error); \
            terminal_reset();                                          \
            exit(1);                                                   \
        }                                                              \
    }

void* userinput_run (void* x)
{
    // wait for command to exit...
    char inputBuffer[256];
    while (!shutdown)
    {
    	printf ("Enter command ('q' to quit):\n");
		fgets (inputBuffer, sizeof(inputBuffer), stdin);
		switch (inputBuffer[0])
		{
			case 'q':
			case 'Q':
				shutdown = 1;
				break;
		}
    }
	printf("\nPROGRAM TERMINATED BY USER\n");
}

void display_help()
{
	printf("\n\n");
	printf("Command Line Arguments for 'sim':\n");
	printf("\n");
	printf("'-?''    Display this help page.\n");
	printf("'-t  N'  Time limit for the simulation in seconds (e.g. '-t 30')\n");
	printf("'-bn N'  Number of borrowers (e.g. '-bn 30')\n");
	printf("'-bd N'  Borrower delay in milliseconds (e.g. '-bd 1000')\n");
	printf("'-bo N'  Random offset to the borrower delay (e.g. '-bo 500')\n");
	printf("'-br N'  Borrower request size (books to request per message))\n");
	printf("'-ld N'  Librarian delay in milliseconds (e.g. '-ld 5000')\n");
	printf("'-lr N'  Librarian request size (books/loans per message))\n");
	printf("'-rn N'  Range of book numbers (e.g. '20' gives book ids from 0 to 19)\n");
	printf("'-nb N'  Number of books to add to the library (random ids from the given range\n");
	printf("\n");
}

// Boilerplate code for handling parameters with int values
#define ELSEIF_CHECK_INT_PARAM(param,var,desc)\
	else if (strncmp(argv[n], param, strlen(param)) == 0)        \
	{                                                            \
		if (++n<argc)                                            \
		{                                                        \
			int scanint;                                         \
			sscanf(argv[n], "%d", &scanint);                     \
			printf("%s set to %d\n", desc, scanint);             \
			var = scanint;                                       \
		}                                                        \
		else                                                     \
		{                                                        \
			success = 0;                                         \
        	terminal_settextred(1);                              \
			printf("ERROR: No value given after '%s'\n", param); \
        	terminal_reset();                                    \
		}                                                        \
	}

// Process the command line
// Display help if requested or on any error
int process_cmdline(int argc, char *argv[], config_t* config)
{
	config->timeLimit     = 0; // 0 is infinite
	config->brwrQty       = DEFAULT_BORROWERS_QTY;
	config->brwrDelay     = DEFAULT_BORROWERS_DELAY;
	config->brwrOffset    = DEFAULT_BORROWERS_OFFSET;
	config->brwrRqstSize  = DEFAULT_BORROWER_RQSTSIZE;
	config->lbrnDelay     = DEFAULT_LIBRARIAN_DELAY;
	config->lbrnRqstSize  = DEFAULT_LIBRARIAN_RQSTSIZE;
	config->lbryBookRange = DEFAULT_LIBRARY_BOOKRANGE;
	config->lbryNumBooks  = DEFAULT_LIBRARY_NUMBOOKS;

	int n;
	int success = 1;
	for (n=1;n<argc;n++)
	{
		printf("CMD LINE[%d]: '%s', ", n, argv[n]);

		if (strncmp(argv[n], "-?", 2) == 0)
		{
			success = 0;
			break;
		}
		ELSEIF_CHECK_INT_PARAM("-t",  config->timeLimit,     "Time limit for simulation")
		ELSEIF_CHECK_INT_PARAM("-bn", config->brwrQty,       "Number of borrowers")
		ELSEIF_CHECK_INT_PARAM("-bd", config->brwrDelay,     "Delay for borrowers")
		ELSEIF_CHECK_INT_PARAM("-br", config->brwrRqstSize,  "Size of borrower requests")
		ELSEIF_CHECK_INT_PARAM("-bo", config->brwrOffset,    "Random delay offset for borrowers")
		ELSEIF_CHECK_INT_PARAM("-ld", config->lbrnDelay,     "Delay for librarian")
		ELSEIF_CHECK_INT_PARAM("-lr", config->lbrnRqstSize,  "Size of librarian requests")
		ELSEIF_CHECK_INT_PARAM("-rn", config->lbryBookRange, "Range of book numbers")
		ELSEIF_CHECK_INT_PARAM("-nb", config->lbryNumBooks,  "Number of books")
		else
		{
			success = 0;
        	terminal_settextred(1);
			printf("Unrecognised command line value!\n");
			terminal_reset();
		}
	}

	printf("-------------------------------------------------------------------------------\n");
	if (!success) display_help();
	return success;
}

main_simulation (config_t* config)
{
	assert(config != NULL);

	// Prepare thread attribute
	pthread_attr_t attr;
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);

	// Create thread handles...
	int i;
	int numThreads = 2 + config->brwrQty;
	SAFE_MALLOC_ARRAY(pthread_t, threads, numThreads);

	// Create the main threads...
	library_t* lib = library_create (config->brwrQty);
	SAFE_CREATE_THREAD(&threads[0], &attr, library_run,   (void*)lib);
	SAFE_CREATE_THREAD(&threads[1], &attr, librarian_run, (void*)lib);
    for (i=2; i<numThreads; i++)
    {
        SAFE_CREATE_THREAD(&threads[i], &attr, borrower_run, (void*)lib);
    }

	// And one to check for user input...
    pthread_t inputThread;
    SAFE_CREATE_THREAD(&inputThread, &attr, userinput_run, 0);

	// Wait for timeout or indefinitely...
	if (config->timeLimit > 0)
	{
		printf("Time limit set to %d", config->timeLimit);
		sleep_allowing_shutdown(config->timeLimit);
		if(!shutdown)
		{
			printf("\nPROGRAM TERMINATED BY TIMEOUT\n");
			pthread_cancel(inputThread);
			shutdown = 1;
		}
	}
	else while(!shutdown) sleep(1);

	// nudge the queue in case it's waiting...
	library_nudge(lib);
	for (i=0; i<numThreads; i++)
	{
		pthread_join(threads[i], NULL);
	}
	library_release(lib);

	pthread_attr_destroy(&attr);
}

main (int argc, char *argv[])
{
	printf("\n\n");
	printf("===============================================================================\n");
	printf("CTEC2901: Library Simulator                                   (Barnaby Stewart)\n");
	printf("===============================================================================\n");

	srand(time(NULL));
	if (process_cmdline(argc, argv, &config) != 0)
	{
		main_simulation(&config);
	}

	printf("-------------------------------------------------------------------------------\n");
	printf("Program Complete\n");
	printf("-------------------------------------------------------------------------------\n");
	printf("\n\n");
	return 0;
}
