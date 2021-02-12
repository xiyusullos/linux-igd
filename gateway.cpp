/******************************************************************************
*  Copyright (c) 2002 Linux UPnP Internet Gateway Device Project              *    
*  All rights reserved.                                                       *
*                                                                             *   
*  This file is part of The Linux UPnP Internet Gateway Device (IGD).         *
*                                                                             *
*  The Linux UPnP IGD is free software; you can redistribute it and/or modify *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *    
*  The Linux UPnP IGD is distributed in the hope that it will be useful,      *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *   
*  You should have received a copy of the GNU General Public License          * 
*  along with Foobar; if not, write to the Free Software                      *
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  *
*                                                                             *  
*                                                                             *  
******************************************************************************/
// 
// Special thanks to Genmei Mori and his team for the work he done on the
// ipchains version of this code. .
//

#include <time.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <upnp/upnp.h>
#include "gateway.h"
#include "ipcon.h"
#include "sample_util.h"
#include "portmap.h"
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pmlist.h"
#include "gate.h"
#include <sys/types.h>
#include <sys/stat.h>

// The global GATE object
Gate gate;

// Callback Function wrapper.  This is needed because ISO forbids a pointer to a bound
// member function.  This corrects the issue.
int GateDeviceCallbackEventHandler(Upnp_EventType EventType, void *Event, void *Cookie)
{
	return gate.GateDeviceCallbackEventHandler(EventType, Event, Cookie);
}

int substr(char *docpath, char *infile, char *outfile, char *str_from, char *str_to);

int main (int argc, char** argv)
{
	char *desc_doc_name=NULL, *conf_dir_path=NULL;
	char lan_ip_address[16];
	char desc_doc_url[200];
	int sig;
	sigset_t sigs_to_catch;
	int port;
	int ret;
	char *address;
	pid_t pid,sid;

	// Log startup of daemon
	syslog(LOG_INFO, "The Linux UPnP Internet Gateway Device Ver 0.92 by Dime (dime@gulfsales.com)");
	syslog(LOG_INFO, "Special Thanks for Intel's Open Source SDK and original author Genmei Mori's work.");
	
	if (argc != 3)
	{
		cout << "Usage: upnpd <external ifname> <internal ifname>" <<endl;
		cout << "Example: upnpd ppp0 eth0 " << endl;
		cout << "Example: upnpd eth1 eth0 " << endl;
		exit(0);
	}
	
	pid = fork();
        if (pid < 0)
        {
                perror("Error forking a new process.");
                exit(EXIT_FAILURE);
        }
        if (pid > 0)
                exit(EXIT_SUCCESS);

        if ((sid = setsid()) < 0)
        {
                perror("Error running setsid");
                exit(EXIT_FAILURE);
        }
        if ((chdir("/")) < 0)
        {
                perror("Error setting root directory");
                exit(EXIT_FAILURE);
        }

        umask(0);

        //close (STDOUT_FILENO);
        close (STDERR_FILENO);

	gate.m_ipcon = new IPCon(argv[2]);
	address = gate.m_ipcon->IPCon_GetIpAddrStr();
	strcpy(lan_ip_address, address);
	if (address) delete [] address;
	delete gate.m_ipcon;
	
	port = INIT_PORT;
	desc_doc_name=INIT_DESC_DOC;
	conf_dir_path=INIT_CONF_DIR;

	sprintf(desc_doc_url, "http://%s:%d/%s.xml", lan_ip_address, port,desc_doc_name);
   	syslog(LOG_DEBUG, "Intializing UPnP with desc_doc_url=%s\n",desc_doc_url);
        syslog(LOG_DEBUG, "ipaddress=%s port=%d\n", lan_ip_address, port);
	syslog(LOG_DEBUG, "conf_dir_path=%s\n", conf_dir_path);
        substr(conf_dir_path, "gatedesc.skl", "gatedesc.xml", "!ADDR!",lan_ip_address);

	if ((ret = UpnpInit(lan_ip_address, port)) != UPNP_E_SUCCESS)
	{
		syslog(LOG_ERR, "Error with UpnpInit -- %d\n", ret);
		UpnpFinish();
		exit(1);
	}
	syslog(LOG_DEBUG, "UPnP Initialization Completed");
	
	syslog(LOG_DEBUG, "Setting webserver root directory -- %s\n",conf_dir_path);
	if ((ret = UpnpSetWebServerRootDir(conf_dir_path)) != UPNP_E_SUCCESS)
	{
		syslog(LOG_ERR, "Error setting webserver root directory -- %s: %d\n",
			       	conf_dir_path, ret);
		UpnpFinish();
		exit(1);
	}
	gate.m_ipcon = new IPCon(argv[1]);
	syslog(LOG_DEBUG, "Registering the root device\n");
	if ((ret = UpnpRegisterRootDevice(desc_doc_url, GateDeviceCallbackEventHandler,
				&gate.device_handle, &gate.device_handle)) != UPNP_E_SUCCESS)
	{
		syslog(LOG_ERR, "Error registering the rootdevice : %d\n", ret);
		UpnpFinish();
		exit(1);
	}
	else
	{
		syslog(LOG_DEBUG, "RootDevice Registered\n");
		syslog(LOG_DEBUG, "Initializing State Table\n");
		gate.GateDeviceStateTableInit(desc_doc_url);
		syslog(LOG_DEBUG, "State Table Initialized\n");

		if ((ret = UpnpSendAdvertisement(gate.device_handle, 1800))
			!= UPNP_E_SUCCESS)
		{
			syslog(LOG_ERR, "Error sending advertisements : %d\n", ret);
			UpnpFinish();
			exit(1);
		}

		syslog(LOG_DEBUG, "Advertisements Sent\n");
	}
	
	gate.startup_time = time(NULL);
	
	//Start The Command Loop

	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGINT);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigwait(&sigs_to_catch, &sig);
	syslog(LOG_INFO, "Shutting down on signal %d...\n", sig);

	UpnpUnRegisterRootDevice(gate.device_handle);
	UpnpFinish();

	exit(0);

}	

int substr(char *docpath, char *infile, char *outfile, char *str_from, char *str_to)
{
	FILE *fpi, *fpo;
	char pathi[256], patho[256];
	char buffi[4096], buffo[4096];
	int len_buff, len_from, len_to;
	int i, j;

	sprintf(pathi, "%s%s", docpath, infile);
	if ((fpi = fopen(pathi,"r")) == NULL) 
	{
		printf("input file can not open\n");
		return (-1);
	}

	sprintf(patho, "%s%s", docpath, outfile);
	if ((fpo = fopen(patho,"w")) == NULL) {
		printf("output file can not open\n");
		fclose(fpi);
		return (-1);
	}

	len_from = strlen(str_from);
	len_to   = strlen(str_to);

	while (fgets(buffi, 4096, fpi) != NULL) 
	{
		len_buff = strlen(buffi);
		for (i=0, j=0; i <= len_buff-len_from; i++, j++) 
		{
			if (strncmp(buffi+i, str_from, len_from)==0) 
			{
				strcpy (buffo+j, str_to);
				i += len_from - 1;
				j += len_to - 1;
			} else
			*(buffo + j) = *(buffi + i);
		}
		strcpy(buffo + j, buffi + i);
		fputs(buffo, fpo);
	}

	fclose(fpo);
	fclose(fpi);
	return (0);
}

