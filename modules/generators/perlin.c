/*
 * Copyright (c) 2016 Jean Guyomarc'h
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * This perlin algorithm implementation is strongly inspired from:
 * https://gist.github.com/nowl/828013
 */

#include <Eina.h>

static int _seed = 0;

#define POOL_SIZE 256
static int _random_pool[POOL_SIZE];

static int noise2(int x, int y)
{
    int tmp = _random_pool[(y + _seed) % POOL_SIZE];
    return _random_pool[(tmp + x) % POOL_SIZE];
}

static float lin_inter(float x, float y, float s)
{
    return x + s * (y-x);
}

static float smooth_inter(float x, float y, float s)
{
    return lin_inter(x, y, s * s * (3-2*s));
}

static float noise2d(float x, float y)
{
    int x_int = x;
    int y_int = y;
    float x_frac = x - x_int;
    float y_frac = y - y_int;
    int s = noise2(x_int, y_int);
    int t = noise2(x_int+1, y_int);
    int u = noise2(x_int, y_int+1);
    int v = noise2(x_int+1, y_int+1);
    float low = smooth_inter(s, t, x_frac);
    float high = smooth_inter(u, v, x_frac);
    return smooth_inter(low, high, y_frac);
}

static float
perlin2d(unsigned int x,
         unsigned int y,
         float freq, int depth)
{
    float xa = (float)x*freq;
    float ya = (float)y*freq;
    float amp = 1.0;
    float fin = 0;
    float div = 0.0;

    int i;
    for(i=0; i<depth; i++)
    {
        div += 256 * amp;
        fin += noise2d(xa, ya) * amp;
        amp /= 2;
        xa *= 2;
        ya *= 2;
    }

    return fin/div;
}

static void
perlin_reconfigure(void)
{
   unsigned int i;
   for (i = 0; i < POOL_SIZE; i++)
     _random_pool[i] = rand() % 255;
}

static Eina_Bool
perlin_init(void)
{

   eina_init();
   perlin_reconfigure();

   return EINA_TRUE;
}

static void
perlin_shutdown(void)
{
   eina_shutdown();
}


/*============================================================================*
 *                              External Symbols                              *
 *============================================================================*/

extern float *
perlin(unsigned int width,
       unsigned int height,
       float        freq,
       unsigned int depth)
{
   unsigned int i, j, k = 0;
   float *arr;

   _seed = rand();

   arr = malloc(width * height * sizeof(*arr));
   if (EINA_UNLIKELY(!arr))
     {
        EINA_LOG_CRIT("Failed to allocate memory");
        return NULL;
     }

   for (j = 0; j < height; j++)
     {
        for (i = 0; i < width; i++)
          {
             arr[k++] = perlin2d(i, j, freq, depth);
          }
     }
   return arr;
}

EINA_MODULE_INIT(perlin_init);
EINA_MODULE_SHUTDOWN(perlin_shutdown);
EINA_MODULE_LICENSE("MIT");
EINA_MODULE_AUTHOR("Jean Guyomarc'h");
