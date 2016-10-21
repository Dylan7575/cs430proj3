#include "parse.h"
#include "raycaster.c"

int main(int c, char** argv) {
    //checking if height is empty
    if(!isnumber(argv[1])==0||argv[1]==NULL){
        fprintf(stderr,"width passed in is not a number");
        exit(1);
    }
    else if(!isnumber(argv[2])==0||argv[2]==NULL){
        fprintf(stderr,"height passed in is not a number");
        exit(1);
    }
    int jsonlen=strlen(argv[3]);
    char * last4 = &argv[3][jsonlen-4];

    if(strcmp("json",last4)!=0||argv[3]==NULL){
        fprintf(stderr,"Error: Not a Json file");
        exit(1);
    }
    int len = strlen(argv[4]);
    char* str = &argv[4][len-3];
    if(strcmp(str,"ppm")!=0||argv[4]==NULL){
        fprintf(stderr,"output file is not a ppm File");
    }
    Object objects[129];
    Light lights[129];
    read_scene(argv[3],objects,lights);
    //printf("%f ",objects[2].plane.diffuse[1]);
    raycast(objects,lights,argv[1],argv[2],argv[4]);
    return 0;
}
