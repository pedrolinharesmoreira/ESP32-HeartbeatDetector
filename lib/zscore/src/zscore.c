#include "zscore.h"
#include <math.h>
#define BUFFER_SIZE 1500


int lag, samples, k;
double threshold, influence;
double y[BUFFER_SIZE];
double filteredY[BUFFER_SIZE];
double signals[BUFFER_SIZE];
double avgFilter[BUFFER_SIZE];
double stdFilter[BUFFER_SIZE];

//Helper functions
double mean(double data[], int len);
double std(double data[], int len);
double fabs(double val);
void resetVectors(void)
{
    int i;
    for(i = 0; i < BUFFER_SIZE; i++)
    {
        signals[i] = 0.0;
        avgFilter[i] = 0.0;
        stdFilter[i] = 0.0;
        filteredY[i] = 0;
    }
    avgFilter[lag-1] = mean(y, lag);
    stdFilter[lag-1] = std(y, lag);
}

/**
 * 
 * Initializes the z-score library
 * @param _lag Lag of the moving window
 * @param _threshold Threshold for signaling (in standard deviations)
 * @param _influence Influence of new samples on the standard deviation and mean
 * 
 */
void zscore_init(int _lag, double _threshold, double _influence)
{
    samples = 0;
    lag = _lag;
    threshold = _threshold;
    influence = _influence;
    resetVectors();
}

/**
 * 
 * Process the incoming value
 * @param data The sample received from the sensor to be processed.
 * @returns Smoothed Z-score for the given sample.
 * 
 */
double zscore_process(int data)
{
    y[samples] = (double)data;
    k = samples++;
    if(k < lag)
    {
        return 0;
    }
    else if(k == lag)
    {
        resetVectors();
        return 0;
    }
    signals[k] = 0;
    filteredY[k] = 0;
    avgFilter[k] = 0;
    stdFilter[k] = 0;

    if(fabs(y[k] - avgFilter[k-1]) > threshold * stdFilter[k - 1])
    {
        if(y[k] > avgFilter[k-1])
        {
            signals[k] = 1;
        }else {
            signals[k] = -1;
        }
        filteredY[k] = influence * y[k] + (1-influence)*filteredY[k-1];
        avgFilter[k] = mean(filteredY + k-lag, lag);
        stdFilter[k] = std(filteredY + k-lag, lag);
    }else
    {
        signals[k] = 0;
        filteredY[k] = y[k];
        avgFilter[k] = mean(filteredY + k-lag, lag);
        stdFilter[k] = std(filteredY + k-lag, lag);
    }
    if(samples > BUFFER_SIZE)
    {
        resetVectors();
        samples = 0;
    }
    return signals[k];
}


/**
 * 
 * Calculates the mean value of the given array of samples
 * @param data array of values
 * @param len length of the array
 * @returns the standard deviation
 * 
 */
double mean(double data[], int len)
{
    double sum = 0.0, mean = 0.0;
    int i;
    for (i = 0; i < len; i++)
    {
        sum+=data[i];
    }
    mean = sum/(double)len;
    return mean;
}


/**
 * 
 * Calculates the standard deviation of the given array of samples
 * @param data array of values
 * @param len length of the array
 * @returns the standard deviation
 * 
 */
double std(double data[], int len)
{
    double tmean = mean(data,len);
    double std = 0.0;
    int i;
    for(i=0; i < len; i++)
    {
        std+=pow(data[i] - tmean, 2);
    }
    return sqrt(std/(double)len);
}

/**
 * 
 * Calculates the absolute value of the number given
 * @param val The value
 * @returns the absolute value of the number given
 * 
 */
double fabs(double val)
{
    if (val < 0) return -val;
    return val;
}