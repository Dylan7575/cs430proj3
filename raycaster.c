#include "includes.h"
#include "raycaster.h"
//struct for storing pixels to write to ppm file

/***************Writing to pixel buffer to output file*****************/
void write_p3(Pixel* pixel,int w,int h,char* filename){

    FILE *fh = fopen(filename,"w+");//opening the file handle
    if(fh==NULL){
        fprintf(stderr,"Could not Open Output File");
        exit(1);
    }
    fprintf(fh,"P3 %d %d 255 \n",w,h);//writing header
    for(int i = 0;i<w*h;i++){
        //looping through pix and writing it to the file
        fprintf(fh,"%d ",pixel[i].r);
        fprintf(fh,"%d ",pixel[i].g);
        fprintf(fh,"%d \n",pixel[i].b);
    }
    fclose(fh);//closing file handle
}


/*******************Raycasting*************************/
void raycast(Object* objects,Light* lights,char* picture_width,char* picture_height,char* output_file){

    int j=0,k=0;//loops

    //camera position//
    double cx=0;
    double cy=0;
    /***********/

    //finding the camera in the Objects
    for(j=0;j< sizeof(objects);j++){
        if(objects[j].kind==0){
            break;
        }
    }
    /*****************/

    /*setting the height and width for the camera and the scene and finding the pixheight and width from it*/
    double h =objects[j].cam.height;
    double w=objects[j].cam.width;
    int m =atoi(picture_height);
    int n = atoi(picture_width);
    double pixheight =h/m;
    double pixwidth =w/n;
    /****************/

    Pixel p[m*n];//creating pixel array
    pix=malloc(m*n);
    //Looping thorough each x and y coordinates flipping y coordinates to make the image correct
    for(int y=m;y>0;y--) {
        for (int x = 0; x < n; x++) {
            double ro[3] = {0, 0, 0};//ray origin
            double rd[3] = {     //ray direction
                    cx - (w / 2) + pixwidth * (x + 0.5),
                    cy - (h / 2) + pixheight * (y + 0.5),
                    1
            };
            normalize(rd);//normalizing the ray direction
            double best_t = INFINITY;
            double best_t2;
            Object object; //creating a temp Object to hold the object that is intersecting with  the ray
            double colour[3];
            //looping through the objects
            for (int i = 0; i < sizeof(objects); i++) {
                double t = 0;
                //switching based upon what type the object is and sending it to the right intersection
                switch (objects[i].kind) {
                    case 1:
                        t = sphere_intersection(ro, rd, objects[i].sphere.radius, objects[i].sphere.center);
                        break;
                    case 2:
                        t = plane_intersection(ro, rd, objects[i].plane.normal, objects[i].plane.position);
                        break;
                    case 0:
                        break;
                    default:
                        fprintf(stderr, "Unknown Type Found");
                        exit(-1);
                }

                //finding the best intersection and then assigning object to it and resetting best t so it can check with other objects
                if (t > 0 && t < best_t) {
                    best_t = t;
                    object = objects[i];
                }
            }
            if(best_t>0 && best_t!=INFINITY ){
                illuminate(object,best_t,rd,ro,lights,objects,k,pix);
                //if there is an intersection send it to be illuminated

            }
            else{
                pix[k].r=0;
                pix[k].g=0;
                pix[k].b=0;
            }
            k++;
        }
    }
            //if best_t shows the ray is intersecting it assign the color of the object to the pixel array

    write_p3(pix,atoi(picture_height),atoi(picture_width),output_file);
}

void illuminate(Object object,double t_value,double *rd,double*ro,Light* lights,Object* objects,int pixindex,Pixel* pix) {

    double temp[3];
    double ron[3];
    double rdn[3];
    double color[3] = {0, 0, 0};
    double light_distance;


    for (int j = 0; j < sizeof(lights); j++) {
        //values for equations
        double *N = malloc(sizeof(double) * 3);
        double *L = malloc(sizeof(double) * 3);
        double *R = malloc(sizeof(double) * 3);
        double *V = malloc(sizeof(double) * 3);
        double *diffuse = malloc(sizeof(double) * 3);
        double *specular = malloc(sizeof(double) * 3);
        double *position = malloc(sizeof(double) * 3);

        double t = 0;

        v3_scale(rd, t_value, temp);//getting the new ray origin
        v3_add(temp, ro, ron);



        v3_subtract(lights[j].position, ron, rdn);//getting new ray direction

        //getting the light distance based upon which object type it is
        if (object.kind == 1) {
            light_distance = distance(lights[j].position, object.plane.position);
        }
        else if (object.kind == 2) {
            light_distance = distance(lights[j].position, object.sphere.center);
        }

        normalize(rdn);
        //shadowed value for if its in shadow using same intersection formula as above
        int shadowed;
        for (int i = 0; i < sizeof(objects); i++) {
            shadowed = 0;
            if (equals(objects[i],object))continue;

            //switching based upon what type the object is and sending it to the right intersection
            switch (objects[i].kind) {
                case 1:
                    t = sphere_intersection(ron, rdn, objects[i].sphere.radius, objects[i].sphere.center);
                    break;
                case 2:
                    t = plane_intersection(ron, rdn, objects[i].plane.normal, objects[i].plane.position);
                    break;
                case 0:
                    break;
                default:
                    fprintf(stderr, "Unknown Type Found");
                    exit(-1);
            }
            if (t <= light_distance && t > 0 && t<INFINITY) {
                shadowed = 1;//showing that it is shadowed
            }

        }
        if (shadowed == 0) {//if it isnt go ahead with calculations

            //getting the diffuse, specular, and normal from structures
            if (object.kind == 1) {
                v3_subtract(ron, object.sphere.center, N);
                vector_copy(object.sphere.specular, specular);
                vector_copy(object.sphere.diffuse, diffuse);
            }
            if (object.kind == 2) {
                vector_copy(object.plane.diffuse, diffuse);
                vector_copy(object.plane.specular, specular);
                vector_copy(object.plane.normal, N);
            }

            normalize(N);


            vector_copy(rdn, L);//copying rdn into L for ease of matching the equations
            normalize(L);

            reflect(L, N, R);  //doing the reflection equation
            normalize(R);

            vector_copy(rd, V);//getting V and inverting it to get picture
            v3_scale(V, -1, V);



            specularReflection(10, L, R, N, V, lights[j].color, specular);//calculating specular
            diffuseReflection(N, L, lights[j].color, diffuse);//calculating diffuse
            //v3_scale(rdn, -1, V);
            double fRad = frad(light_distance, lights[j].radA0, lights[j].radA1, lights[j].radA2);//calculating radial attenuation
            double fAng = fang(lights[j],L);//calculating angular attenuation


            color[0] += (diffuse[0] + specular[0]) * fRad * fAng;//calculating color by adding and multiplying values up
            color[1] += (diffuse[1] + specular[1]) * fRad * fAng;
            color[2] += (diffuse[2] + specular[2]) * fRad * fAng;
            //printf("%f\n",color[2]);
        }
    }
        pix[pixindex].r=(unsigned char )255*clamp(color[0]);//assigning to pixel array for printing
        pix[pixindex].g=(unsigned char )255*clamp(color[1]);
        pix[pixindex].b=(unsigned char )255*clamp(color[2]);
}