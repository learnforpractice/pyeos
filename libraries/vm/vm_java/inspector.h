#ifndef print_stack_h
#define print_stack_h

void logMessage(const char* message, ...)
{
    va_list arglist;
    va_start(arglist,message);
    vfprintf(stdout, message, arglist);
    fprintf(stdout, "\n");
    va_end(arglist);
    fflush(stdout);
}

void logError(const char* message, ...)
{
    va_list arglist;
    va_start(arglist,message);
    vfprintf(stderr, message, arglist);
    va_end(arglist);
    fprintf(stderr, "\n");
    fflush(stderr);
}

#endif /* print_stack_h */

