typedef struct{
    int r;
    int g;
    int b;

}Pixel;
Pixel illuminate(Object object,double t_value,double *rd,double*ro,Light* lights,Object* objects);

static inline double sqr(double v){
    return v*v;
}
void vector_copy(double* vector1, double* vector2){
    vector2[0]=vector1[0];
    vector2[1]=vector1[1];
    vector2[2]=vector1[2];
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

double distance(double *p1, double *p2) {
    return sqrt(sqr(p2[0] - p1[0]) + sqr(p2[1] - p1[1]) +
                sqr(p2[2] - p1[2]));
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
static inline double fang(Light light,double* direction) {
    double cosA = v3_dot(light.direction, direction);
    cosA = ((cosA * 3.14159265358979323846) / 180);
    if (isnumber(light.angA0) || isnumber(light.direction)) {
        return 1;
    } else if (cosA < light.theta) {
        return 0;
    } else {
        double ret = pow(cosA, light.angA0);

        return ret;
    }
}
static inline void reflect(double *L, double *N, double *R) {
    double temp = 2 * v3_dot(L, N);
    double temp2[3];
    v3_scale(N,temp , temp2);
    v3_subtract(temp2, L, R);
}


static inline void specularReflection(double factor, double *L, double *R,
                                      double *N, double *V, double *lColor,
                                      double *oColor, double *spec) {
    double VR = v3_dot(V, R);
    double NL = v3_dot(N, L);
    if (VR > 0 && NL > 0) {
        double temp = pow(VR, factor);
        double temp2[3];
        temp2[0] = lColor[0] * oColor[0]*temp;
        temp2[1] = lColor[1] * oColor[1]*temp;
        temp2[2] = lColor[2] * oColor[1]*temp;
        vector_copy(temp2,spec);
    } else {
        spec[0] = 0;
        spec[1] = 0;
        spec[2] = 0;
    }
}
static inline void diffuseReflection(double *N, double *L, double *lColor,
                                     double *oColor, double *diff) {
    double temp = v3_dot(N, L);
    if (temp > 0) {
        double temp2[3];
        temp2[0] = lColor[0] * oColor[0]*temp;
        temp2[1] = lColor[1] * oColor[1]*temp;
        temp2[2] = lColor[2] * oColor[1]*temp;
        vector_copy(temp2,diff);
    } else {
        diff[0] = 0;
        diff[1] = 0;
        diff[2] = 0;
    }
}

int equals(Object a,Object b){
    if(a.kind==b.kind){
        if(a.kind==1){
            if(a.sphere.center==b.sphere.center&&
               a.sphere.diffuse==b.sphere.diffuse&&
               a.sphere.radius==b.sphere.radius&&
               a.sphere.specular==b.sphere.specular){
                return 1;
            }
        }
        if(a.kind==2){
            if(a.plane.position==b.plane.position&&
               a.plane.diffuse==b.plane.diffuse&&
               a.plane.normal==b.plane.normal&&
               a.plane.specular==b.plane.specular){
                return 1;
            }
        }
        else return 0;
    }
    else return 0;
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

