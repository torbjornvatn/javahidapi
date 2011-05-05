
package com.codeminders.ardrone;

import java.io.IOException;
import java.net.*;
import java.util.concurrent.*;
import java.util.logging.Logger;

import com.codeminders.ardrone.commands.FlatTrimCommand;
import com.codeminders.ardrone.commands.QuitCommand;

public class ARDrone
{
    public enum State
    {
        DISCONNECTED, BOOTSTRAP, READY, WATCHDOR, ERROR
    }

    private Logger                              log              = Logger.getLogger("ARDrone");

    private State                               state            = State.DISCONNECTED;

    private static final int                    NAVDATA_PORT     = 5554;
    private static final int                    VIDEO_PORT       = 5555;
    private static final int                    CONTROL_PORT     = 5559;

    private static byte[]                       DEFAULT_DRONE_IP = { (byte) 192, (byte) 168, (byte) 1, (byte) 1 };

    private InetAddress                         drone_addr;
    private DatagramSocket                      video_socket;
    private DatagramSocket                      cmd_socket;
    private Socket                              control_socket;

    private PriorityBlockingQueue<DroneCommand> cmd_queue        = new PriorityBlockingQueue<DroneCommand>();
    private BlockingQueue<NavData>              navdata_queue    = new LinkedBlockingQueue<NavData>();

    private NavDataReader                       nav_data_reader;
    private CmdSender                           cmd_sender;

    private Thread                              nav_data_reader_thread;
    private Thread                              cmd_sending_thread;

    public ARDrone() throws UnknownHostException
    {
        this(InetAddress.getByAddress(DEFAULT_DRONE_IP));
    }

    public ARDrone(InetAddress drone_addr)
    {
        this.drone_addr = drone_addr;
    }

    private void changeState(State newstate)
    {
        if(newstate == State.ERROR)
            changeToErrorState(null);

        synchronized(state)
        {
            log.fine("State changed from " + state + " to " + newstate);
            state = newstate;
        }
    }

    public void changeToErrorState(Exception ex)
    {
        synchronized(state)
        {
            try
            {
                if(state != State.DISCONNECTED)
                    doDisconnect();
            } catch(IOException e)
            {
                // Ignoring exceptions on disconnection
            }
            log.fine("State changed from " + state + " to " + State.ERROR + " with exception " + ex);
            state = State.ERROR;
        }
    }

    public void connect() throws IOException
    {
        try
        {
            video_socket = new DatagramSocket(VIDEO_PORT);
            cmd_socket = new DatagramSocket();
            // control_socket = new Socket(drone_addr, CONTROL_PORT);

            nav_data_reader = new NavDataReader(this, drone_addr, NAVDATA_PORT);
            nav_data_reader_thread = new Thread(nav_data_reader);
            nav_data_reader_thread.start();

            cmd_sender = new CmdSender(cmd_queue, this, drone_addr, cmd_socket);
            cmd_sending_thread = new Thread(cmd_sender);
            cmd_sending_thread.start();

            initBootstrap();

        } catch(IOException ex)
        {
            changeToErrorState(ex);
            throw ex;
        }
    }


    /**
     * Sends some random bytes to drone NAVDATA_PORT to initiate bootstrap mode.
     * @throws IOException
     */
    private void initBootstrap() throws IOException
    {
        DatagramSocket socket = new DatagramSocket(NAVDATA_PORT);
        try
        {
            byte[] buf = {0x01, 0x00, 0x00, 0x00};
            DatagramPacket packet = new DatagramPacket(buf, buf.length, drone_addr, NAVDATA_PORT);
            socket.send(packet);
        }
        finally
        {
            socket.close();
        }
        changeState(State.BOOTSTRAP);
    }

    public void disconnect() throws IOException
    {
        try
        {
            doDisconnect();
        } finally
        {
            changeState(State.DISCONNECTED);
        }
    }

    private void doDisconnect() throws IOException
    {
        cmd_queue.add(new QuitCommand());
        nav_data_reader.stop();
        cmd_socket.close();
        video_socket.close();

        // Only the following method can throw an exception.
        // We call it last, to ensure it won't prevent other
        // cleanup operations from being completed
        // control_socket.close();
    }

    public void trim() throws IOException
    {
        cmd_queue.add(new FlatTrimCommand());
    }

    public void takeOff() throws IOException
    {
    }

    public void land() throws IOException
    {
    }

    public void sendEmergencySignal() throws IOException
    {
    }

    public void clearEmergencySignal() throws IOException
    {
    }

    public void hover() throws IOException
    {
    }

    public void setCombinedYawMode(boolean v)
    {
    }

    public boolean isCombinedYawMode()
    {
        return false;
    }

    public void set(float phi, float theta, float gaz, float yaw) throws IOException
    {
    }

    public void sendAllNavigationData() throws IOException
    {
    }

    public void sendADemoNavigationData() throws IOException
    {
    }

    public void setConfigOption(String name, String value) throws IOException
    {
    }

    public void playLED(int animation_no, float freq, int duration) throws IOException
    {
    }

    public void playAnimation(int animation_no, int duration) throws IOException
    {
    }

    // Callback used by receiver
    public void navDataReceived(NavData nd)
    {
        if(state == State.READY)
        {
            navdata_queue.add(nd);
        } else
        {
            // TODO:
        }
    }
}
