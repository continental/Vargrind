#include "pub_tool_libcbase.h"
#include "vr_regex.h"

void split_description(char* description, char** array,int* j){
    int i=0;
    *j=0;
    int b=0;
    int l=0;
    char buffer[256];
    VG_(memset)(buffer,0,256);
    do{
        if(description[i]==' ' || description[i]==',' || description[i]==34 ||description[i]=='\0'){
            if(description[i-1]==' ' || description[i-1]==',' || description[i-1]==34){

            }
            else if (j==0)
            {

            }
            else
            {
                buffer[b] = '\0';
                for(l=0;l<VG_(strlen)(buffer);l++){
                    array[*j][l]=buffer[l];
                }
                array[*j][l]='\0';
                (*j)++;
                b=0;
                VG_(memset)(buffer,0,256);
            }
        }
        else
        {
            buffer[b] = description[i];
            b++;
        }
        i++;
        
    }while(description[i-1]!='\0');
                             
}

