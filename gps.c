/*This code demonstrates how to interact with gpsd to fetch GNSS data and process it in real time*/
/*clone the gps.h to use of APIs to interact with GPSD and zmq.h to transmit the GNSS data to other process*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <gps.h>
#include <stdint.h>
#include <syslog.h>

#include <zmq.h>

struct obu_gps
{
    double lat;
    double lon;
    double alt;
    double speed;
    double track;
    time_t timestamp;
};
struct obu_gps data;

int main()
{
    printf("GPS Thread\n");
    static struct gps_data_t gpsdata;

    if (gps_open("localhost", "2947", &gpsdata) != 0)
    {

        fprintf(stderr, "Error connecting to GPSD: %s\n",
                gps_errstr(errno));
        return NULL;
    }

    gps_stream(&gpsdata, WATCH_ENABLE | WATCH_JSON, NULL);

    while (1)
    {

        gps_clear_fix(&gpsdata.fix);
        if (gps_waiting(&gpsdata, 100000)) // 10000 micro seconds = 10ms timeout
        {
            if (gps_read(&gpsdata, NULL, 0) == -1)
            {

                fprintf(stderr, "Error reading GPS data: %s\n",
                        gps_errstr(errno));
            }
            else
            {

                if (gpsdata.fix.mode >= MODE_2D)
                {
                    // printf("%lf, %lf\n", gpsdata.fix.latitude, gpsdata.fix.longitude);
                    
                    time_t now = (time_t) gpsdata.fix.time;
                    struct tm *tm_info = localtime(&now);
                    char time_buffer[80];
                    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
                    data.lat = gpsdata.fix.latitude;
                    data.lon = gpsdata.fix.longitude;
                    data.alt = gpsdata.fix.altitude;
                    data.speed = gpsdata.fix.speed;
                    data.track = gpsdata.fix.track;
                    data.timestamp = now;
                    printf("Latitude is %lf\n",gpsdata.fix.latitude);
                    printf("Longitude is %lf\n",gpsdata.fix.longitude);
                    printf("GPS Altitude: %.2f meters\n", gpsdata.fix.altitude);
                }
                else
                {
                    fprintf(stderr, "No GPS fix available.\n");
                }
            }
        }
        // else
        // {

        //     fprintf(stderr, "No data available within 100ms.\n");
        // }
    }

    gps_stream(&gpsdata, WATCH_DISABLE, NULL);
    gps_close(&gpsdata);
}
