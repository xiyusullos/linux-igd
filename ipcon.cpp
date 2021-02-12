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

#include "ipcon.h"
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>

int get_sockfd()
{
	static int sockfd = -1;
	
	if (sockfd == -1)
	{
		if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1)
		{
			perror("user: socket creating failed");
			return (-1);
		}
	}
	return sockfd;
}

IPCon::IPCon()
{
	m_ifname = new char[18];
}

IPCon::IPCon(char *ifname)
{
	m_ifname = new char[strlen(ifname)+1];
	strcpy (m_ifname,ifname);
}

IPCon::~IPCon()
{
	if (m_ifname)
	{
		delete [] m_ifname;
		m_ifname = NULL;
	}
}

int IPCon::IPCon_SetIfName(char * ifname)
{
	if (strlen(ifname) < sizeof(m_ifname)+1)
	{
		strcpy(m_ifname,ifname);
		return 1;
	}
	else
	{
		m_ifname=NULL;
		return 0;
	}
}

char * IPCon::IPCon_GetIfName(void)
{
	return m_ifname;
}

char * IPCon::IPCon_GetIpAddrStr(void)
{
	struct ifreq ifr;
	struct sockaddr_in *saddr;
	int fd;
	char *address;

	address = new char[20];
	fd = get_sockfd();
	if (fd >= 0 )
	{
		strcpy(ifr.ifr_name, m_ifname);
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
		{
			saddr = (sockaddr_in *)&ifr.ifr_addr;
			strcpy(address,inet_ntoa(saddr->sin_addr));
			return address;
		}
		else
		{
			close(fd);
			return NULL;
		}
	}
	return NULL;
}

int IPCon::IPCon_IsIfUp(void)
{
	struct ifreq ifr;
	int fd;

	fd = get_sockfd();
	if (fd >=0)
	{
		if (strlen(m_ifname) < sizeof(ifr.ifr_name))
			strcpy (ifr.ifr_name, m_ifname);
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
		{
			close(fd);
			return (-1);
		}
		if (ifr.ifr_flags & IFF_UP)
		{
			close(fd);
			return (1);
		}
		else
		{
			close(fd);
			return(0);
		}
	}
	return (-1);
}

char * IPCon::IPCon_GetIfStatStr(void)
{
	if (IPCon_IsIfUp())
		return ("UP");
	else
		return NULL;
}

