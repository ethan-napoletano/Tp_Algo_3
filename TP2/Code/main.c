// Need this to use the getline C function on Linux. Works without this on MacOs. Not tested on Windows.
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "token.h"
#include "queue.h"
#include "stack.h"


/** 
 * Utilities function to print the token queues
 */
void print_token(const void* e, void* user_param);
void print_queue(FILE* f, Queue* q);

/** 
 * Function to be written by students
 */

bool isSymbol(char c){
	return c=='+' || c=='-' || c=='*' || c=='/' || c=='^' || c=='(' || c==')';
}

Queue* stringToTokenQueue(const char* expression){
	Queue* Queuefinal = create_queue();
	const char* curpos = expression;
	while(*curpos != '\0'){
		while(*curpos == ' ' || *curpos == '\n') {
			curpos++;
		}
		int nbValNum =0;
		if(isSymbol(*curpos)){
			nbValNum = 1;
		}	
		else{
			while (curpos[nbValNum] != '\0' && !isSymbol(curpos[nbValNum]) && curpos[nbValNum]!=' ' && curpos[nbValNum] != '\n'){
				nbValNum++;
			}
		}
		if(nbValNum > 0){
			Token* token = create_token_from_string(curpos, nbValNum);
			queue_push(Queuefinal, token);
			curpos += nbValNum;
		}
		else{
			curpos++;
		}
	}
	return Queuefinal; 
}



void CasOperateur(Stack* operators, Queue* postfix, Token* token) {
    while (!stack_empty(operators)) {
        Token* top = (Token*) stack_top(operators);
		
		if (!token_is_operator(top))
            break;
		
		if (token_is_parenthesis(top) && token_parenthesis(top) == '(')
            break;

        int prioTop = token_operator_priority(top);
        int prioCurrent = token_operator_priority(token);
		
		if (prioTop > prioCurrent || (prioTop == prioCurrent && token_operator_leftAssociative(token))) {
            queue_push(postfix, top);
            stack_pop(operators);
			
        }else {
            break; 
        }
    }

    
    stack_push(operators, token);
}


void CasParentheseDroite(Stack* operators, Queue* postfix, Token* token) {
    while (!stack_empty(operators) && (token_is_parenthesis(stack_top(operators)) ? token_parenthesis(stack_top(operators)) != '(': true)){
        queue_push(postfix, stack_top(operators));
        stack_pop(operators);
    }

    if (!stack_empty(operators)) {
        Token* tok = (Token*) stack_top(operators);
        stack_pop(operators);
        delete_token(&tok);
    }

    delete_token(&token);
}



Queue* shuntingYard(Queue* infix)
{
    Queue* postfix = create_queue();
    Stack* operators = create_stack(queue_size(infix));

    while (!queue_empty(infix)) {
        Token* token = (Token*) queue_top(infix);

        if (token_is_number(token)) {
            queue_push(postfix, token);
        }
        
        else if (token_is_operator(token)) {
            CasOperateur(operators, postfix, token);
        }
        
        else if (token_is_parenthesis(token) && token_parenthesis(token) == '(') {
            stack_push(operators, token);
        }
        
        else if (token_is_parenthesis(token) && token_parenthesis(token) == ')') {
            CasParentheseDroite(operators, postfix, token);
        }

        queue_pop(infix);
    }

    while (!stack_empty(operators)) {
        queue_push(postfix, stack_top(operators));
        stack_pop(operators);
    }

    delete_stack(&operators);
    return postfix;
}

float Puissance(float base, int exponent) {
    float result = 1.0f;
    for (int i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}

Token* evaluateOperator(Token* arg1, Token* op, Token* arg2)
{
    if (token_is_number(arg1) && token_is_operator(op) && token_is_number(arg2))
    {
        float v1 = token_value(arg1);
        float v2 = token_value(arg2);
        char operator = token_operator(op);
        float result = 0;

        switch (operator)
        {
            case '+':
                result = v1 + v2;
                break;

            case '-':
                result = v1 - v2;
                break;

            case '*':
                result = v1 * v2;
                break;

            case '/':
                if (v2 == 0.0f) {
                    fprintf(stderr, "division par zéro\n");
                    exit(1);
                }
                result = v1 / v2;
                break;

            case '^': {
                int exp = (int)v2; //en gros on transformme v2 qui etait un flaot en int dans la variable exp
                if (v2 != exp || exp < 0) {
                    fprintf(stderr, "exposant doit être un entier positif\n");
                    exit(1);
                }
                result = Puissance(v1, exp);
                break;
            }

            default:
                fprintf(stderr, "%c n'est pas un opérateur accepté\n", operator);
                delete_token(&arg1);
                delete_token(&arg2);
                delete_token(&op);
                exit(1);
        }

        delete_token(&arg1);
        delete_token(&arg2);
        delete_token(&op);
        return create_token_from_value(result);
    }
    else 
    {
        fprintf(stderr,"Erreur : un des arguments n'est pas du bon type\n");
        exit(2);
    }
}


float evaluateExpression(Queue* postfix) {
    Stack* stack = create_stack(queue_size(postfix));  

    while (!queue_empty(postfix)) {
        Token* token = (Token*) queue_top(postfix);

        if (token_is_number(token)) {
			stack_push(stack, token);
        }
        else if (token_is_operator(token)) {
            if (stack_size(stack) < 2) {
                fprintf(stderr, "Erreur : pas assez d'opérandes pour l'opérateur\n");
                exit(1);
            }
            Token* operand2 = (Token*) stack_top(stack);
            stack_pop(stack);
            Token* operand1 = (Token*) stack_top(stack);
            stack_pop(stack);
            Token* result = evaluateOperator(operand1, token, operand2);
            stack_push(stack, result);
        }
        else {
            fprintf(stderr, "Erreur \n");
            exit(1);
        }

        queue_pop(postfix);  
    }

    if (stack_size(stack) != 1) {
        fprintf(stderr, "Erreur stack!=1\n");
        exit(1);
    }

    Token* final_result = (Token*) stack_top(stack);
    float value = token_value(final_result);
	
    delete_token(&final_result);
    delete_stack(&stack);

    return value;
}



void computeExpressions(FILE* input) {
    char* ligne = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&ligne, &len, input)) != -1) {
        if (strlen(ligne) > 1) {
            printf("Input : %s", ligne);

            Queue* infix = stringToTokenQueue(ligne);
			
			printf("Infix : ");
            print_queue(stdout, infix);
            printf("\n");
			
            Queue* postfix = shuntingYard(infix);
            
            printf("Postfix : ");
            print_queue(stdout, postfix);
            printf("\n");

            float result = evaluateExpression(postfix);
            printf("Result : %.6f\n\n", result);

            delete_queue(&infix);
            delete_queue(&postfix);
        }
    }

    free(ligne);
}








/** Main function for testing.
 * The main function expects one parameter that is the file where expressions to translate are
 * to be read.
 *
 * This file must contain a valid expression on each line
 *
 */
int main(int argc, char** argv){
	if (argc<2) {
		fprintf(stderr,"usage : %s filename\n", argv[0]);
		return 1;
	}
	
	FILE* input = fopen(argv[1], "r");

	if ( !input ) {
		perror(argv[1]);
		return 1;
	}

	computeExpressions(input);

	fclose(input);
	return 0;
}
 
void print_token(const void* e, void* user_param) {
	FILE* f = (FILE*)user_param;
	Token* t = (Token*)e;
	token_dump(f, t);
}

void print_queue(FILE* f, Queue* q) {
	fprintf(f, "(%d) --  ", queue_size(q));
	queue_map(q, print_token, f);
}
