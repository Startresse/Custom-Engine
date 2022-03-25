
// exemple d'integration numerique sur une hemisphere

#include <cstdio>
#include <cassert>
#include <random>

#include "vec.h"

float f( const Vector& w )
{
    // evalue cos theta
    Vector n= Vector(0, 0, 1);
    assert(w.z > 0);
    return dot(n, w);
}

// genere une direction sur l'hemisphere, 
// cf GI compendium, eq 34
Vector sample34( const float u1, const float u2 )
{
    // coordonnees theta, phi
    float cos_theta= u1;
    float phi= float(2 * M_PI) * u2;
    
    // passage vers x, y, z
    float sin_theta= std::sqrt(1 - cos_theta*cos_theta);
    return Vector( std::cos(phi) * sin_theta, std::sin(phi) * sin_theta, cos_theta );
}

// evalue la densite de proba, la pdf de la direction, cf GI compendium, eq 34
float pdf34( const Vector& w )
{
    if(w.z < 0) return 0;
    return 1 / float(2 * M_PI);
}

// genere une direction sur l'hemisphere, 
// cf GI compendium, eq 35
Vector sample35( const float u1, const float u2 )
{
    // coordonnees theta, phi
    float cos_theta= std::sqrt(u1);
    float phi= float(2 * M_PI) * u2;
    
    // passage vers x, y, z
    float sin_theta= std::sqrt(1 - cos_theta*cos_theta);
    return Vector( std::cos(phi) * sin_theta, std::sin(phi) * sin_theta, cos_theta );
}

// evalue la densite de proba, la pdf de la direction, cf GI compendium, eq 35
float pdf35( const Vector& w )
{
    if(w.z < 0) return 0;
    return w.z / float(M_PI);
}

// remarque : on pourrait aussi ecrire une fonction qui renvoie une direction et sa pdf...


int main( )
{
    // generateur materiel
    std::random_device seed;
    
    // initialise le generateur de nombres aleatoires.
    // (random genere des entiers non signes 32 bits)
    std::default_random_engine random(seed());
    // normalise les nombres aleatoires entre 0 et 1
    std::uniform_real_distribution<float> u01(0, 1);
    
    FILE *out= fopen("plot.txt", "wt");
    assert(out);
    
    int N= 1024*1024;
    for(int n= 1; n <= N; n*= 2)
    {
        float I1= 0;
        for(int i= 0; i < n; i++)
        {
            // genere 2 nombres aleatoires entre 0 et 1
            float u1= u01(random);
            float u2= u01(random);
            
            // genere une direction aleatoire, utilise les nombres aleatoires
            Vector w= sample34(u1, u2);
            // evalue la densite de proba / la pdf de la direction generee
            float p= pdf34(w);
            
            I1+= f(w) / p;
        }
        I1/=float(n);
        printf("N= %d I1= %f\n", n, I1);
        
        float I2= 0;
        for(int i= 0; i < n; i++)
        {
            // genere 2 nombres aleatoires entre 0 et 1
            float u1= u01(random);
            float u2= u01(random);
            
            // genere une direction aleatoire, utilise les nombres aleatoires
            Vector w= sample35(u1, u2);
            // evalue la densite de proba / la pdf de la direction generee
            float p= pdf35(w);
            
            I2+= f(w) / p;
        }
        I2/=float(n);
        printf("N= %d I2= %f\n", n, I2);
        
        fprintf(out, "%d %f %f\n", n, I1, I2);
    }
    fclose(out);
    
    return 0;
}
