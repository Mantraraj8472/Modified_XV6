// User program to test get_year system call

#include "types.h"
#include "stat.h"
#include "user.h"

int main()
{
    // calling getyear() system call
    printf(1, "Unix was released in the year %d\n",getyear());
    exit();
}


