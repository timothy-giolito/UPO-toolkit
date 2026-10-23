/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file apps/postfix_eval.c
 *
 * \brief An application to evaluate postfix mathematical expressions.
 *
 * \author Your Name
 *
 * \copyright 2015 University of Piemonte Orientale, Computer Science Institute
 *
 * This file is part of UPOalglib.
 *
 * UPOalglib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * UPOalglib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with UPOalglib.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include "postfix_eval.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <upo/stack.h>


#define MAX_LINE_LEN 256

/*TO STUDENTS: Complete the body of eval_postfix:
    1. The type of operands and operations result is long int
    2. Parameter expr is a string containing the postfix expression
    3. Valid (binary) operators: +; -; *; =; %; ^
    4. Numbers and operators are separated by spaces
    5. Only nonnegative integer numbers: 0, 19, . . .
  
    Hint: use isdigit() to check if a character is a digit, and
          sscanf() to read numbers from a string as long int (format "%ld")
    
    6. Parameter res is the result of the evaluation of the postfix expression
    7. Returns 1 if expr is well-formed, 0 otherwise
*/

int eval_postfix(const char *expr, long *res)
{
    upo_stack_t stack = NULL;
    const char *p = expr;

    if (expr == NULL || res == NULL)
    {
        return 0;
    }

    stack = upo_stack_create(sizeof(long));
    if (stack == NULL)
    {
        return 0;
    }

    while (*p != '\0')
    {
        /* Salta gli spazi e i caratteri di spaziatura */
        if (isspace((unsigned char)*p))
        {
            p++;
            continue;
        }

        /* Caso 1: È un numero intero non negativo */
        if (isdigit((unsigned char)*p))
        {
            long num = 0;
            int bytes_read = 0;

            if (sscanf(p, "%ld%n", &num, &bytes_read) == 1)
            {
                upo_stack_push(stack, &num);
                p += bytes_read;
            }
            else
            {
                upo_stack_destroy(stack, 0);
                return 0;
            }
        }
        /* Caso 2: È un operatore valido (+, -, *, /, %, ^) */
        else if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '%' || *p == '^')
        {
            char op = *p;
            long op1 = 0, op2 = 0, op_res = 0;
            long *top_ptr = NULL;

            /* L'operatore richiede esattamente 2 operandi nello stack */
            if (upo_stack_size(stack) < 2)
            {
                upo_stack_destroy(stack, 0);
                return 0;
            }

            /* Estrae prima il secondo operando, poi il primo */
            top_ptr = (long *)upo_stack_top(stack);
            op2 = *top_ptr;
            upo_stack_pop(stack, 0);

            top_ptr = (long *)upo_stack_top(stack);
            op1 = *top_ptr;
            upo_stack_pop(stack, 0);

            switch (op)
            {
                case '+':
                    op_res = op1 + op2;
                    break;
                case '-':
                    op_res = op1 - op2;
                    break;
                case '*':
                    op_res = op1 * op2;
                    break;
                case '/':
                    if (op2 == 0)
                    {
                        /* Divisione per zero non permessa */
                        upo_stack_destroy(stack, 0);
                        return 0;
                    }
                    op_res = op1 / op2;
                    break;
                case '%':
                    if (op2 == 0)
                    {
                        /* Modulo per zero non permesso */
                        upo_stack_destroy(stack, 0);
                        return 0;
                    }
                    op_res = op1 % op2;
                    break;
                case '^':
                    if (op2 < 0)
                    {
                        /* Esponente negativo non supportato per interi */
                        upo_stack_destroy(stack, 0);
                        return 0;
                    }
                    op_res = (long) pow((double)op1, (double)op2);
                    break;
                default:
                    upo_stack_destroy(stack, 0);
                    return 0;
            }

            upo_stack_push(stack, &op_res);
            p++;
        }
        /* Caso 3: Carattere non riconosciuto */
        else
        {
            upo_stack_destroy(stack, 0);
            return 0;
        }
    }

    /* Alla fine dell'espressione deve rimanere esattamente un valore nello stack */
    if (upo_stack_size(stack) == 1)
    {
        long *top_ptr = (long *)upo_stack_top(stack);
        *res = *top_ptr;
        upo_stack_pop(stack, 0);
        upo_stack_destroy(stack, 0);
        return 1;
    }

    upo_stack_destroy(stack, 0);
    return 0;
}

void eval_lines(FILE *fp)
{
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp))
    {
        size_t slen = strlen(line);
        int ok = 0;
        long res = 0;
        char *res_str = NULL;
        long expect_res = 0;

        /* Strips the terminating newline (if any) */
        if (slen > 0 && line[slen-1] == '\n')
        {
            line[slen-1] = '\0';
            --slen;
        }

        /* Checks if we have the expected result. In this case the input line should be: <expr>, <result> */
        res_str = strstr(line, "=>");
        if (res_str != NULL)
        {
            line[res_str-line] = '\0';
            res_str += 2;
            expect_res = atol(res_str);
        }

        ok = eval_postfix(line, &res);
        if (ok)
        {
            if (res_str != NULL)
            {
                printf("Expression '%s' -> %ld (expected: %ld -> %s)\n", line, res, expect_res, (res == expect_res) ? "OK" : "KO");
            }
            else
            {
                printf("Expression '%s' -> %ld\n", line, res);
            }
        }
        else
        {
            printf("Expression '%s' -> Malformed\n", line);
        }
    }
}

void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s -f [<filename>]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "-f <filename>: The full path name to the file containing strings (one for each line).\n"
                    "               If not given, strings are read from standard input.\n"
                    "               [default: standard input]\n");
    fprintf(stderr, "-h: Displays this message\n");
    fprintf(stderr, "-v: Enables output verbosity\n");
}


int main(int argc, char *argv[])
{
    char *opt_filename = NULL;
    int opt_help = 0;
    int opt_verbose = 0;
    int arg = 0;
    FILE *fp = NULL;

    for (arg = 1; arg < argc; ++arg)
    {
        if (!strcmp("-f", argv[arg]))
        {
            ++arg;
            if (arg >= argc)
            {
                fprintf(stderr, "ERROR: expected file name.\n");
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            opt_filename = argv[arg];
        }
        else if (!strcmp("-h", argv[arg]))
        {
            opt_help = 1;
        }
        else if (!strcmp("-v", argv[arg]))
        {
            opt_verbose = 1;
        }
    }

    if (opt_help)
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (opt_verbose)
    {
        printf("-- Options:\n");
        printf("* File name: %s\n", opt_filename ? opt_filename : "<not given>");
    }

 
    if (opt_filename != NULL)
    {
        fp = fopen(opt_filename, "r");
        if (fp == NULL)
        {
            perror("ERROR: cannot open input file");
            return EXIT_FAILURE;
        }
    }
    else
    {
        fp = stdin;
    }

    if (opt_verbose)
    {
        printf("-- Evaluating...\n");
    }

    eval_lines(fp);

    if (opt_filename != NULL)
    {
        fclose(fp);
    }

    return EXIT_SUCCESS;
}