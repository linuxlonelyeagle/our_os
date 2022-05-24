#include <stdio.h>

struct a{
    int a;
    int b;
    char c;
};

int main(int argc,char* argv[]) 
{
    printf("%d\n",&((struct a*)0)->a);
    printf("%d\n",&((struct a*)0)->b);
    printf("%d\n",&((struct a*)0)->c);
    return 0;
}

