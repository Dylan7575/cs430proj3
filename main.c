#include "parse.c"
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
    printf("%f %f %f %f %f",lights[0].radA0,lights[0].radA1,lights[0].radA2,lights[0].angA0,lights[0].color[1]);
    //raycast(objects,argv[1],argv[2],argv[4],lights);
    return 0;
}
