using UnityEngine;
using System.Text;
using System.IO.Pipes;

// This class connects via named pipe to the C-program and sends it the EMS values it needs
public class NamedPipeClient : MonoBehaviour
{
    private const string PIPE_NAME = "UnityToCPipe";
    private NamedPipeClientStream pipeClient;

    // The C-program needs to be started first. STart the Unity program second. 
    // Connects Unity via named pipe with the C-program.
    void Start()
    {
        pipeClient = new NamedPipeClientStream(".", PIPE_NAME, PipeDirection.Out);
        try
        {
            pipeClient.Connect(5000); // Wait up to 5 seconds
            Debug.Log("Pipe connection established");
        }
        catch (System.TimeoutException)
        {
            Debug.LogError("Pipe connection timed out");
        }
    }

    // Closes the connection
    void OnDestroy()
    {
        if (pipeClient != null)
        {
            pipeClient.Close();
            pipeClient.Dispose();
        }
    }

    // Sends the data via the named pipe to the C-program.
    public void SendLlConfigParametersWithBaseValue(int microSeconds, float milliAmpere, float baseValue)
    {
        if (pipeClient != null && pipeClient.IsConnected)
        {
            string message = $"{microSeconds},{milliAmpere},{baseValue}\n";
            byte[] buffer = Encoding.ASCII.GetBytes(message);
            pipeClient.Write(buffer, 0, buffer.Length);
            pipeClient.Flush();
        }
        else
        {
            Debug.LogError("Pipe is not connected");
        }
    }
}
