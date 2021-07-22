#include <iostream>
#include <iomanip>
#include <math.h>
#include <string.h>

extern "C"
{
#include "sofa.h"
};

#include "SGP4.h"
#include "passpredict.h"



int main()
{
    int i;
    std::cout << "hello\n";

    // Location information
    double lat, lon, h;

    // Define TLE as two strings

    // Put TLE orbit data into appropriate cpp data structure

    // Put Location information into appropriate cpp data structure

    // phi_gd = 39.007  # [deg]
    //     lmda = -104.883  # [deg]
    //     alt = 2187.0  # [m]
    //     r_ECEF = topocentric.site_ECEF(phi_gd, lmda, alt)
    //     r_ECEFtrue = np.array([-1275.1219, -4797.9890, 3994.2975])
    //     for i in [0, 1, 2]:

    lat = 39.007;
    lon = -104.883;
    h = 2187.0;
    passpredict::Location location(lat, lon, h);
    {
        using namespace std;
        cout << "Lat: " << location.lat << "\n";
        cout << "Lon: " << location.lon << "\n";
        cout << "H: " << location.h << "\n";
        // Find location ecef position
        location.site_ecef();
        cout << "recef: ";
        cout << fixed;
        for (i = 0; i < 3; i++)
        {
            cout << location.recef[i] << ", ";
        }
        cout << endl;
    }

    // Put TLE and Location data structures into Observer cpp data structure
    // ISS (ZARYA)
    char tle1[] = "1 25544U 98067A   21201.46980141  .00001879  00000-0  42487-4 0  9993";
    char tle2[] = "2 25544  51.6426 178.1369 0001717 174.7410 330.7918 15.48826828293750";
    passpredict::Orbit sat (tle1, tle2);

    const double pi = 3.14159265358979323846;
    const double deg2rad = PASSPREDICT_DEG2RAD;
	const double xpdotp = 1440.0 / PASSPREDICT_2PI;

    // Print satrec
    {
        using namespace std;
        cout << endl << "satrec from TLE strings" << endl;
        cout << "satrec.jdsatepoch = " << sat.satrec.jdsatepoch << endl;
        cout << "satrec.jdsatepochF = " << sat.satrec.jdsatepochF << endl;
        cout << "sgp4init epoch = " << (sat.satrec.jdsatepoch + sat.satrec.jdsatepochF) - 2433281.5 << endl;
        cout << "satrec.bstar = " << sat.satrec.bstar << endl;
        cout << "satrec.inclo = " << sat.satrec.inclo / deg2rad << endl;
        cout << "satrec.nodeo = " << sat.satrec.nodeo / deg2rad << endl;
        cout << "satrec.ecco = " << sat.satrec.ecco << endl;
        cout << "satrec.argpo = " << sat.satrec.argpo / deg2rad << endl;
        cout << "satrec.mo = " << sat.satrec.mo / deg2rad << endl;
        cout << "satrec.no_kozai = " << sat.satrec.no_kozai * xpdotp << endl;
        cout << "satrec.revnum = " << sat.satrec.revnum << endl;
    };

    // Use Omm
    passpredict::Omm omm;
    strcpy(omm.satnum, "25544");        // satnum
    omm.jdsatepoch = 2.45942e+6;     // jdsatepoch
    omm.jdsatepochF = 0.469801;       // jdsatepochF
    omm.bstar = 4.2487e-5;      // bstar
    omm.inclo = 51.6426;        // inclo
    omm.nodeo = 178.1369;       // nodeo
    omm.ecco = 0.0001717;      // ecco
    omm.argpo = 174.7410;       // argpo
    omm.mo = 330.7918;       // mo
    omm.no_kozai = 15.4883;        // no_kozai
    omm.revnum = 293750;         // revnum
    omm.elnum = 993;            // elnum


    omm.classification = 'u';            // classification
    omm.ephtype = 0;               // ephtype

    passpredict::Orbit sat2 (omm);
    // Print satrec
    {
        using namespace std;
        cout << endl << "satrec from Omm" << endl;
        cout << "satrec.jdsatepoch = " << sat2.satrec.jdsatepoch << endl;
        cout << "satrec.jdsatepochF = " << sat2.satrec.jdsatepochF << endl;
        cout << "sgp4init epoch = " << (sat2.satrec.jdsatepoch + sat2.satrec.jdsatepochF) - 2433281.5 << endl;
        cout << "satrec.bstar = " << sat2.satrec.bstar << endl;
        cout << "satrec.inclo = " << sat2.satrec.inclo / deg2rad << endl;
        cout << "satrec.nodeo = " << sat2.satrec.nodeo / deg2rad << endl;
        cout << "satrec.ecco = " << sat2.satrec.ecco << endl;
        cout << "satrec.argpo = " << sat2.satrec.argpo / deg2rad << endl;
        cout << "satrec.mo = " << sat2.satrec.mo / deg2rad << endl;
        cout << "satrec.no_kozai = " << sat2.satrec.no_kozai * xpdotp << endl;
        cout << "satrec.revnum = " << sat2.satrec.revnum << endl;
    };

    // Propagate satellite
    {
        char tle1[] = "1 00005U 58002B   00179.78495062  .00000023  00000-0  28098-4 0  4753";
        char tle2[] = "2 00005  34.2682 348.7242 1859667 331.7664  19.3264 10.82419157413667";
        double tsince;
        passpredict::Orbit orbit (tle1, tle2);
        passpredict::Satellite satellite (orbit);
        for (i=0; i<13; i++) {
            tsince = i * 360.0;
            satellite.propagate_tsince(tsince);
            satellite.print_oneline();
            std::cout << std::endl;
        }

    }



    // Find az, el, range of observer


    return 0;
}