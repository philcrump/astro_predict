#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <time.h>
#include <math.h>

#include "pal/pal.h"
#include "stars.h"
#include "lunarRdPlan.h"

#define KMINAU 1.49597870e8

#define VACLIGHTSPEED_KMPS  299792.458 // km/s

// Vernon Way
#define STATION_LATITUDE_DEG    51.25
#define STATION_LONGITUDE_DEG   (-0.6)
#define STATION_ALTITUDE_M   (100)

#define SIDEREAL_DAY_HOURS  24.0 // SOLAR DAY: 23.9344696
// Note that the seconds here are sidereal rather than SI.  One
//  sidereal second is about 0.99727 SI seconds.

#define RAD2DEG(x)  (((x) * 180.0) / M_PI)
#define DEG2RAD(x)  (((x) / 180.0) * M_PI)

#define RAD2HR(x)   (((x) * SIDEREAL_DAY_HOURS) / (2 * M_PI))
#define HR2RAD(x)   (((x) / SIDEREAL_DAY_HOURS) * (2 * M_PI))

#define DEG2HR(x)   (((x) / 360.0) * SIDEREAL_DAY_HOURS)

#define MAS2RAD(x)  ((x) * 4.8481368110954E-9)

// https://hpiers.obspm.fr/eop-pc/index.php
#define POLAR_X_MAS     (124.53)
#define POLAR_Y_MAS     (445.24)
#define DELTA_UT_MS     (-182.909)

uint64_t timestamp_ms(void)
{
    struct timespec tp;

    if(clock_gettime(CLOCK_REALTIME, &tp) != 0)
    {
        return 0;
    }

    return ((uint64_t)tp.tv_sec * 1000) + (tp.tv_nsec / (1000*1000));
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("== astro_predict ==\r\n");

    /* Antenna Location */
    double antenna_latitude_rad = DEG2RAD(STATION_LATITUDE_DEG);
    double antenna_longitude_rad = DEG2RAD(STATION_LONGITUDE_DEG);

#if 0
    /* Polar Motion - Overkill (typically a few meters and ~0.2 millidegrees azimuth) */
    double antenna_azimuth_polar_correction_rad = 0.0;
    palPolmo(
        antenna_longitude_rad, antenna_latitude_rad,
        MAS2RAD(POLAR_X_MAS), MAS2RAD(POLAR_Y_MAS),
        &antenna_longitude_rad, &antenna_latitude_rad,
        &antenna_azimuth_polar_correction_rad
    );
#endif

    printf("Antenna:\n");
    printf(" - Latitude:  %.05f°\n", RAD2DEG(antenna_latitude_rad));
    printf(" - Longitude: %.05f°\n", RAD2DEG(antenna_longitude_rad));

    /* Timestamps */
    printf("Timing:\n");

    double utc = ((double)timestamp_ms() / 1000.0);
    double ut1 = utc + (DELTA_UT_MS / 1000.0);

    double modified_julian_date = (( ut1 / 86400.0 ) + 40587.0); // MJD = (JD - 2400000.5)

    printf(" - MJD: %.5f\r\n", modified_julian_date); // Can be checked on https://planetcalc.com/503/ (note UTC/Local TZ)

    double greenwich_mean_sidereal_time_hours = RAD2HR(palGmst(modified_julian_date));

    printf(" - GMST: %.4fh (%dh %dm %.1fs)\r\n",
        greenwich_mean_sidereal_time_hours,
        (uint32_t)(floor(greenwich_mean_sidereal_time_hours)),
        (uint32_t)(60*(greenwich_mean_sidereal_time_hours - floor(greenwich_mean_sidereal_time_hours))),
        60*((60*greenwich_mean_sidereal_time_hours) - floor((60*greenwich_mean_sidereal_time_hours)))
    );

    double local_mean_sidereal_time_hours = greenwich_mean_sidereal_time_hours + RAD2HR(antenna_longitude_rad);
    if(local_mean_sidereal_time_hours > 24.0) local_mean_sidereal_time_hours -= 24.0;
    if(local_mean_sidereal_time_hours < 0.0) local_mean_sidereal_time_hours += 24.0;

    printf(" - LMST: %.4fh (%dh %dm %.1fs)\r\n",
        local_mean_sidereal_time_hours,
        (uint32_t)(floor(local_mean_sidereal_time_hours)),
        (uint32_t)(60*(local_mean_sidereal_time_hours - floor(local_mean_sidereal_time_hours))),
        60*((60*local_mean_sidereal_time_hours) - floor((60*local_mean_sidereal_time_hours)))
    );

    /* Mean Place to Apparent Place
        - Light deflection
        - Annual Aberration
        - Precession-Nutation
    */
    double out_ra_rad, out_dec_rad;

    palMap(
        radiostar_taua.mean_ra_rad,
        radiostar_taua.mean_dec_rad,
        radiostar_taua.pm_ra_rad_per_year,
        radiostar_taua.pm_dec_rad_per_year,
        radiostar_taua.parallax_arcsec,
        radiostar_taua.radial_vel_kmpersec,
        radiostar_taua.epoch_equinox_julian,
        modified_julian_date,
        &out_ra_rad, &out_dec_rad
    );

    printf("palMap:\n");
    printf(" - Before: RA %.4f, DEC: %.4f\n", radiostar_taua.mean_ra_rad, radiostar_taua.mean_dec_rad);
    printf(" - After:  RA %.4f, DEC: %.4f\n", out_ra_rad, out_dec_rad);

    /* Apparent Place to Observed Place */

    double ambient_temperature_kelvin = ((15.5)+273.15);
    double ambient_pressure_mb = (998.6);
    double ambient_humidity_normalised = (0.9);
    double ambient_lapserate_kpermeter = (0.00649); // (ICAO is 0.00649 K/m)

    double rf_wavelength_microns = (40.0*1000); // Set for approx 7.5GHz RF (40mm)

    double aop_az_rad, aop_zen_rad, ha_rad, dec_rad, ra_rad;
    palAop(
        out_ra_rad, // Right Ascension (Radians)
        out_dec_rad, // Declination (Radians)
        modified_julian_date,
        (DELTA_UT_MS / 1000.0),
        antenna_longitude_rad, antenna_latitude_rad, STATION_ALTITUDE_M,
        MAS2RAD(POLAR_X_MAS), MAS2RAD(POLAR_Y_MAS), // Polar Motion (radians)
        ambient_temperature_kelvin, // Ambient Temperature (K)
        ambient_pressure_mb, // Ambient Pressure (mB)
        ambient_humidity_normalised, // Relative Humidity (normalised)
        rf_wavelength_microns, // Wavelength (micron)
        ambient_lapserate_kpermeter, // Tropospheric lapse rate (K/metre)
        &aop_az_rad, &aop_zen_rad, &ha_rad,
        &dec_rad, &ra_rad
    );

    /* Negative Azimuth */
    if(aop_az_rad < 0) { aop_az_rad += 2*M_PI; }

    printf("RA/DEC -> Az: %.3f°, El: %.3f°\r\n", RAD2DEG(aop_az_rad), 90.0-RAD2DEG(aop_zen_rad));

    printf("**** MARS ****\n");

    // palRdplan()  Approximate topocentric apparent RA,Dec of a planet, and its angular diameter.
    // 1 = Mercury 2 = Venus 3 = Moon 4 = Mars 5 = Jupiter 6 = Saturn 7 = Uranus 8 = Neptune
    double calc_ra, calc_dec, calc_dia;
    palRdplan(
        modified_julian_date, 4, DEG2RAD(STATION_LONGITUDE_DEG), DEG2RAD(STATION_LATITUDE_DEG),
        &calc_ra, &calc_dec, &calc_dia
    );

    palAop(
        calc_ra, // Right Ascension (Radians)
        calc_dec, // Declination (Radians)
        modified_julian_date,
        (DELTA_UT_MS / 1000.0),
        antenna_longitude_rad, antenna_latitude_rad, STATION_ALTITUDE_M,
        MAS2RAD(POLAR_X_MAS), MAS2RAD(POLAR_Y_MAS), // Polar Motion (radians)
        ambient_temperature_kelvin, // Ambient Temperature (K)
        ambient_pressure_mb, // Ambient Pressure (mB)
        ambient_humidity_normalised, // Relative Humidity (normalised)
        rf_wavelength_microns, // Wavelength (micron)
        ambient_lapserate_kpermeter, // Tropospheric lapse rate (K/metre)
        &aop_az_rad, &aop_zen_rad, &ha_rad,
        &dec_rad, &ra_rad
    );

    /* Negative Azimuth */
    if(aop_az_rad < 0) { aop_az_rad += 2*M_PI; }

    printf("MARS RdPlan:     Az: %.3f°, El: %.3f°\r\n", RAD2DEG(aop_az_rad), 90.0-RAD2DEG(aop_zen_rad));

    double mars_range, mars_rangerate;
    lunarRdplan(
        modified_julian_date, 4, DEG2RAD(STATION_LONGITUDE_DEG), DEG2RAD(STATION_LATITUDE_DEG),
        &calc_ra, &calc_dec, &mars_range, &mars_rangerate
    );

    palAop(
        calc_ra, // Right Ascension (Radians)
        calc_dec, // Declination (Radians)
        modified_julian_date,
        (DELTA_UT_MS / 1000.0),
        antenna_longitude_rad, antenna_latitude_rad, STATION_ALTITUDE_M,
        MAS2RAD(POLAR_X_MAS), MAS2RAD(POLAR_Y_MAS), // Polar Motion (radians)
        ambient_temperature_kelvin, // Ambient Temperature (K)
        ambient_pressure_mb, // Ambient Pressure (mB)
        ambient_humidity_normalised, // Relative Humidity (normalised)
        rf_wavelength_microns, // Wavelength (micron)
        ambient_lapserate_kpermeter, // Tropospheric lapse rate (K/metre)
        &aop_az_rad, &aop_zen_rad, &ha_rad,
        &dec_rad, &ra_rad
    );

    /* Negative Azimuth */
    if(aop_az_rad < 0) { aop_az_rad += 2*M_PI; }

    printf("MARS PS_1996:    Az: %.3f°, El: %.3f°\r\n", RAD2DEG(aop_az_rad), 90.0-RAD2DEG(aop_zen_rad));
    printf("  Range:  %.3f AU, Range-Rate: %.3f km/s (rate could be negative)\n", mars_range, mars_rangerate * KMINAU);
    printf("  Doppler at 8.45GHz: %.1fHz\n", -0.5 * 8.45e9 * ((mars_rangerate * KMINAU) / VACLIGHTSPEED_KMPS));

    return 0;
}