//
// Created by Dylan La Frenz on 9/27/16.
//
int line=1;
//timers tcp protocol
//rtt = estimated rtt + estimated volatility
//rtt = 1 - alpha + sample rtt alpha - .125
//offered load
#include "includes.h"
//creating a struct for sphere camera and plane
#include "parse.h"

int next_c(FILE* json) {
    int c = fgetc(json);
#ifdef DEBUG
    printf("next_c: '%c'\n", c);
#endif
    if (c == '\n') {
        line += 1;
    }
    if (c == EOF) {
        fprintf(stderr, "Error: Unexpected end of file on line number %d.\n", line);
        exit(1);
    }
    return c;
}


// expect_c() checks that the next character is d.  If it is not it emits
// an error.
void expect_c(FILE* json, int d) {
    int c = next_c(json);
    if (c == d) return;
    fprintf(stderr, "Error: Expected '%c' on line %d.\n", d, line);
    exit(1);
}


// skip_ws() skips white space in the file.
void skip_ws(FILE* json) {
    int c = next_c(json);
    while (isspace(c)) {
        c = next_c(json);
    }
    ungetc(c, json);
}


// next_string() gets the next string from the file handle and emits an error
// if a string can not be obtained.
char* next_string(FILE* json) {
    char buffer[129];
    int c = next_c(json);
    if (c != '"') {
        fprintf(stderr, "Error: Expected string on line %d.\n", line);
        exit(1);
    }
    c = next_c(json);
    int i = 0;
    while (c != '"') {
        if (i >= 128) {
            fprintf(stderr, "Error: Strings longer than 128 characters in length are not supported.\n");
            exit(1);
        }
        if (c == '\\') {
            fprintf(stderr, "Error: Strings with escape codes are not supported.\n");
            exit(1);
        }
        if (c < 32 || c > 126) {
            fprintf(stderr, "Error: Strings may contain only ascii characters.\n");
            exit(1);
        }
        buffer[i] = c;
        i += 1;
        c = next_c(json);
    }
    buffer[i] = 0;
    return strdup(buffer);
}

double next_number(FILE* json) {
    double value;
    int fail;
    fail =fscanf(json, "%lf", &value);
    if (fail==EOF||fail==0){
        fprintf(stderr,"Not a valid Number on Line Number%d",line);
        exit(1);

    }
    return value;
}

double* next_vector(FILE* json) {
    double* v = malloc(3*sizeof(double));
    expect_c(json, '[');
    skip_ws(json);
    v[0] = next_number(json);
    skip_ws(json);
    expect_c(json, ',');
    skip_ws(json);
    v[1] = next_number(json);
    skip_ws(json);
    expect_c(json, ',');
    skip_ws(json);
    v[2] = next_number(json);
    skip_ws(json);
    expect_c(json, ']');
    //printf("%f\n",v[0]);
    return v;
}



void read_scene(char* filename,Object* object,Light* lights) {
    int c,i=0,cameras=0,j=0;

    FILE* json = fopen(filename, "r");

    if (json == NULL) {
        fprintf(stderr, "Error: Could not open file \"%s\"\n", filename);
        exit(1);
    }

    skip_ws(json);

    // Find the beginning of the list
    expect_c(json, '[');

    skip_ws(json);

    // Find the objects

    while (1) {
        //creating temporary Objects to assign to Object array later
        Plane plane;
        Sphere sphere;
        Camera cam;
        char type;
        c = fgetc(json);
        if (c == ']') {
            fprintf(stderr, "Error: This is the worst scene file EVER.\n");
            fclose(json);
            exit(1);
        }
        if (c == '{') {
            skip_ws(json);

            // Parse the object
            char* key = next_string(json);
            if (strcmp(key, "type") != 0) {
                fprintf(stderr, "Error: Expected \"type\" key on line number %d.\n", line);
                exit(1);
            }

            skip_ws(json);

            expect_c(json, ':');

            skip_ws(json);

            char* value = next_string(json);

            //assigning kind values to temporary objects
            if (strcmp(value, "camera") == 0) {
                if (cameras==1){
                    fprintf(stderr,"Too many camera Objects");
                    exit(1);
                }
                object[i].kind=0;
                type ='o';
                cameras=1;
            } else if (strcmp(value, "sphere") == 0) {
                object[i].kind = 1;
                type ='o';
            } else if (strcmp(value, "plane") == 0) {
                object[i].kind = 2;
                type ='o';
            }
              else if(strcmp(value,"light")==0){
                type ='l';
            } else
                {
                fprintf(stderr, "Error: Unknown type, \"%s\", on line number %d.\n", value, line);
                exit(1);
            }

            skip_ws(json);

            while (1) {
                // , }
                c = next_c(json);
                if (c == '}') {
                    // stop parsing this object
                    break;

                } else if (c == ',') {
                    // read another field
                    skip_ws(json);
                    char* key = next_string(json);
                    skip_ws(json);
                    expect_c(json, ':');
                    skip_ws(json);

                    //checking which json value is next
                    if ((strcmp(key, "width") == 0) ||
                        (strcmp(key, "height") == 0) ||
                        (strcmp(key, "radius") == 0)) {
                        double value = next_number(json);
                        char* err;

                        //based upon which value is there put value in correct temporary value
                        if (strcmp(key, "width")==0 && object[i].kind==0) {
                            cam.width=value;
                             err = "width";
                        }
                        else if (strcmp(key, "height") == 0 && object[i].kind==0) {
                            cam.height=value;
                            err = "height";

                        } else if (strcmp(key, "radius") == 0&& object[i].kind==1) {
                            sphere.radius=value;
                            err = "radius";
                        }

                        else{
                            fprintf(stderr,"unexpected %s value on line %i",err,line);
                            exit(1);
                        }
                    }
                    //based upon which value is there put value in correct temporary value
                    else if ((strcmp(key, "specular_color") == 0) ||
                               (strcmp(key, "position") == 0) ||
                               (strcmp(key, "normal") == 0) ||
                               (strcmp(key,"diffuse_color")==0)) {

                        double* value = next_vector(json);

                        if (strcmp(key, "specular_color") == 0) {

                            if (object[i].kind == 1||object[i].kind==2) {

                                if (object[i].kind == 1) {
                                    sphere.specular[0] = value[0];
                                    sphere.specular[1] = value[1];
                                    sphere.specular[2] = value[2];
                                }

                                if (object[i].kind == 2) {
                                    plane.specular[0] = value[0];
                                    plane.specular[1] = value[1];
                                    plane.specular[2] = value[2];
                                }
                            }
                            else{

                                fprintf(stderr,"unexpected position value on line: %i",line);
                                exit(1);
                            }
                        }
                        if (strcmp(key,"diffuse_color")==0){
                            if (object[i].kind == 1) {
                                sphere.diffuse[0] = value[0];
                                sphere.diffuse[1] = value[1];
                                sphere.diffuse[2] = value[2];
                            }

                            if (object[i].kind == 2) {
                                plane.diffuse[0] = value[0];
                                plane.diffuse[1] = value[1];
                                plane.diffuse[2] = value[2];
                            }
                        }
                        if (strcmp(key, "position") == 0) {
                            if (type=='l'){
                                lights[j].position[0]=value[0];
                                lights[j].position[1]=value[1];
                                lights[j].position[2]=value[2];

                            }
                            else if (object[i].kind == 1||object[i].kind==2){

                                if (object[i].kind == 1) {
                                    sphere.center[0] = value[0];
                                    sphere.center[1]  = value[1];
                                    sphere.center[2]  = value[2];
                                }


                                if (object[i].kind == 2) {
                                    plane.position[0]  = value[0];
                                    plane.position[1]  = value[1];
                                    plane.position[2]  = value[2];
                                }

                            }
                            else {
                            fprintf(stderr,"unexpected position value on line: %i",line);
                            exit(1);
                            }
                        }
                        if (strcmp(key, "normal") == 0) {
                            if (object[i].kind == 2) {
                                plane.normal[0]  = value[0];
                                plane.normal[1]  = value[1];
                                plane.normal[2]  = value[2];
                            }
                            else{
                                fprintf(stderr,"unexpected normal value on line: %i",line);
                                exit(1);
                            }
                        }
                    }
                    else if(strcmp(key,"radial-a2")==0){
                        double value = next_number(json);
                        lights->radA2=value;
                    }
                    else if(strcmp(key,"radial-a1")==0){
                        double value = next_number(json);
                        lights->radA1=value;
                    }
                    else if(strcmp(key,"radial-a0")==0){
                        double value = next_number(json);
                        lights->radA0=value;
                    }
                    else if(strcmp(key,"angular-a0")==0){
                        double value = next_number(json);
                        lights->angA0=value;
                    }

                    else if(strcmp(key,"color")==0&&type=='l'){
                        double* value = next_vector(json);
                        lights[j].color[0]=value[0];
                        lights[j].color[1]=value[1];
                        lights[j].color[2]=value[2];
                        //printf("%f",lights[j].color[1]);
                    }

                    else if(strcmp(key,"direction")==0){
                        double* value = next_vector(json);
                        lights[j].direction[0]=value[0];
                        lights[j].direction[1]=value[1];
                        lights[j].direction[2]=value[2];
                    }
                    else if(strcmp(key,"theta")==0){
                        double value = next_number(json);
                        lights[j].theta=value;
                    }


                    else {
                        fprintf(stderr, "Error: Unknown property, \"%s\", on line %d.\n",
                                key, line);
                        exit(1);

                        //char* value = next_string(json);
                    }
                    skip_ws(json);

                } else {
                    fprintf(stderr, "Error: Unexpected non comma value on line %d\n", line);
                    exit(1);
                }
                //assigning values in object array to temporary Objects
                object[i].cam=cam;
                object[i].sphere=sphere;
                object[i].plane=plane;
            }
            skip_ws(json);

            c = next_c(json);

            if (c == ',') {
                if(type =='o'){i++;}
                if(type=='l'){j++;}
                skip_ws(json);

            } else if (c == ']') {
                fclose(json);
                return;

            } else {
                fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
                exit(1);
            }
        }

    }

}


