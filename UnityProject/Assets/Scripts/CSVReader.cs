using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System;
using System.Diagnostics;

public class CSVReader : MonoBehaviour
{
    [SerializeField] private string delimiter = ";";
    [SerializeField] private string filePath = "Assets/EMG-data/emg_data.csv";
    [SerializeField] private double targetMilliSeconds = 1.0; // 1ms equals a emg polling rate of 1000Hz

    private int currentIndex = 0;
    private List<string[]> allData = new List<string[]>();

    private Stopwatch stopwatch;
    public event Action<float> OnDataReceived;

    private void Start()
    {
        LoadCSV();
        stopwatch = new Stopwatch();
        StartCoroutine(ProcessDataLineByLine());
    }

    void LoadCSV()
    {
        try
        {
            string[] lines = File.ReadAllLines(filePath);

            for (int i = 1; i < lines.Length; i++)
            {
                string[] values = lines[i].Split(delimiter);
                allData.Add(values);
            }

            UnityEngine.Debug.Log("Loaded " + allData.Count + " rows from csv");
        }

        catch (Exception e)
        {
            UnityEngine.Debug.LogError("Error loading CSV: " + e.Message);
        }
    }

    /*private IEnumerator ProcessDataLineByLine()
    {
        while (currentIndex < allData.Count)
        {
            ProcessRow(allData[currentIndex]);
            currentIndex++;

            yield return new WaitForSeconds(readInterval);
        }
    }*/

    private IEnumerator ProcessDataLineByLine()
    {
        stopwatch.Start();
        double lastElapsedMicroseconds = 0;

        while (currentIndex < allData.Count)
        {
            double currentElapsedMicroseconds = stopwatch.Elapsed.TotalMilliseconds;
            double deltaTime = currentElapsedMicroseconds - lastElapsedMicroseconds;

            if (deltaTime >= targetMilliSeconds)
            {
                int rowsToProcess = (int)(deltaTime / targetMilliSeconds);
                
                for (int i = 0; i < rowsToProcess && currentIndex < allData.Count; i++)
                {
                    ProcessRow(allData[currentIndex]);
                    currentIndex++;
                    lastElapsedMicroseconds += targetMilliSeconds;

                }
            }
            yield return null;
        }

        stopwatch.Stop();
    }



    private void ProcessRow(string[] rowData)
    {
        if (rowData.Length < 2)
        {
            UnityEngine.Debug.LogError("CSV file doesn't have at least two columns");
            return;
        }

        string rawData = rowData[1];
        if (float.TryParse(rawData, out float rawDataValue))
        {
            OnDataReceived?.Invoke(rawDataValue);
        }
        
    }
}
