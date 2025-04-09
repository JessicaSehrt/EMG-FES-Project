using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using System.Linq;

// This classe receives an array of EMG-values and converts them to mA and 탎 which can be used to generate EMS signals
public class EMG_Processor : MonoBehaviour
{
    public enum Mode
    {
        CONSTANT_PULSEWIDTH,
        CONSTANT_CURRENT
    }

    [SerializeField] private NamedPipeClient namedPipeClient;
    [SerializeField] private CSVReader csvReader;

    [SerializeField] private Mode mode = Mode.CONSTANT_PULSEWIDTH;

    [SerializeField] private int CURRENT_MIN = 5; // in mA
    [SerializeField] private int CURRENT_MAX = 10; // in mA

    [SerializeField] private int PULSEWIDTH_MIN = 50; // in 탎
    [SerializeField] private int PULSEWIDTH_MAX = 100; // in 탎

    // Either the current or the pulsewidth will be modulated after the EMG. The other value will be kept constant.
    [SerializeField] private int CURRENT_CONSTANT = 15; // in mA
    [SerializeField] private int PULSEWIDTH_CONSTANT = 150; // in 탎 

    // assuming 16 bit emg data. The data oscillates around 2^15.
    // We will transform values below 2^15 such that they are above it while keeping the same absolute difference.
    [SerializeField] private int EMG_RANGE_MIN = 32768;
    [SerializeField] private int EMG_RANGE_MAX = 65536;

    private float calibratedEmgMax = 0;
    private int calibrationPassesLeft = 5;
    private bool isEmgCalibrated = false;

    private int dataCounter = 0;
    private List<float> data = new List<float>();


    private void Start()
    {
        if (csvReader != null)
        {
            csvReader.OnDataReceived += HandleDataRead;
        }
    }

    private void OnDestroy()
    {
        if (csvReader != null)
        {
            csvReader.OnDataReceived -= HandleDataRead;
        }
    }

    private void HandleDataRead(float rawData)
    {
        UpdateData(rawData);
        if (!isEmgCalibrated)
        {
            CalibrateEMGMax();
        }

        else if (data.Count >= 50)
        {
            ConvertEMGValuesToEMSValues();
        }
    }

    // Calibrate the maximum read EMG value.
    private void CalibrateEMGMax()
    {
        calibratedEmgMax = data.Max();
        calibrationPassesLeft--;
        if (calibrationPassesLeft == 0)
        {
            isEmgCalibrated = true;
        }
        
    }

    // Ensure the data is in the correct range before updating the data Arraylist
    private void UpdateData(float rawData)
    {
        float maxValue = isEmgCalibrated ? calibratedEmgMax : EMG_RANGE_MAX;
        
        if (data.Count >= 50)
        {
            data.Clear();
        }

        if (rawData >= EMG_RANGE_MIN && rawData <= maxValue)
        {
            data.Add(rawData);
        }

        else if (rawData < EMG_RANGE_MIN)
        {
            data.Add((2 * EMG_RANGE_MIN) - rawData);
        }

        else
        {
            data.Add(maxValue);
        }
    }

    // Uses the average of the passed values to determine the correct EMS values and sends them to the C-program
    public void ConvertEMGValuesToEMSValues()
    {
        // we get a new data point every ms. Skipping the first 20 entries ensures that we do not pick up any noice 
        // cause by the last ems impulse.
        var last30 = data.Skip(Mathf.Max(0, data.Count - 30)).ToList();
        float average = CalculateMedian(last30);

        // Only send an EMS signal if the EMG average passes a threshhold
        float emgPercent = NormalizeRange(average, EMG_RANGE_MIN, calibratedEmgMax, 0, 1);
        float threshhold = 0.2f;
        if (emgPercent >= threshhold)
        {
            // The calculated mA and 탎 values are proportional to the EMG.
            // The threshold equals the min current / pulsewidth and
            // 50000 (max callibrated EMG) equals the max current / pulsewidth
            float scaledEmgPercent = NormalizeRange(emgPercent, 0, 1, threshhold, 1);

            if (mode == Mode.CONSTANT_PULSEWIDTH)
            {
                float milliAmpere = NormalizeRange(scaledEmgPercent, threshhold, 1, CURRENT_MIN, CURRENT_MAX);
                namedPipeClient.SendLlConfigParametersWithBaseValue(PULSEWIDTH_CONSTANT, milliAmpere, average);
            }
            else if (mode == Mode.CONSTANT_CURRENT)
            {
                int pulseWidth = (int)NormalizeRange(scaledEmgPercent, threshhold, 1, PULSEWIDTH_MIN, PULSEWIDTH_MAX);
                namedPipeClient.SendLlConfigParametersWithBaseValue(pulseWidth, CURRENT_CONSTANT, average);
            }
        }
        
    }

    // takes a value between originalMin and originalMax and return a new value between targetMin and targetMax that has the 
    // same distance to its limits as the passed value has to its limits
    private float NormalizeRange(float value, float originalMin, float originalMax, float targetMin, float targetMax)
    {
        float normalizedValue = Mathf.InverseLerp(originalMin, originalMax, value);
        float targetValue = Mathf.Lerp(targetMin, targetMax, normalizedValue);
        return targetValue;
    }

    private float CalculateMedian(List<float> values)
    {
        values.Sort();
        int count = values.Count;

        if (count % 2 == 0)
        {
            return (values[(count / 2) - 1] + values[count / 2]) / 2f;
        }

        else
        {
            return values[count / 2];
        }


    }
}
