/******************************************************
 *Lab 1b                                               *
 * Contributors:                                       *
 * Kwai Hung Shea    UID: 304497354                    *
 * Fun Pramono       UID: 604498984                    *
 *******************************************************/


#include "command.h"
#include "command-internals.h"
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define DEBUG 0


int command_status (command_t c)
{
    return c->status;
}


//   Set up file descriptors duplicating std_in or std_out as appropriate for a given command c

void SetFileDes(command_t tmp)
{
    if (tmp->output) {    //  output
        int result= open(tmp->output, O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (result < 0) {
            return;
        }
        dup2(result, 1);
        
        close(result);
    }
    
    if (tmp->input) {     //input
        int result2 = open(tmp->input, O_RDONLY, 0644);
        if (result2 < 0) {
            return;
        }
        dup2(result2, 0);
        close(result2);
    }
    
    
    
}

void simpleExe(command_t tmp)
{
    pid_t pp_id = fork();
    if (pp_id > 0) {          // parent process
        int status;
        waitpid(pp_id, &status, 0);
        tmp->status = WEXITSTATUS(status);
    }
    else if (pp_id == 0) {                // child process
        SetFileDes(tmp);
        execvp(tmp->u.word[0], tmp->u.word);
        error(1, 0, "Invalid simple command\n");
    }
    else {
        // something went wrong with fork()
        error(1, 0, "Error forking process\n");
    }
}


void orExecute(command_t tmp1, bool time_travel)
{
    execute_command(tmp1->u.command[0], time_travel);
    if (tmp1->u.command[0]->status != 0) {
        execute_command(tmp1->u.command[1], time_travel);
        
        tmp1->status = tmp1->u.command[1]->status;
    } else {
        tmp1->status = tmp1->u.command[0]->status;
    }
}

void andExecute(command_t tmp2, bool time_travel)
{
    execute_command(tmp2->u.command[0], time_travel);
    if (tmp2->u.command[0]->status == 0) {
        execute_command(tmp2->u.command[1], time_travel);
        tmp2->status = tmp2->u.command[1]->status;
    }
    else {
        tmp2->status = tmp2->u.command[0]->status;
    }
}


void sequenceExecute(command_t tmp, bool time_travel)
{
    execute_command(tmp->u.command[0], time_travel);
    if(tmp->u.command[0]->status == 0) {
        execute_command(tmp->u.command[1],time_travel);
        tmp->status = tmp->u.command[1]->status;
    }
    else {
        tmp->status = tmp->u.command[0]->status;
    }
}

void subshellexecute(command_t tmp, bool time_travel)
{
    if (tmp->output && tmp->u.subshell_command->output == NULL) {
        tmp->u.subshell_command->output = tmp->output;
    }
    
    if (tmp->input && tmp->u.subshell_command->input == NULL) {
        tmp->u.subshell_command->input = tmp->input;
    }
    
    execute_command(tmp->u.subshell_command, time_travel);
    
    tmp->status = tmp->u.subshell_command->status;
}

void pipeExecute(command_t tmp, bool time_travel)
{
    int descriptorarray[2];
    
    pipe(descriptorarray);
    
    pid_t first_pid = fork();
    
    if (first_pid > 0) {
        int status;
        waitpid(-1, &status, 0);
        status = WEXITSTATUS(status);
        if (status != 0) {
            
            error(1, 0, "Invalid pipe command\n");
        }
        pid_t second_pid = fork();
        if (second_pid == 0) {
            close(descriptorarray[1]);   // close unused write end
            dup2(descriptorarray[0],0);
            execute_command(tmp->u.command[1], time_travel);
            close(descriptorarray[0]);
            _exit(tmp->u.command[1]->status);
        } else if (second_pid > 0) {
            
            close(descriptorarray[0]);    // close unnused pipe
            close(descriptorarray[1]);
            waitpid(-1, &status, 0);
            tmp->status = WEXITSTATUS(status);
        }
    }
    else if(first_pid == 0) {
        close(descriptorarray[0]);   // close unused read end
        dup2(descriptorarray[1], 1);
        execute_command(tmp->u.command[0], time_travel);
        close(descriptorarray[1]);
        _exit(tmp->u.command[0]->status);
    }
}

void
execute_command (command_t tmp, bool time_travel)
{
    char cmd_type = ' ';
    switch(tmp->type) {
        case OR_COMMAND:
            orExecute(tmp, time_travel);
            cmd_type = 'o';
            break;
            
        case SIMPLE_COMMAND:
            simpleExe(tmp);
            cmd_type = 's';
            break;
        case SUBSHELL_COMMAND:
            subshellexecute(tmp, time_travel);
            cmd_type = '(';
            break;
        case SEQUENCE_COMMAND:
            sequenceExecute(tmp, time_travel);
            cmd_type = ';';
            break;
        case AND_COMMAND:
            andExecute(tmp, time_travel);
            cmd_type = 'a';
            break;
            
            
        case PIPE_COMMAND:
            pipeExecute(tmp, time_travel);
            cmd_type = '|';
            break;
        default:
            return;
    }
    if (DEBUG) {
        printf("%c cmd - exit status: %d\n", cmd_type, tmp->status);
    }
}