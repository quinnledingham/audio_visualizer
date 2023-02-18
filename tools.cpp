//
// strings
//

function b32
equal(const char *l, const char *r)
{
    u32 i = 0;
    while (l[i] != 0 && r[i] != 0)
    {
        if (l[i] != r[i])
            return false;
        i++;
    }
    
    if (r[i] != 0)
        return false;
    
    return true;
}

function u32
get_length(const char *in)
{
    int count = 0;
    while(in[count] != 0)
        count++;
    return count;
}

//
// ints
//

function u32
get_digits(u32 i)
{
    u32 count = 0;
    u32 t = i;
    while ((t = t / 10) != 0)
        count++;
    count++;
    return count;
}

function char*
u32_to_string(u32 in)
{
    char *out;
    
    u32 digits = get_digits(in);
    out = (char*)SDL_malloc(digits + 1);
    
    u32 help = 0;
    for (u32 i = 0; i < digits; i++)
    {
        if (help == 0)
            help++;
        else
            help *= 10;
    }
    
    for (u32 i = 0; i < digits; i++)
    {
        out[i] = ((char)(in / help) + '0');
        in = in - ((in / help) * help);
        help = help / 10;
    }
    
    out[digits] = 0;
    return out;
}