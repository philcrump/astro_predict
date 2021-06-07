
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <time.h>
#include <math.h>

#include "pal/pal.h"

/* Light time for 1 AU (sec) */
static const double PAL__CR = 499.004782;
/* Seconds per day */
static const double PAL__SPD = 86400.0;

#include "erfa/src/erfa.h"

#define DLL_FUNC
#define DLLPTR
#define FAR
#include "lunar/lunar.h"

static void lunarPlanet(double mjd, int np, double pv[6])
{
  FILE *ps1996_file;
  void *ps1996_ptr;

  ps1996_file = fopen("PS_1996.DAT", "rb");
  if(ps1996_file == NULL)
  {
    printf("ps_1996.dat not loaded\n");
    return;
  }
  ps1996_ptr = load_ps1996_series(ps1996_file, mjd+2400000.5, np);
  if(ps1996_ptr == NULL)
  {
      printf("Error loading from PS1996\n");
      return;
  }
  if(get_ps1996_position(mjd+2400000.5, ps1996_ptr, pv, 1) != 0)
  {
      printf("Error computing from PS1996\n");
      return;
  }
  unload_ps1996_series(ps1996_ptr);

  fclose(ps1996_file);

  /* Convert velocity from AU/d to AU/s */
  pv[3] /= PAL__SPD;
  pv[4] /= PAL__SPD;
  pv[5] /= PAL__SPD;
}

/* A re-writing of palRdPlan using liblunar PS1996 series */
/* NB: ARGUMENTS ARE NOT THE SAME */
/*
*  Arguments:
*     date = double (Given)
*        MJD of observation (JD-2400000.5) in TDB. For all practical
*        purposes TT can be used instead of TDB, and for many applications
*        UT will do (except for the Moon).
*     np = int (Given)
*        Planet: 1 = Mercury
*                2 = Venus
*                3 = Moon
*                4 = Mars
*                5 = Jupiter
*                6 = Saturn
*                7 = Uranus
*                8 = Neptune
*             else = Sun
*     elong = double (Given)
*        Observer's east longitude (radians)
*     phi = double (Given)
*        Observer's geodetic latitude (radians)
*     ra = double * (Returned)
*        RA (topocentric apparent, radians)
*     dec = double * (Returned)
*        Dec (topocentric apparent, radians)
*     range = double * (Returned)
*        Range (one-way, AU)
*     range_rate = double * (Returned)
*        Range-Rate (one-way, AU/s)
*/
void lunarRdplan( double date, int np, double elong, double phi,
                double * ra, double * dec, double *range, double *range_rate)
{
  /* AU in km */
  //const double AUKM = 1.49597870e8;

  /* Local variables */
  int i;
  double stl;
  double vgm[6];
  double v[6];
  double rmat[3][3];
  double vse[6];
  double vsg[6];
  double vsp[6];
  double vgo[6];
  double dx,dy,dz,r,tl;

  /* Approximate local sidereal time */
  stl = palGmst( date - palDt( palEpj(date)) / 86400.0) + elong;

  /* Geocentre to Moon (mean of date) */
  palDmoon( date, v );

  /* Nutation to true of date */
  palNut( date, rmat );
  eraRxp( rmat, v, vgm );
  eraRxp( rmat, &(v[3]), &(vgm[3]) );

  /* Not moon: precession/nutation matrix J2000 to date */
  palPrenut( 2000.0, date, rmat );

  /* Sun to Earth-Moon Barycentre (J2000) */
  //palPlanet( date, 3, v, &j );
  lunarPlanet(date, 3, v);

  /* Precession and nutation to date */
  eraRxp( rmat, v, vse );
  eraRxp( rmat, &(v[3]), &(vse[3]) );

  /* Sun to geocentre (true of date) */
  for (i=0; i<6; i++) {
    vsg[i] = vse[i] - 0.012150581 * vgm[i];
  }

  /* Sun to Planet (J2000) */
  //palPlanet( date, np, v, &j );
  lunarPlanet(date, np, v);

  /* Precession and nutation to date */
  eraRxp( rmat, v, vsp );
  eraRxp( rmat, &(v[3]), &(vsp[3]) );

  /* Geocentre to planet */
  for (i=0; i<6; i++) {
    v[i] = vsp[i] - vsg[i];
  }

  /* Refer to origina at the observer */
  palPvobs( phi, 0.0, stl, vgo );
  for (i=0; i<6; i++) {
    v[i] -= vgo[i];
  }

  /* Geometric distance (AU) */
  dx = v[0];
  dy = v[1];
  dz = v[2];
  r = sqrt( dx*dx + dy*dy + dz*dz );

  /* Light time */
  tl = PAL__CR * r;

  /* Correct position for planetary aberration */
  for (i=0; i<3; i++) {
    v[i] -= tl * v[i+3];
  }

  /* To RA,Dec */
  eraC2s( v, ra, dec );
  *ra = eraAnp( *ra );

  *range = r;
  *range_rate = sqrt( v[3]*v[3] + v[4]*v[4] + v[5]*v[5] );
}