#ifndef LOG_H
#define LOG_H

function void
log(const char* msg, va_list valist)
{
    const char *msg_ptr = msg;
    while (*msg_ptr != 0)
    {
        fputc(*msg_ptr, stderr);
        msg_ptr++;
    }
    
    fputc('\n', stderr);
}

function inline void
log(const char* msg, ...)
{
    va_list valist;
    va_start(valist, msg);
    log(msg, valist);
}

function void
error(const char* msg, ...)
{
    fprintf(stderr, "error: ");
    
    va_list valist;
    va_start(valist, msg);
    log(msg, valist);
}

#endif //LOG_H
