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
#include "shutdown.h"
#include "library.h"
#include "librarian.h"
#include "borrower.h"


#define DEFAULT_BORROWERS_QTY     20
#define DEFAULT_BORROWERS_DELAY 5000
#define DEFAULT_LIBRARIAN_DELAY 5000


#define SAFE_CREATE_THREAD(t,a,f,d)                                    \
	{                                                                  \
        int error = pthread_create(t,a,f,d);                           \
        if (error != 0)                                                \
        {                                                              \
            printf("ERROR: pthread_create() failed, code %i\n",error); \
            exit(1);                                                   \
        }                                                              \
    }

void* userinput_run (void* x)
{
    // wait for command to exit...
    char inputBuffer[256];
    while (!shutdown)
    {
    	printf ("Enter command ('q' to quit): ");
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

typedef struct 
{
	int timeLimit;
	int brwrQty;
	int brwrDelay;
	int lbrnDelay;
} config_t;

void display_help()
{
	printf("\n");
	printf("Command Line Arguments for 'sim':\n");
	printf("\n");
	printf("'-?''    Display this help page.\n");
	printf("'-t  N'  Set the time limit for the simulation in seconds (e.g. '-t 30')\n");
	printf("'-bq N'  Set the borrower quantity (e.g. '-bq 30')\n");
	printf("'-bd N'  Set the borrower delay in milliseconds (e.g. '-bd 1000')\n");
	printf("'-ld N'  Set the librarian delay in milliseconds (e.g. '-ld 5000')\n");
	printf("\n");
}

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
			printf("ERROR: No value given after '%s'\n", param); \
		}                                                        \
	}

int process_cmdline(int argc, char *argv[], config_t* config)
{
	config->timeLimit    = 0; // 0 is infinite
	config->brwrQty      = DEFAULT_BORROWERS_QTY;
	config->brwrDelay    = DEFAULT_BORROWERS_DELAY;
	config->lbrnDelay    = DEFAULT_LIBRARIAN_DELAY;

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
		ELSEIF_CHECK_INT_PARAM("-bq", config->brwrQty,   "Number of borrowers")
		ELSEIF_CHECK_INT_PARAM("-bd", config->brwrDelay, "Delay for borrowers")
		ELSEIF_CHECK_INT_PARAM("-ld", config->lbrnDelay, "Delay for librarian")
		ELSEIF_CHECK_INT_PARAM("-t",  config->timeLimit, "Time limit for simulation")
		else
		{
			printf("Unrecognised command line value!\n");
		}
	}
	if (!success) display_help();
	return success;
}

main_simulation (config_t* config)
{
	srand(time(NULL));

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
	// Read command line arguments...
	// TODO
	printf("\n\n");
	printf("===============================================================================\n");
	printf("CTEC2901: Library Simulator                                   (Barnaby Stewart)\n");
	printf("===============================================================================\n");

	config_t config;
	if (process_cmdline(argc, argv, &config) != 0)
	{
		main_simulation(&config);
	}

	printf("-------------------------------------------------------------------------------\n");
	printf("Program Complete\n");
	printf("-------------------------------------------------------------------------------\n");
	return 0;
}
