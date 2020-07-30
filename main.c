#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <time.h>
#include <math.h>

#include "pal/pal.h"

#define STATION_LATITUDE_DEG    51.25
#define STATION_LONGITUDE_DEG   (-0.6)

#define SIDEREAL_DAY_HOURS  24.0 // SOLAR DAY: 23.9344696

#define RAD2DEG(x)  (((x) * 180.0) / M_PI)
#define DEG2RAD(x)  (((x) / 180.0) * M_PI)

#define RAD2HR(x)   (((x) * SIDEREAL_DAY_HOURS) / (2 * M_PI))
#define HR2RAD(x)   (((x) / SIDEREAL_DAY_HOURS) * (2 * M_PI))

#define DEG2HR(x)   (((x) / 360.0) * SIDEREAL_DAY_HOURS)

uint64_t timestamp_ms(void)
{
    struct timespec tp;

    if(clock_gettime(CLOCK_REALTIME, &tp) != 0)
    {
        return 0;
    }

    return ((uint64_t)tp.tv_sec * 1000) + (tp.tv_nsec / (1000*1000));
}

double unixMsTolocalSiderealTime(double modified_julian_date, double station_longitude_deg)
{
    double greenwich_mean_sidereal_time = RAD2HR(palGmst(modified_julian_date));

    printf("GMST: %.2fh (%dm %.1fs)\r\n",
        greenwich_mean_sidereal_time,
        (uint32_t)(60*(greenwich_mean_sidereal_time - floor(greenwich_mean_sidereal_time))),
        60*((60*greenwich_mean_sidereal_time) - floor((60*greenwich_mean_sidereal_time)))
    );

    double lst_hours = greenwich_mean_sidereal_time + DEG2HR(station_longitude_deg);

    if(lst_hours > 24.0)
    {
        lst_hours = lst_hours - 24.0;
    }
    else if(lst_hours < 0.0)
    {
        lst_hours = lst_hours + 24.0;
    }

    printf("LST: %.2fh\r\n", lst_hours);

    return lst_hours;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("== astro_predict ==\r\n");

    double modified_julian_date = (( ((double)timestamp_ms()) / 86400000.0 ) + 40587.0); // MJD = (JD - 2400000.5)
    printf("MJD: %.5f\r\n", modified_julian_date);

    double ra_hours;
    double dec_deg;

    double local_sidereal_hours = unixMsTolocalSiderealTime(modified_julian_date, STATION_LONGITUDE_DEG);

    //double hour_angle = local_sidereal_hours - ra_hours;

    double azimuth, elevation;

    /** Mars (JPL Horizons - 29th Jul 2020) **/
    ra_hours  =  1. + 9./60. + 28.82/3600.;
    dec_deg   =  3. + 16./60. + 28.9/3600.;

    // Mean Place -> geocentric apparent: palMap ( double rm, double dm, double pr, double pd, double px, double rv, double eq, double date, double ∗ra, double ∗da ); 

    //Apparent (mass center) -> Observed (with refraction): palAop ( double rap, double dap, double date, double dut, double elongm, double phim, double hm, double xp, double yp, double tdk, double pmb, double rh, double wl, double tlr, double ∗aob, double ∗zob, double ∗hob, double ∗dob, double ∗rob ); 

    // palDe2h() Convert equatorial to horizon coordinates.
    palDe2h(
        HR2RAD(local_sidereal_hours - ra_hours), DEG2RAD(dec_deg), DEG2RAD(STATION_LATITUDE_DEG),
        &azimuth, &elevation
    );

    printf("MARS:        Az: %.2f°, El: %.2f°\r\n", RAD2DEG(azimuth), RAD2DEG(elevation));

    // palRdplan()  Approximate topocentric apparent RA,Dec of a planet, and its angular diameter.
    // 1 = Mercury 2 = Venus 3 = Moon 4 = Mars 5 = Jupiter 6 = Saturn 7 = Uranus 8 = Neptune
    double calc_ra, calc_dec, calc_dia;
    palRdplan(
        modified_julian_date, 4, DEG2RAD(STATION_LONGITUDE_DEG), DEG2RAD(STATION_LATITUDE_DEG),
        &calc_ra, &calc_dec, &calc_dia
    );

    // palDe2h() Convert equatorial to horizon coordinates.
    palDe2h(
        HR2RAD(local_sidereal_hours - RAD2HR(calc_ra)), calc_dec, DEG2RAD(STATION_LATITUDE_DEG),
        &azimuth, &elevation
    );

    printf("MARS (calc): Az: %.2f°, El: %.2f°\r\n", RAD2DEG(azimuth), RAD2DEG(elevation));


    // Geocentric Apparent (mass center) -> Observed (with refraction): palAop ( double rap, double dap, double date, double dut, double elongm, double phim, double hm, double xp, double yp, double tdk, double pmb, double rh, double wl, double tlr, double ∗aob, double ∗zob, double ∗hob, double ∗dob, double ∗rob ); 
    // WARNING: geocentric / topocentric 

    /** Tianwen-1 (28th Jul 2020) **/

    ra_hours  = 18. + 36./60. + 56.3/3600.;
    dec_deg   = 38. + 47./60. + 01./3600.;

    palDe2h(
        (local_sidereal_hours - ra_hours), DEG2RAD(dec_deg), DEG2RAD(STATION_LATITUDE_DEG),
        &azimuth, &elevation
    );

    printf("Tianwen-1: Az: %.1f°, El: %.1f°\r\n", RAD2DEG(azimuth), RAD2DEG(elevation));


    return 0;
}