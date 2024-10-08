package com.example.javainterface;

import android.content.Intent;
import android.net.VpnService;
import android.os.ParcelFileDescriptor;
import java.io.IOException;

public class MyVpnService extends VpnService
{
    private ParcelFileDescriptor vpnInterface = null;

    // Start the VPN service
    public void startVpn()
    {
        Builder builder = new Builder();
        // Configure VPN address and routing
        builder.setSession("MyVPN")
                .addAddress("10.0.0.2", 24)     // Local VPN IP address
                .addRoute("0.0.0.0", 0)         // Route all traffic
                .addDnsServer("8.8.8.8");                  // DNS server (Google DNS for testing)

        // Establish VPN connection
        vpnInterface = builder.establish();
    }
    @Override
    public int onStartCommand(Intent intent, int flags, int startID)
    {
        startVpn();
        return START_STICKY;
    }
    @Override
    public void onDestroy()
    {
        if (vpnInterface != null)
        {
            try
            {
                vpnInterface.close();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
            vpnInterface = null;
        }
        super.onDestroy();
    }
}