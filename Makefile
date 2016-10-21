all:
    gcc main.c -lm -o raytracer

clean:
    rm -rf raytracer *.o *~