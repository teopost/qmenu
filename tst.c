#include<stdio.h>
#include<stdlib.h>
#include<time.h>


time_t t1,t2,dt;
struct tm *tptr;
main(){

/*     (void)time(&t1);  */

     t1=time((time_t *)0);  

    tptr=localtime(&t1);
    
    printf("Local date is %d/%d/%d\n", 1900+tptr->tm_year,      1+tptr->tm_mon,tptr->tm_mday);
    
    printf("Local time is %d.%d.%d\n", tptr->tm_hour,             tptr->tm_min,tptr->tm_sec);
}
