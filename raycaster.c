#include "includes.h"

//struct for storing pixels to write to ppm file
typedef struct{
    int r;
    int g;
    int b;

}Pixel;

static inline double sqr(double v){
    return v*v;
}
static inline void v3_add(double* a, double* b, double* c) {
    c[0] = a[0] + b[0];
    c[1] = a[1] + b[1];
    c[2] = a[2] + b[2];
}
static inline void normalize(double* v){
    double len = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
    v[0] /= len;
    v[1] /= len;
    v[2] /= len;
}
static inline void v3_scale(double* a, double s, double* c) {
    c[0] = s * a[0];
    c[1] = s * a[1];
    c[2] = s * a[2];
}
static inline double v3_dot(double* a, double* b) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
static inline void v3_subtract(double* a, double* b, double* c) {
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
}
void illuminate(Object object,double t_value,double *rd,double*ro,int pixel_index,Light* lights,Object** objects,Pixel **p);
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

static inline double distance(double *p1, double *p2) {
    return sqrt(square(p2[0] - p1[0]) + square(p2[1] - p1[1]) +
                square(p2[2] - p1[2]));
}

static inline double frad(double lightDistance, double a0, double a1, double a2) {

    if (lightDistance == INFINITY) {
        return 1.0;
    }
    else {
        return 1/(a2 * sqr(lightDistance) + (a1 * lightDistance) + a0);
    }
}
double clamp(double color){
    if(color>1){
        return 1;
    }
    if (color<0){
        return 0;
    }
    return color;
}
static inline double fang(Light light,double* direction){
    if(light.angA0== NULL){
        return 1;
    }
    else{
        double cosA= v3_dot(light.direction,direction);
        double ret = pow(cosA,light.angA0);

        return ret;
    }

}
static inline void specularReflection(double factor, double *L, double *R,
                                      double *N, double *V, double *lColor,
                                      double *oColor, double *spec) {
    double VR = dot(V, R);
    double NL = dot(N, L);
    if (VR > 0 && NL > 0) {
        double temp = pow(VR, factor);
        double temp2[3];
        temp2[0] = lColor[0] * oColor[0];
        temp2[1] = lColor[1] * oColor[1];
        temp2[2] = lColor[2] * oColor[1];
        v3_scale(temp2,temp,spec);
    } else {
        spec[0] = 0;
        spec[1] = 0;
        spec[2] = 0;
    }
}
static inline void diffuseReflection(double *N, double *L, double *lColor,
                                     double *oColor, double *diff) {
    double temp = dot(N, L);
    if (dot > 0) {
        double temp2[3];
        temp2[0] = lColor[0] * oColor[0];
        temp2[1] = lColor[1] * oColor[1];
        temp2[2] = lColor[2] * oColor[1];
        v3_scale(temp2,temp,diff);
    } else {
        diff[0] = 0;
        diff[1] = 0;
        diff[2] = 0;
    }
}



/*********************Sphere Intersection*************/
double sphere_intersection(double* ro,double* rd,double rad,double* center ){
    //doing math
    double a = sqr(rd[0]) + sqr(rd[1]) + sqr(rd[2]);
    double b = 2 * (rd[0] * (ro[0] - center[0]) + rd[1] * (ro[1] - center[1]) + rd[2] * (ro[2] - center[2]));
    double c = sqr(ro[0] - center[0]) + sqr(ro[1] - center[1]) + sqr(ro[2] - center[2]) - sqr(rad);


    double det = sqr(b) - 4 * a * c;//finding determinant


    if (det < 0)
        return -1;

    det = sqrt(det);

    //for quadratic equation can be positive and it can be negative so we do it twice.
    double t0 = (-b - det) / (2 * a);// One for it being negative

    if (t0 > 0)
        return t0;//returning t value

    double t1 = (-b + det) / (2 * a);// One for it being positive
    if (t1 > 0)
        return t1;//returing t value

    return -1;
}

/*******************Plane Intersection*****************/
double plane_intersection(double*ro,double*rd,double* normal,double* position){
    //doing math;
    normalize(normal);
    double D = -(v3_dot(position,normal)); // distance from origin to plane
    double t = -(normal[0] * ro[0] + normal[1] * ro[1] + normal[2] * ro[2] + D) /
                (normal[0] * rd[0] + normal[1] * rd[1] + normal[2] * rd[2]);

    if (t > 0)
        return t;//returning t value

    return -1;
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
            if(best_t>0 && best_t!=INFINITY){
                illuminate(object,best_t,rd,ro,k,lights,objects,p);
            }
            else{
                p[k].r=0;
                p[k].g=0;
                p[k].b=0;
            }
            k++;
        }
    }
            //if best_t shows the ray is intersecting it assign the color of the object to the pixel array

    write_p3(p,atoi(picture_height),atoi(picture_width),output_file);
}
void illuminate(Object object,double t_value,double *rd,double*ro,int pixel_index,Light* lights,Object** objects,Pixel **p){
    double temp[3];
    double ron[3];
    double rdn[3];
    double color[3];
    double light_distance;
    Object closest_shadow_object;
    for(int i = 0; i<sizeof(lights);i++){
        Object tempObj;
        double* N=malloc(sizeof(double)*3);
        double* L=malloc(sizeof(double)*3);;
        double* R=malloc(sizeof(double)*3);;
        double* V=malloc(sizeof(double)*3);;
        v3_scale(rd,t_value,temp);
        v3_add(temp,ro,ron);

        v3_subtract(lights[i].position,ron,rdn);

        for(int j =0;j< sizeof(objects);j++){
            double best_t=0;
            if(object.kind==1){
                double light_distance=distance(lights[i].position,object.sphere.center);
            }
            if (object.kind==2){
                double light_distance=distance(lights[i].position,object.plane.position);
            }

            double* diffuse,*specular,*position;
            if (objects[i]==&object)continue;

            for (int i = 0; i < sizeof(objects); i++) {
                double t = 0;
                //switching based upon what type the object is and sending it to the right intersection
                switch (objects[i]->kind) {
                    case 1:
                        t = sphere_intersection(ro, rd, objects[i]->sphere.radius, objects[i]->sphere.center);
                        break;
                    case 2:
                        t = plane_intersection(ro, rd, objects[i]->plane.normal, objects[i]->plane.position);
                        break;
                    case 0:
                        break;
                    default:
                        fprintf(stderr, "Unknown Type Found");
                        exit(-1);
                }
                if(best_t>light_distance){
                    continue;
                }
            }
            if(closest_shadow_object==NULL){
                if(object.kind==1){
                    diffuse=object.sphere.diffuse;
                    specular=object.sphere.specular;
                    position=object.sphere.center;
                    v3_subtract(ron,tempObj.sphere.center,N);

                }
                if(object.kind==2){
                    diffuse=object.plane.diffuse;
                    specular=object.plane.specular;
                    position=object.plane.position;
                    N=tempObj.plane.normal;
                }
                v3_subtract(lights[i].position,ron,L);
                double temper = 2*v3_dot(L,N);
                v3_scale(N,temper,R);
                v3_subtract(R,L,R);
                V=rd;

                specularReflection(10,L,R,position,V,lights[i].color,specular,specular);
                diffuseReflection(position,L,lights[i].color,diffuse,diffuse);

                color[0] =(diffuse[0]+specular[0])*
                        frad(light_distance,lights[i].radA0,lights[i].radA1,lights->radA2)*
                        fang(lights[i],rdn);
                color[1] =(diffuse[1]+specular[1])*
                        frad(light_distance,lights[i].radA0,lights[i].radA1,lights->radA2)*
                        fang(lights[i],rdn);
                color[2] =(diffuse[2]+specular[2])*
                        frad(light_distance,lights[i].radA0,lights[i].radA1,lights->radA2)*
                        fang(lights[i],rdn);







            }

        }

        p[pixel_index]->r=(int)255*clamp(color[0]);
        p[pixel_index]->r=(int)255*clamp(color[1]);
        p[pixel_index]->r=(int)255*clamp(color[2]);



}

}