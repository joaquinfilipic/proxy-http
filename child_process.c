#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "cbuffer.h"

#ifndef CHILD_PROCESS
#include "child_process.h"
#include "settings.h"

#endif

int init_child(child_process *cp, settings *st) {
	int in [] = { -1, -1};
    int out[] = { -1, -1};
    int infd  = STDIN_FILENO;
    int outfd = STDOUT_FILENO;


    // Create the pipes.
    if (pipe(in) == -1 || pipe(out) == -1) {
        perror("creating pipes");
        return ERROR_CREATING_CHILD;
    }

    int retval1 = fcntl( in[W], F_SETFL, fcntl(in[W], F_GETFL) | O_NONBLOCK);
    int retval2 = fcntl( out[R], F_SETFL, fcntl(out[R], F_GETFL) | O_NONBLOCK);

    if (retval1 != 0 || retval2 != 0) {
        printf("Error creating non-blocking pipes\n");
        return ERROR_CREATING_CHILD;
    }

    printf("Created child.\n");
    const pid_t pid = fork();


    if (pid == -1) {
       	// Error f**king return -1.
    	return ERROR_CREATING_CHILD;
    } else if (pid == 0) {
        // Here we set up the child
        close(infd);
        close(outfd);
        close(in [W]);
        close(out[R]);
        in [W] = out[R] = -1;
        dup2(in [R], STDIN_FILENO);
        dup2(out[W], STDOUT_FILENO);

        if (execl("/bin/sh", "sh", "-c", st->cmd, NULL) == -1) {
        	// Error running shell script.
            close(in [R]);
            close(out[W]);
            return ERROR_CREATING_CHILD;
        }

        // Shouldn't get here.
        exit(1);
    } else {
    	// Here we continue to set up the parent.

        close(in [R]);
        close(out[W]);
        in[R] = out[W] = -1;

        cp->fds[2] = in[W];
        cp->fds[3] = out[R];
    }


    return SUCCESS;

}



void set_fd_for_child(child_process *cp, int *max_fd, fd_set *readfds, fd_set *writefds, struct buffer *write_to, struct buffer *read_from) {

	int h;
    for(h = 0 ; h < 4; h++) {
        if(cp->fds[h] > *max_fd) {
            *max_fd = cp->fds[h];
        }
    }

    if(cp->fds[CH_R] != -1 && buffer_can_write(write_to)) {
        FD_SET(cp->fds[CH_R], readfds);
    }
    if(cp->fds[CH_W] != -1 && buffer_can_read(read_from)) {
        FD_SET(cp->fds[CH_W], writefds);
    }

}

void resolve_child(child_process *cp, fd_set *readfds, fd_set *writefds, struct buffer *write_to, struct buffer *read_from) {

    // Reading from the child.
    if(FD_ISSET(cp->fds[CH_R], readfds)) {
        int r = doread(cp->fds + CH_R, write_to);
        if (r == 0){
            close(cp->fds[CH_R]);
            cp->fds[CH_R] = -1;
        }
    }

    // Writing to the child.
    if(FD_ISSET(cp->fds[CH_W], writefds)) {
        dowrite(cp->fds + CH_W, read_from);
    }
}


void free_child(child_process *cp) {
    if (cp == NULL)
        return;
    printf("Killed child.\n");
	close(cp->fds[CH_R]);
	close(cp->fds[CH_W]);
	free(cp);
}
