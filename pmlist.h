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

#ifndef _PMLIST_H_
#define _PMLIST_H_

#include "portmap.h"
#include <list>
#include <syslog.h>

using namespace std;

class PortMapList
{
	public:
		PortMapList();
		~PortMapList();

		int PortMapAdd(char *RemoteHost, char *Proto, char *ExtIP, int ExtPort,
				char *IntIP, int IntPort, int Enabled, char *Desc, int LeaseDuration);
		int PortMapDelete(char  *Proto, int ExtPort);
		int addPacketFilter(char *Proto, char *SrcIP, char *DestIP, int DestPort, 
			int Enabled, char *Desc);
		int addPortForward(char *Proto, char *ExtIP, int ExtPort, char *IntIP,
			 int IntPort, int Enabled, char *Desc);
		int delPacketFilter(char *Proto, char *SrcIP, char *DestIP, int DestPort);
		int delPortForward(char *Proto, char *ExtIP, int ExtPort, char* IntIP, int IntPort);

	list <PortMap *> m_pmap;
};

#endif
