// prototype for apple driver
int unsafe_function(__constant int *buffer, int offset);

int unsafe_function(__constant int *buffer, int offset)
{
    return buffer[offset];
}
