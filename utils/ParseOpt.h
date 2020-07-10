#ifndef __PARSEOPT_H_
#define __PARSEOPT_H_

#include <stdarg.h> 
#include <string>
#include <time.h>
#include <direct.h> 
#include <io.h>

// --------------------------------------
// static members initialization
// --------------------------------------
static const char* help_str[] = {
    "-path      set the aedat file path   \n",
    "-loop      loop or not(default:0, or 1)\n",
    "-help  print usage information\n",
    0
};

typedef enum {
    ARG_UNSUPPORT = -1,
    ARG_PATH = 0,
    ARG_LOOP,
    ARG_HELP,
    ARG_MAX
}ARG_TYPE;

struct option {
    char optstr[16];
    bool bhasargument;
    ARG_TYPE  argidx;
};

static struct option lopts[] = {
    { "-path",		true,	ARG_PATH },
    { "-loop",      true,  ARG_LOOP },
    { "-help",		false,	ARG_HELP},
    {0, 0, 0}
};

// --------------------------------------
// function declaration
// --------------------------------------
static void print_usage()
{
    int i = 0;

    fputs("=====Usage======\n", stdout);
    for (i = 0; help_str[i]; i++)
        fputs(help_str[i], stdout);
}

/*
param:
    opt_index   [io] argv参数的位置

return:
    argument index value.
*/

static ARG_TYPE getopt_long(int argc, char* argv[], struct option* opttbl, int& opt_index)
{
    int opttblcnt = 0;
    struct option* optitem;

    if (opt_index == -1)
    {   //start from one
        opt_index = 1;
    }

    for (opttblcnt = 0; opttbl[opttblcnt].optstr && (opt_index < argc); opttblcnt++)
    {
        optitem = &opttbl[opttblcnt];
        if (strcmp(optitem->optstr, argv[opt_index]) == 0)
        {
            if (optitem->bhasargument) opt_index += 2;    //say argument is only one
            else opt_index++;

            return optitem->argidx;
        }
    }

    return ARG_UNSUPPORT;
}

int make_output_folder(char* dirname)
{
    int flag = -1;

    if (_access(dirname, 0) == -1)
    {
        printf("%s is not existing, now make it\n", dirname);
        flag = _mkdir(dirname);
    }

    return flag;
}

#endif //__PARSEOPT_H_
