
/*
	Mneminic:	fifo.c
	Abstract: 	Unit test for the fifo module.
				Tests obvious things, may miss edge cases.
	Date:		04 February 2016
	Author:		E. Scott Daniels
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "gadgetlib.h"


/*
	Loops reading the fifo.
*/
static void loop_test( char* fname ) {
	char	*rbuf;			// read buffer
	void*	fifo;			// fifo 'handle'

	if( fname == NULL ) {
		fprintf( stderr, "argv2 was nil: requires loop pipe-name\n" );
		exit( 1 );
	}

	fifo = rfifo_create( fname, 0664 );
	if( fifo == NULL ) {
		fprintf( stderr, "[FAIL] unable to create fifo: %s\n", strerror( errno ) );
		exit( 1 );
	}
	fprintf( stderr, "[OK]  fifo created successfully, entering read loop.\n" );
	
	while( 1 ) {
		rbuf = rfifo_read( fifo );
		if( rbuf != NULL ) {
			if( strlen( rbuf ) > 0 ) {
				fprintf( stderr, "read got %d bytes in the buffer: %s\n", (int) strlen( rbuf ), rbuf );
			}
		} 

		sleep( 1 );
	}
}

int main( int argc, char** argv ) {
	char*	fname = "test_pipe";
	int	rc = 0;
	int i;
	void*	fifo;			// we'll create the fifo with the library stuff and then...
	int		fd;				// we will open it again and write something, then call read
	char*	wbuf = "Mary didn't really have a lamb, she had a dog.\nIt was brown, lazy, and liked to play down by the river.\n\n";	// we should get both records back, each with a trailing newline
	char*	rbuf;			// read buffer


	if( argv[1] != NULL ) {
		if( strcmp( argv[1], "loop" ) == 0 ) {
			if( argv[2] != NULL ) {
				fname = argv[2];
			}

			loop_test( fname );
			exit( 0 );
		}

		fname = argv[1];
	}

	fifo = rfifo_create( fname, 0664 );
	if( fifo == NULL ) {
		fprintf( stderr, "[FAIL] unable to create fifo: %s\n", strerror( errno ) );
		exit( 1 );
	}
	fprintf( stderr, "[OK]  fifo created successfully\n" );

	fd = open( fname, O_WRONLY, 0 );			// we'll read from the fifo too


	fprintf( stderr, "reading before write to test nonblocking...\n" );
	rbuf = rfifo_read( fifo );
	if( rbuf != NULL ) {
		fprintf( stderr, "nb read got %d bytes in the buffer\n", (int) strlen( rbuf ) );
	} else {
		fprintf( stderr, "nb read got null pointer back\n" );
	}


	if( fd >= 0 ) {
		if( write( fd, wbuf, strlen( wbuf ) ) < strlen( wbuf ) ) {
			fprintf( stderr, "write to fifo failed\n" );
			exit( 1 );
		}
	} else {
		fprintf( stderr, "open for write failed\n" );
		exit( 1 );
	}

	fprintf( stderr, "reading...\n" );
	rbuf = rfifo_read( fifo );
	if( rbuf != NULL ) {
		fprintf( stderr, "read from fifo was successful: %ld bytes\n", (long) strlen( rbuf ) );
		fprintf( stderr, "(%s)\n", rbuf );
	} else {
		fprintf( stderr, "read from pipe failed\n" );
	}

	rfifo_close( fifo );
}
