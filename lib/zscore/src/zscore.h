#ifndef ZSCORE_H
#define ZSCORE_H

/*
Library created based on resources found on:
    * https://stackoverflow.com/questions/22583391/peak-signal-detection-in-realtime-timeseries-data/22640362#22640362
    * https://turi.com/learn/userguide/anomaly_detection/moving_zscore.html
*/

void zscore_init(int _lag, double _threshold, double _influence);
double zscore_process(int data);

#endif