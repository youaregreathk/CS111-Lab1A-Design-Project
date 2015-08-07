/**************************************/
/* UCLA CS 111 Lab 1 command reading  */
/*  Contributors:                     */
/*  Kwai Hung Shea UID :304497354     */
/*  FnuPramono     UID :604498984     */
/**************************************/

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <string.h>

#define STACK_SIZE 50
#define DEBUG 0

#define INIT 0
#define AND 1
#define SEMICOLON 2
#define OR 3
#define PIPE 4
#define CMD 5
#define LEFT_SUBSHELL 6
#define RIGHT_SUBSHELL 7
#define LEFT_REDIR 8
#define RIGHT_REDIR 9
#define NEWLINE 10
#define APPEND 11       // >>
#define INPUT2 12        // <&
#define OUTPUT2 13       // >&
#define OPEN 14         // <>
#define OUTPUT_C 15     // >|

//Lab 1a
/****************************************************************************************
 In this lab1 lab 1a we need to implement make_command_stream, which should create and
 initialize a command_stream_t instance. We will also have to implement the
 read_command_stream. We use a stack data structure to help us storing the command we read
 and then pop it and store it into the tree.
 We also use the linked-list data structure to link the different tree togeather.
 
 *****************************************************************************************/

//Lab 1b
//****************************************************************************************




//*********************************************

typedef struct Ccommand_node
{
    command_t command;
    struct command_node *prev;
    struct command_node *next;
    
} command_node;

//linked list for the command tree
//*********************************************************
typedef struct Ccommand_stream
{
    int read;
    struct command_node *head;
    struct command_node *tail;
    struct command_node *cursor; // initialize to head
} command_stream;




//Stack Implementation
//*****************Stack Begin***********************************
typedef struct stack
{
    command_t command;
    struct stack* prev;   
}* myCommandStack;


command_t peek(myCommandStack* stack)
{
    if (stack == NULL || *stack == NULL)
        return NULL;
    return (*stack)->command;
}

void push(myCommandStack* stack, command_t command)
{
    myCommandStack temp = (myCommandStack) checked_malloc(sizeof(struct stack));
    temp->command = command;
    temp->prev = *stack;
    *stack = temp;
}

void pop(myCommandStack* stack)
{
    if (stack != NULL && (*stack) != NULL)
    	*stack = (*stack)->prev;
}


//*********************************************************************
//Second Stack
typedef struct stack2
{
    int operator;
    struct stack2* prev;
}* myOperatorStack;

void pop2(myOperatorStack* stack)
{
    if (stack != NULL && (*stack) != NULL)
        *stack = (*stack)->prev;
}

int peek2(myOperatorStack* st)
{
    if ( *st == NULL || st == NULL )
        return -1;
    return (*st)->operator;
}

void Push_st2(myOperatorStack* stack, int oper)
{
    myOperatorStack temp = (myOperatorStack) checked_malloc(sizeof(struct stack2));
    temp->operator = oper;
    temp->prev = *stack;
    *stack = temp;
}





//*************************************************************************
//Tokenizer

typedef struct token_Node {    // This linked list holds tokens
    int type;
    char *string;
    struct token_Node *next;
} token_Node;


typedef struct token_stream {
    token_Node *head;           // This container that holds token lists
    token_Node *tail;
    struct token_stream *next;
    int size;
} token_stream;


void insert_token(token_stream* stream, token_Node token)
{
    token_Node* temp = (token_Node*) checked_malloc(sizeof(token_Node));
    temp->string = token.string;
    temp->type = token.type;
            //This funtion inserst a token in the end of the token stream
    temp->next = token.next;
    if (stream->head == NULL){
        stream->head = temp;
        stream->tail = temp;
    }
    else
    { 
        stream->tail->next = temp;
        stream->tail = temp;
    }

    stream->size++;
    return;
}


token_stream* tokenizer(char* input)
{
    char c;
    token_stream* stream = checked_malloc(sizeof(token_stream));
       stream->tail = NULL;
    
    stream->next = NULL;
    stream->head = NULL;

    //It Creates a token_stream from input.
    stream->size = 0;

    token_stream* origin = checked_malloc(sizeof(token_stream));
    origin = stream;
    token_Node temptoken;
    
    temptoken.string = NULL;
    temptoken.next = NULL;
temptoken.type = INIT;
    size_t i=0;
    for (; i < strlen(input); i++)
    {
        c = input[i];
        switch(c)
        {
            case '\n':
            {
                if (temptoken.type != INIT)
                    insert_token(stream, temptoken);
                
                stream->next = checked_malloc(sizeof(token_stream));
                stream = (token_stream*) (stream->next);
                char* str = checked_malloc(sizeof(char));
                str[0] = '\0';
                temptoken.string = str;
                temptoken.type = NEWLINE;
                break;
            }
            case '&':
            {
                if (temptoken.type != INIT)
                    insert_token(stream, temptoken);
                if (input[i+1] == '&')
                {
                    char* str = checked_malloc(3*sizeof(char));
                    str[0] = c;
                    str[1] = c;
                    str[2] = '\0';
                    temptoken.string = str;
                    temptoken.type = AND;
                    i++;
                    break;
                }
            }
            case '#': break;
            case '@':
            {
                if (temptoken.type != INIT)
                    insert_token(stream, temptoken);

                char* str = checked_malloc(2* sizeof(char));
                str[0] = '\n';
                str[1] = '\0';
                temptoken.string = str;
                temptoken.type = CMD;

                break;
            }
            
            case ';':
            {
                if (temptoken.type != INIT)      
                    insert_token(stream, temptoken);

                char* str = checked_malloc(sizeof(char));
                str[0] = c;
                str[1] = '\0';
                temptoken.string = str;
                temptoken.type = SEMICOLON;
                break;
            }
            case '|':
            {
                if (temptoken.type != INIT)
                    insert_token(stream, temptoken);

                if (input[i+1] == '|')
                {
                    char* str = checked_malloc(3*sizeof(char));
                    str[0] = c;
                    str[1] = c;
                    str[2] = '\0';
                    temptoken.string = str;
                    temptoken.type = OR;
                    i++;
                    break;
                }

                else if (input[i+1] != '|')
                {
                    char* str = checked_malloc(2*sizeof(char));
                    str[0] = c;
                    str[1] = '\0';
                    temptoken.string = str;
                    temptoken.type = PIPE;
                    break;
                }
            }
            case '>':
            {
                if (temptoken.type != INIT)
                    insert_token(stream, temptoken);
                
                if (input[i+1] == '>')
                {
                    char* str = checked_malloc(3*sizeof(char));
                    str[0] = c;
                    str[1] = c;
                    str[2] = '\0';
                    temptoken.string = str;
                    temptoken.type = APPEND;
                    i++;
                    break;
                }
                else if (input[i+1] == '&')
                {
                    char* str = checked_malloc(3*sizeof(char));
                    str[0] = c;
                    str[1] = '&';
                    str[2] = '\0';
                    temptoken.string = str;
                    temptoken.type = OUTPUT2;
                    i++;
                    break;
                }
                else if (input[i+1] == '|')
                {
                    char* str = checked_malloc(3*sizeof(char));
                    str[0] = c;
                    str[1] = '|';
                    str[2] = '\0';
                    temptoken.string = str;
                    temptoken.type = OUTPUT_C;
                    i++;
                    break;
                }
                else
                {
                    char* str = checked_malloc(2*sizeof(char));
                    str[0] = c;
                    str[1] = '\0';
                    temptoken.string = str;
                    temptoken.type = RIGHT_REDIR;
                    break;
                }
            }

            case '<':
            {
                if (temptoken.type != INIT)
                    insert_token(stream, temptoken);

                if (input[i+1] == '&')
                {
                    char* str = checked_malloc(3*sizeof(char));
                    str[0] = c;
                    str[1] = '&';
                    str[2] = '\0';
                    temptoken.string = str;
                    temptoken.type = INPUT2;
                    i++;
                    break;
                }
                else if (input[i+1] == '>')
                {
                    char* str = checked_malloc(3*sizeof(char));
                    str[0] = c;
                    str[1] = '>';
                    str[2] = '\0';
                    temptoken.string = str;
                    temptoken.type = OPEN;
                    i++;
                    break;
                }
                else
                {
                    char* str = checked_malloc(2*sizeof(char));
                    str[0] = c;
                    str[1] = '\0';
                    temptoken.string = str;
                    temptoken.type = LEFT_REDIR;
                    break;
                }
            }

           
            case '(':
            {
                if (temptoken.type != INIT)
                    insert_token(stream, temptoken);

                char* str = checked_malloc(2*sizeof(char));
                str[0] = c;
                str[1] = '\0';
                temptoken.string = str;
                temptoken.type = LEFT_SUBSHELL;
                break;
            }

            // subshell end
            case ')':
            {
                if (temptoken.type != INIT)
                    insert_token(stream, temptoken);

                    char* str = checked_malloc(2*sizeof(char));
                    str[0] = c;
                    str[1] = '\0';
                    temptoken.string = str;
                    temptoken.type = RIGHT_SUBSHELL;
                    break;
            }
            case ' ':
            {
          
                if (temptoken.type != CMD)
                    break;                // if the space is not inside a command, we don't care
                else
                {
                    if (temptoken.type != INIT)
                    insert_token(stream, temptoken);
                    char* str = checked_malloc(sizeof(char));
                    str[0] = '\0';
                    temptoken.string = str;
                    temptoken.type = CMD;     
                    break; 
                }
            }
            default:
            {
                if (temptoken.type != CMD)
                {
                    if (temptoken.type != INIT)
                        insert_token(stream, temptoken);

                    // Save the new character
                    char* str = checked_malloc(sizeof(char));
                    str[0] = '\0';
                    temptoken.string = str;
                    temptoken.type = CMD;
                }

                
                size_t length = strlen(temptoken.string);
                temptoken.string = checked_realloc(temptoken.string, (length+1)*sizeof(token_Node));
                temptoken.string[length] = c;
                temptoken.string[length+1] = '\0';
                break;                                   // Add character to end of string

            }

        }

    }
 //For the remain tokens
    if (temptoken.type != INIT)
    	insert_token(stream, temptoken);

    
    stream->next = checked_malloc(sizeof(token_stream));
    return origin;
}


//***********************************************************
//The Parser

int Predence(int oper)
{                                                                 //This function Returns the numeric rank of an operator.
    
    int rank[5] = {0, 0, 1, 1, 2};
    int operators[5] = {SEMICOLON, NEWLINE, AND, OR, PIPE};
  
    
    int x=0;
    int pos = -1;
    for (; x < 5; x++)
    {
        if (oper == operators[x])
        {
            pos = x;
            break;
        }
    }
    if (pos == -1)
        return -1;

    return rank[pos];
}



command_t combineCommand(command_t first, command_t second, int operator)
{
    switch(operator)
    {
        case OR:
        {
                //This function combines two commands together and returns a new command based on operator.
            command_t newCommand = (command_t) checked_malloc(sizeof(struct command));
            newCommand->type = OR_COMMAND;
            newCommand->status = -1;
            newCommand->input = NULL;
            newCommand->output = NULL;
            newCommand->u.command[0] = first;
            newCommand->u.command[1] = second;
            return newCommand;
            break;
        }
        case SEMICOLON:
	    case NEWLINE:
	    {
		    command_t newCommand = (command_t) checked_malloc(sizeof(struct command));
		    newCommand->type = SEQUENCE_COMMAND;
		    newCommand->status = -1;
		    newCommand->input = NULL;
		    newCommand->output = NULL;
		    newCommand->u.command[0] = first;
		    newCommand->u.command[1] = second;
		    return newCommand;
		    break;
	    }
        case PIPE:
        {
            command_t newCommand = (command_t) checked_malloc(sizeof(struct command));
            newCommand->type = PIPE_COMMAND;
            newCommand->status = -1;
            newCommand->input = NULL;
            newCommand->output = NULL;
            newCommand->u.command[0] = first;
            newCommand->u.command[1] = second;
            return newCommand;
            break;
        }
        case LEFT_REDIR:
        {
            first->input = second->u.word[0];
            return first;
            
        }
            
	    case AND:
	    {
		    command_t newCommand = (command_t) checked_malloc(sizeof(struct command));
		    newCommand->type = AND_COMMAND;
		    newCommand->status = -1;
		    newCommand->input = NULL;
		    newCommand->output = NULL;
		    newCommand->u.command[0] = first;
		    newCommand->u.command[1] = second;
		    return newCommand;
		    break;
	    }
	    
	    
	    
	    case RIGHT_REDIR:
        {
	    	first->output = second->u.word[0];
	    	return first;
	    }
	    default: 
		    return NULL;
		    break;
    }
}

command_t parser(token_stream* stream)
{
    if (stream->size == 0)
        return NULL;
    token_Node* iter = stream->head;

    myCommandStack command_stack = NULL;
    myOperatorStack operator_stack = NULL;
    size_t wordpos = 0;
    size_t word_length = 0;

    bool CMD_FLAG = false;

    while (iter != NULL)
    {
	    if ( iter->type == NEWLINE || iter->type == INIT )
	    {
	        iter = iter->next;
	        continue;
	    }
	    else if (iter->type == CMD)
	    {
	        if (CMD_FLAG == false)
	        {
	            command_t simple = (command_t) checked_malloc (sizeof(struct command));
	            simple->type = SIMPLE_COMMAND;
	            simple->status = -1;
	              simple->output_c = NULL;
	            simple->output = NULL;
                simple->append = NULL;
                simple->input2 = NULL;
                simple->output2 = NULL;
                simple->input = NULL;
                simple->open = NULL;
	            simple->u.word = (char**) checked_malloc(2*sizeof(char*));
	            simple->u.word[wordpos] = iter->string;
	            simple->u.word[++wordpos] = NULL;
	            word_length += 2;
                simple->word_size = word_length-1; 
	            CMD_FLAG = true;
	            push(&command_stack, simple);
	            iter = iter->next;
	           
	            continue;
	        }
	        else
	        {
	            command_t simple = peek(&command_stack);
	            pop(&command_stack);
                simple->u.word = (char**) checked_realloc(simple->u.word, (word_length+1)*sizeof(char*));
	            simple->u.word[wordpos] = iter->string;
	            simple->u.word[++wordpos] = NULL;
	            word_length++;
                simple->word_size++; 
	            push(&command_stack, simple);
	            iter = iter->next;
                continue;
	        }
	    }
	  
	    else if (iter->type == LEFT_SUBSHELL)
	    {
	        wordpos = 0;                            //LeftSubshrll
	        word_length = 0;
	        Push_st2(&operator_stack, iter->type);
	        iter = iter->next;
	        CMD_FLAG = false;
	        continue;
	    }
	    else if (iter->type == RIGHT_SUBSHELL)
	    {
	        wordpos = 0;
	        word_length = 0;
	        int top_operator = peek2(&operator_stack);
	        pop2(&operator_stack);
	        while (top_operator != LEFT_SUBSHELL)
	        {
	            command_t second_command = peek(&command_stack);
	            pop(&command_stack);
	            command_t first_command = peek(&command_stack);
	            pop(&command_stack);
	            command_t new_command = combineCommand(first_command, second_command, top_operator);
	            push(&command_stack, new_command);
	            top_operator = peek2(&operator_stack);
	            pop2(&operator_stack);
	        }

	        command_t subshell = (command_t) checked_malloc (sizeof(struct command));
	        subshell->type = SUBSHELL_COMMAND;
	        subshell->status = -1;
            subshell->open = NULL;
	        subshell->input = NULL;
            subshell->output_c = NULL;
	        subshell->output = NULL;
            subshell->input2 = NULL;
            subshell->output2 = NULL;
	        command_t top_command = peek(&command_stack);
	        subshell->u.subshell_command = top_command;
	        pop(&command_stack);
        push(&command_stack, subshell);
	        iter = iter->next;
	        CMD_FLAG = false;
	        continue;
	    }
        else if (iter->type == APPEND)
        {
            wordpos = 0;
            word_length = 0;
            iter = iter->next;
            char* next_token = iter->string;
            command_t top_command = peek(&command_stack);
            pop(&command_stack);
            top_command->append = next_token;
            push(&command_stack, top_command);
            iter = iter->next;
            CMD_FLAG = false;
            continue;
        }
	    else if (iter->type == LEFT_REDIR)
	    {                                                 // Redirection
	        wordpos = 0;
	        word_length = 0;
	        iter = iter->next;
	        char* next_token = iter->string;
	        command_t top_command = peek(&command_stack);
	        pop(&command_stack);
	        top_command->input = next_token;
	        push(&command_stack, top_command);
	        iter = iter->next;
	        CMD_FLAG = false;
	        continue;
	    }
        else if (iter->type == OUTPUT2)
        {
            wordpos = 0;
            word_length = 0;
            iter = iter->next;
            char* next_token = iter->string;
            command_t top_command = peek(&command_stack);
            pop(&command_stack);
            top_command->output2 = next_token;
            push(&command_stack, top_command);
            iter = iter->next;
            CMD_FLAG = false;
            continue;
        }

	    else if (iter->type == RIGHT_REDIR)
	    {
	        wordpos = 0;
	        word_length = 0;
	        iter = iter->next;
	        char* next_token = iter->string;
	        command_t top_command = peek(&command_stack);
	        pop(&command_stack);
	        top_command->output = next_token;
	        push(&command_stack, top_command);
	        iter = iter->next;
	        CMD_FLAG = false;
	        continue;
	    }
        
        else if (iter->type == INPUT2)
        {
            wordpos = 0;
            word_length = 0;
            iter = iter->next;
            char* next_token = iter->string;
            command_t top_command = peek(&command_stack);
            pop(&command_stack);
            top_command->input2 = next_token;
            push(&command_stack, top_command);
            iter = iter->next;
            CMD_FLAG = false;
            continue;
        }
                else if (iter->type == OPEN)
        {
            wordpos = 0;
            word_length = 0;
            iter = iter->next;
            char* next_token = iter->string;
            command_t top_command = peek(&command_stack);
            pop(&command_stack);
            top_command->open = next_token;
            push(&command_stack, top_command);
            iter = iter->next;
            CMD_FLAG = false;
            continue;
        }
        else if (iter->type == OUTPUT_C)
        {
            wordpos = 0;
            word_length = 0;
            iter = iter->next;
            char* next_token = iter->string;
            command_t top_command = peek(&command_stack);
            pop(&command_stack);
            top_command->output_c = next_token;
            push(&command_stack, top_command);
            iter = iter->next;
            CMD_FLAG = false;
            continue;
        }
	    else if (iter->type == SEMICOLON||iter->type == AND || iter->type == OR  || iter->type == PIPE)
	    {
	        wordpos = 0;
	        word_length = 0;                                   // Operators
	        if (operator_stack == NULL)
	        {
		        Push_st2(&operator_stack, iter->type);
		        iter = iter->next;
		        CMD_FLAG = false;
		        continue;
	        }

	        else
	        {
		        int top_operator = peek2(&operator_stack);
		        if (Predence(iter->type) > Predence(top_operator))
		        {
			        Push_st2(&operator_stack, iter->type);
			        iter = iter->next;
			        CMD_FLAG = false;
			        continue;
	        	}

		        else
		        {
			        while(top_operator != LEFT_SUBSHELL && (Predence(iter->type) <= Predence(top_operator)))
			        {
				        int operator = peek2(&operator_stack);
				        pop2(&operator_stack);
				        command_t second_command = peek(&command_stack);
				        pop(&command_stack);
				        command_t first_command = peek(&command_stack);
				        pop(&command_stack);
				        command_t new_command = combineCommand(first_command, second_command, operator);
				        push(&command_stack, new_command);
				        top_operator = peek2(&operator_stack);
				        if (top_operator == -1)
				            break;
			        }
			        Push_st2(&operator_stack, iter->type);
			        iter = iter->next;
			        CMD_FLAG = false;
			        continue;
			    }
		    }
	    }

	    else
	    {
	        wordpos = 0;
	        word_length = 0;
	        iter = iter->next;
	        CMD_FLAG = false;
	        continue;
	    }
    }

    while (operator_stack != NULL)
    {                                                     //Some remain tokens
	    int operator = peek2(&operator_stack);
	    pop2(&operator_stack);
	    command_t second_command = peek(&command_stack);
	    pop(&command_stack);
	    command_t first_command = peek(&command_stack);
	    pop(&command_stack);
	    command_t new_command = combineCommand(first_command, second_command, operator);
	    push(&command_stack, new_command);
    }

    command_t root = peek(&command_stack);
    if (root == NULL)
        return NULL;
    else
    {
        pop(&command_stack);

        return root;
    }

}


//**************************************************************************************************************

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
         void *get_next_byte_argument)
{
    char current;
    bool COMMENT_FLAG = false;
    char* buffer = checked_malloc(sizeof(char));
    size_t unpair = 0;
    bool QUOTE_FLAG = false;    // Flags that we are in quotes
    char last = '\0';
    char last_nospace = '\0';   // tracks last previous nonspace character
    char last_last_nospace = '\0';    // tracks the last previous nonspace char before last_nospace
    bool AND_FLAG = false;
    bool OR_FLAG = false;
    int num_lines = 1;

    bool REDIR_FLAG = false; 
    
    bool BEGINNING_FLAG = true; 
    size_t allocSize = 0;
    bool LINE_FLAG = true;      // Flags beginning of line
    bool SUBSHELL_FLAG = false;     // Flags that we are in a subshell
    
    while ((current = get_next_byte(get_next_byte_argument)) != EOF)
    {
        if (!COMMENT_FLAG)
        {
            if (current == '\n')
                num_lines++;

            if ((BEGINNING_FLAG == true) && (((current == '|') || (current == '<') || (current == '&') || (current == '>')) && (last == '\0')))
            {
                fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                exit(1);
            }

            if (BEGINNING_FLAG == true && current == ' ')
                continue;

            BEGINNING_FLAG = false; 


            if (current == '(')
            {
                SUBSHELL_FLAG = true;
                unpair++;
            }
            if (current == ')')
            {
                SUBSHELL_FLAG = false;
                unpair--;


                if (last == ' ')
                {
                    size_t length = strlen(buffer);
                    buffer[length-1] = '\0';
                }
                
            }

            if(current == '\n' && (last_nospace == '\0') && COMMENT_FLAG == false)
            {
                continue;
            }
            if ((current == ' ' || current == '\r') && (last_nospace == '\0') && COMMENT_FLAG == false)
            {
                continue;
            }
            if (current == ';' && (last == ';' || last_nospace == '\n' || last == '\0'))
            {
                fprintf(stderr, "%d: Invalid semicolon\n", num_lines);
                exit(1);
            }
            if (current == '|' && (last_nospace == '\n' || last == '\0'))
            {
                fprintf(stderr, "%d: Invalid pipe\n", num_lines);
                exit(1);
            }

            if ((current == '>' || current == '<') && (last_nospace == '\n' || last == '\0'))
            {
                fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                exit(1);
            }
            if ((current == '<' && last_nospace == '<' && last_last_nospace == '<') || (current == '>' && last_nospace == '>' && last_last_nospace == '>'))
            {
                fprintf(stderr, "%d: Invalid redirection\n", num_lines);
                exit(1);
            }

            if (current == '&')
            {
                if (last_nospace == '<' && last_last_nospace == '<')
                {
                    fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                    exit(1);
                }
                if (last_nospace == '&' && last_last_nospace == '<')
                {
                    fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                    exit(1);
                }
                if (last_nospace == '>' && last_last_nospace == '>')
                {
                    fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                    exit(1);
                }
                if (last_nospace == '|' && last_last_nospace == '>')
                {
                    fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                    exit(1);
                }
                if (last_nospace == '&' && last_last_nospace == '>')
                {
                    fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                    exit(1);
                }
            }

            if (current == '|')
            {
                if (last_nospace == '<' && last_last_nospace == '<')
                {
                    fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                    exit(1);
                }
                if (last_nospace == '&' && last_last_nospace == '<')
                {
                    fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                    exit(1);
                }
                if (last_nospace == '>' && last_last_nospace == '>')
                {
                    fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                    exit(1);
                }
                if (last_nospace == '|' && last_last_nospace == '>')
                {
                    fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                    exit(1);
                }
                if (last_nospace == '&' && last_last_nospace == '>')
                {
                    fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                    exit(1);
                }
            }


            if (current == ';')
            {
                ;
            }

            if (current == ';' && last == ' ')
            {
                size_t length = strlen(buffer);
                buffer[length-1] = '\0';
            }

            if (current == '`')
            {
                fprintf(stderr, "%d: Invalid syntax\n", num_lines);
                exit(1);
            }

            if (AND_FLAG == true && current == '&')
            {
                fprintf(stderr, "%d: Invalid &\n", num_lines);
                exit(1);
            }
            if (OR_FLAG == true && current == '|')
            {
                fprintf(stderr, "%d: Invalid |\n", num_lines);
                exit(1); 
            }
            if (REDIR_FLAG == true && current == '>')
            {
                fprintf(stderr, "%d: Invalid >\n", num_lines);
                exit(1); 
            }

            
            if (current == '&' && last == '&' && AND_FLAG == false)
                AND_FLAG = true;    // Set the flag to be true if && or ||
            else
                AND_FLAG = false;

            if (current == '|' && last == '|' && OR_FLAG == false)
                OR_FLAG = true;
            else
                OR_FLAG = false;

          
            if (current == '>' && last == '>' && REDIR_FLAG == false)
                REDIR_FLAG = true;      // Set the flag to be true if >>
            else
                REDIR_FLAG = false; 

            
            if ((current == '>' || current == '<' || current == '|' || current == '&') && last == ' ')
            {
                size_t length = strlen(buffer);    // Get rid of spaces between special tokens
                buffer[length-1] = '\0';
            }

            if ((current == ' ') && (last == '>' || last == '<' || last == '|' || last == '&')) //last_nospace == '('))
                continue;

            
            if ((current == '#') && (last_nospace == '>' || last_nospace == '<' || last_nospace == '|' || last_nospace == '&'))
            {
                fprintf(stderr, "%d: Invalid comment\n", num_lines);
                exit(1);     // Comment after special token
            }

            if ((last == ' ' && current == ' ') || current == '\t' || (LINE_FLAG == true && current == ' '))
                continue;

            if (current == '#')
            {
                COMMENT_FLAG = true;
                continue;
            }

            if (current == '\n' && COMMENT_FLAG == true)
            {
                COMMENT_FLAG = false;
                continue;
            }

            if (current == '\n' && (last_nospace == '|' || last_nospace == '&'))
            {
                LINE_FLAG = true;
                continue;
            }

            if (current == '\n' && (last_nospace == '>' || last_nospace == '<'))
            {
                num_lines--; 
                fprintf(stderr, "%d: Invalid redirection\n", num_lines);
                exit(1); 
            }

            if (current == '\n' && last == '\n')
            {
        	    LINE_FLAG = true;
        	    continue;
            }

            if (current == '\n' && SUBSHELL_FLAG == true)
            {
            	continue;
            }

            if (current == ')' && SUBSHELL_FLAG == true)
            {
        	    allocSize++;
        	    buffer = checked_realloc(buffer, (2+allocSize)*sizeof(char));
        	    size_t length = strlen(buffer);
        	    buffer[length] = current; 
        	    buffer[length+1] = '\0'; 
        	    SUBSHELL_FLAG = false;
        	    continue;
            }


            if (current == '"' && QUOTE_FLAG == false)
            {
                QUOTE_FLAG = true;

                continue;
            }

            if (current == '\n' && QUOTE_FLAG == true)
            {
                allocSize++;
                buffer = checked_realloc(buffer, (2+allocSize)*sizeof(char));
                size_t length = strlen(buffer);
                buffer[length] = '@'; 
                buffer[length+1] = '\0'; 
                continue;
            }

            if (current == '"' && QUOTE_FLAG == true)
            {
                QUOTE_FLAG = false;
                continue;
            }

            if(!COMMENT_FLAG)
            {
                if (current != '\n')
                    LINE_FLAG = false;
                else
                    LINE_FLAG = true;

                allocSize++;
                buffer = checked_realloc(buffer, (2+allocSize)*sizeof(char));
                size_t length = strlen(buffer);
                buffer[length] = current;
                buffer[length+1] = '\0';
                last = current;
                if (current != ' ')
                {
        	        if (last_nospace != '\0')
        	            last_last_nospace = last_nospace;

        	        last_nospace = current;
                }
            }
        }
        else
        {
            if (current == '\n' && COMMENT_FLAG == true)
            {
                COMMENT_FLAG = false;
                continue;
            }
            else
                continue;
        }
    }


    if (last == '|' || last == '&' || last == '>' || last == '<')
    {
        fprintf(stderr, "%d: Invalid syntax\n", num_lines);
        exit(1); 
    }

    if (last == ')' && unpair != 0)
    {
	    fprintf(stderr, "%d: Unpaired parantheses\n", num_lines);
	    exit(1);
    }
    if (unpair != 0)
    {
	    fprintf(stderr, "%d: Unpaired parantheses\n", num_lines);
	    exit(1); 
    }


    token_stream* stream = tokenizer(buffer);

    command_stream_t cmd_stream = checked_malloc(sizeof(struct command_stream));
    cmd_stream->size = 0;
    cmd_stream->iterator = 0;
    cmd_stream->commands = checked_malloc(sizeof(struct command));
    int pos = 0;

    while (stream != NULL)
    {
        
        command_t newcommand = parser(stream);

        
        if (newcommand == NULL)
            break;     //The last extra token we have at the end
        else
        {
            

            cmd_stream->commands[pos] = newcommand;
            cmd_stream->size++;
            cmd_stream->commands = checked_realloc(cmd_stream->commands, (cmd_stream->size)*sizeof(struct command));                    pos++;
        }
        
        token_stream* toDelete = stream;
        stream = (token_stream*)stream->next;
        free(toDelete);
    }

        free(buffer);
    return cmd_stream;

}

command_t
read_command_stream (command_stream_t s)
{
    if (s->iterator >= s->size)
        return NULL;
    return s->commands[(s->iterator++)];
}

