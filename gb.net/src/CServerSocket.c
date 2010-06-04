/***************************************************************************

  CServerSocket.c

  (c) 2003-2004 Daniel Campos Fernández <dcamposf@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CSERVERSOCKET_C


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>

#include "main.h"
#include "tools.h"

#include "CServerSocket.h"
#include "CSocket.h"



DECLARE_EVENT(EVENT_Connection);
DECLARE_EVENT(EVENT_Error);

void srvsock_post_error(CSERVERSOCKET *_object)
{
	GB.Raise(THIS, EVENT_Error, 0);
	GB.Unref(POINTER(&_object));
}

// BM: Fix bug in allocations

void CServerSocket_NewChild(CSERVERSOCKET *_object, void *cli_obj)
{
	if (THIS->nchildren++)
		GB.Realloc ( POINTER(&THIS->children),THIS->nchildren * sizeof(*THIS->children));
	else
		GB.Alloc   ( POINTER(&THIS->children),THIS->nchildren * sizeof(*THIS->children));

	THIS->children[THIS->nchildren-1]=cli_obj;
}

void CServerSocket_DeleteChild(CSERVERSOCKET *_object, void *cli_obj)
{
	int myloop;
	int myloop2;

	if (!THIS->nchildren) return;

	for (myloop=0;myloop<THIS->nchildren;myloop++)
	{
		if (THIS->children[myloop]==cli_obj)
		{
			for (myloop2=myloop;myloop2<(THIS->nchildren-1);myloop2++)
				THIS->children[myloop2]=THIS->children[myloop2+1];
			if ( --THIS->nchildren)
			{
				GB.Realloc ( POINTER(&THIS->children),THIS->nchildren * sizeof(*THIS->children));
			}
			else
			{
				GB.Free (POINTER(&THIS->children));
				THIS->children=NULL;
			}
			return;
		}
	}
}

void CServerSocket_OnClose(void *sck)
{
	CSOCKET *chd=(CSOCKET*)sck;
	CSERVERSOCKET *_object;

	if (!chd) return;
	if (!chd->c_parent) return;
	CServerSocket_DeleteChild(chd->c_parent,sck);
	_object=(CSERVERSOCKET*)chd->c_parent;
	THIS->iCurConn--;

}

void CServerSocket_CallBack(int fd, int type, intptr_t lParam)
{
	int okval=0;
	char *rem_ip_buf;
	unsigned int ClientLen;
	CSERVERSOCKET *_object = (CSERVERSOCKET*)lParam;
	
	if ( SOCKET->status != 1) return;

	SOCKET->status=2;
	ClientLen=sizeof(struct sockaddr_in);
	THIS->Client=accept(SOCKET->socket,(struct sockaddr*)&THIS->so_client.in,&ClientLen);
	if (THIS->Client == -1)
	{
		close(THIS->Client);
		SOCKET->status=1;
		return;
	}
	rem_ip_buf=inet_ntoa(THIS->so_client.in.sin_addr);
	if ( (!THIS->iMaxConn) || (THIS->iCurConn < THIS->iMaxConn) ) okval=1;
	if ( (!THIS->iPause) && (okval) )
		GB.Raise(THIS,EVENT_Connection,1,GB_T_STRING,rem_ip_buf,0);
	if  ( SOCKET->status == 2) close(THIS->Client);
	SOCKET->status=1;
}

void CServerSocket_CallBackUnix(int fd, int type, intptr_t lParam)
{
	//int position=0;
	int okval=0;
	unsigned int ClientLen;
	CSERVERSOCKET *_object = (CSERVERSOCKET*)lParam;
	
	if ( SOCKET->status != 1) return;

	SOCKET->status=2;
	ClientLen=sizeof(struct sockaddr_un);
	THIS->Client=accept(SOCKET->socket,(struct sockaddr*)&THIS->so_client.un,&ClientLen);
	if (THIS->Client == -1)
	{
		close(THIS->Client);
		SOCKET->status=1;
		return;
	}
	if ( (!THIS->iMaxConn) || (THIS->iCurConn < THIS->iMaxConn) ) okval=1;
	if ( (!THIS->iPause) && (okval) )
		GB.Raise(THIS,EVENT_Connection,1,GB_T_STRING,NULL,0);
	if  ( SOCKET->status == 2) close(THIS->Client);
	SOCKET->status=1;

}


/***************************************************
 This property reflects current status of the
 socket (closed, listening...)
 ***************************************************/
BEGIN_PROPERTY ( CSERVERSOCKET_Status )

	GB.ReturnInteger(SOCKET->status);

END_PROPERTY

/******************************************************************
 This property gets/sets the port to listen to (TCP or UDP sockets)
 ******************************************************************/
BEGIN_PROPERTY ( CSERVERSOCKET_Port )

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->iPort);
		return;
	}
	if (SOCKET->status>0)
	{
		GB.Error("Port value can not be changed when socket is active");
		return;
	}
	if ( (VPROP(GB_INTEGER)<1) || (VPROP(GB_INTEGER)>65535) )
	{
		GB.Error("Invalid Port Value");
		return;
	}
	THIS->iPort=VPROP(GB_INTEGER);

END_PROPERTY


/***************************************************************
 This property gets/sets the address to listen to (UNIX socket)
 ***************************************************************/
BEGIN_PROPERTY ( CSERVERSOCKET_Path )

	char *tmpstr=NULL;

	if (READ_PROPERTY)
	{
		GB.ReturnString(THIS->sPath);
		return;
	}
	if (SOCKET->status>0)
	{
		GB.Error("Path value can not be changed when socket is active");
		return;
	}
	tmpstr=GB.ToZeroString ( PROP(GB_STRING) );
	if ( (strlen(tmpstr)<1) || (strlen(tmpstr)>NET_UNIX_PATH_MAX) )
	{
		GB.Error ("Invalid path length");
		return;
	}
	GB.StoreString(PROP(GB_STRING), &THIS->sPath);

END_PROPERTY


/***************************************************************
 This property gets/sets the socket type (0 -> TCP, 1 -> UNIX)
 ***************************************************************/
BEGIN_PROPERTY ( CSERVERSOCKET_Type )

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(THIS->type);
		return;
	}
	if (SOCKET->status>0)
	{
		GB.Error("Socket Type can not be changed when socket is active");
		return;
	}
	if ( (VPROP(GB_INTEGER)<0) || (VPROP(GB_INTEGER)>1) )
	{
		GB.Error("Invalid Socket Type Value");
		return;
	}
	THIS->type=VPROP(GB_INTEGER);

END_PROPERTY
/***********************************************
 Gambas object "Constructor"
 ***********************************************/
BEGIN_METHOD(CSERVERSOCKET_new,GB_STRING sPath;GB_INTEGER iMaxConn;)

	int retval;
	char *buf=NULL;
	int nport=0;
	int iMax=0;

	THIS->iPort=0;
	SOCKET->status=0;
	THIS->sPath=NULL;
	THIS->iPause=0;
	THIS->iMaxConn=0;
	THIS->iCurConn=0;
	THIS->type=1;
	THIS->children=NULL;
	THIS->nchildren=0;

	if (MISSING(sPath)) return;
	if (!STRING(sPath)) return;

	if (!MISSING(iMaxConn)) iMax=VARG(iMaxConn);
	retval=IsHostPath(STRING(sPath),&buf,&nport);
	if (!retval)
	{
		GB.Error("Invalid Host / Path string");
		return;
	}
	if (retval==2)
	{
		THIS->type=0;
		buf=GB.ToZeroString ( (GB_STRING*)STRING(sPath) );
		if ( (strlen(buf)<1) || (strlen(buf)>NET_UNIX_PATH_MAX) )
		{
			GB.Error ("Invalid path length");
			return;
		}
		GB.StoreString((GB_STRING*)STRING(sPath),&THIS->sPath);
		return;
	}
	else
	{
		if (buf)
		{
			GB.Free(POINTER(&buf));
			GB.Error("Invalid Host String");
			return;
		}
		if (nport<1)
		{
			GB.Error("Invalid Port value");
			return;
		}

		THIS->type=1;
		THIS->iPort=nport;
	}

	switch(srvsock_listen(THIS,iMax))
	{
		case 1:
			GB.Error("Socket is already listening");
			return;
		case 7:
			GB.Error ("You must define Path");
			return;
		case 8:
			GB.Error ("Error. You must define port");
			return;
		case 13:
			GB.Error ("Invalid maximun connections value");
			return;
	}


END_METHOD



void close_server(CSERVERSOCKET *_object)
{
	CSOCKET *chd;

	if (SOCKET->status <= 0) return;

	GB.Watch (SOCKET->socket , GB_WATCH_NONE , (void *)CServerSocket_CallBack,0);
	close(SOCKET->socket);
	SOCKET->status=0;

	if (!THIS->nchildren) return;

	while(THIS->nchildren)
	{
		chd=(CSOCKET*)THIS->children[0];
		if (chd->common.stream.desc) CSocket_stream_close(&chd->common.stream);
		CServerSocket_DeleteChild(THIS,(void*)chd);
	}

}
/*************************************************
 Gambas object "Destructor"
 *************************************************/
BEGIN_METHOD_VOID(CSERVERSOCKET_free)


	close_server(THIS);
	GB.FreeString(&THIS->sPath);

END_METHOD

/********************************************************
 Stops listening (TCP/UDP/UNIX), and closes all client sockets associated
 to this server
 *********************************************************/
BEGIN_METHOD_VOID(CSERVERSOCKET_Close)

	close_server(THIS);

END_METHOD

/********************************************************
 Do not accept more connections until Resume is used
 *********************************************************/
BEGIN_METHOD_VOID(CSERVERSOCKET_Pause)

	THIS->iPause=1;

END_METHOD

/********************************************************
 Accept connections again
 *********************************************************/
BEGIN_METHOD_VOID(CSERVERSOCKET_Resume)

	THIS->iPause=0;

END_METHOD

/*********************************************************
 Starts listening (TCP/UDP/UNIX)
 **********************************************************/
int srvsock_listen(CSERVERSOCKET* _object,int mymax)
{
	int NoBlock=1;
	int retval;
	int auth = 1;

	if ( (!THIS->iPort) && (THIS->type) ) return 8;

	if ( SOCKET->status >0 ) return 1;

	if (mymax<0) return 13;

	if ( (!THIS->type) && (!THIS->sPath) ) return 7;


	if (THIS->type)
	{
		THIS->so_server.in.sin_family=AF_INET;
		THIS->so_server.in.sin_addr.s_addr=INADDR_ANY;
		THIS->so_server.in.sin_port=htons(THIS->iPort);
		SOCKET->socket=socket(PF_INET,SOCK_STREAM,0);
	}
	else
	{
		unlink(THIS->sPath);
		THIS->so_server.un.sun_family=AF_UNIX;
		strcpy(THIS->so_server.un.sun_path,THIS->sPath);
		SOCKET->socket=socket(AF_UNIX,SOCK_STREAM,0);
	}

	if ( SOCKET->socket==-1 )
	{
		SOCKET->status=-2;
		GB.Ref(THIS);
		GB.Post(srvsock_post_error,(intptr_t)THIS);
		return 2;
	}
	// thanks to Benoit : this option allows non-root users to reuse the
	// port after closing it and reopening it in a short interval of time.
	// However, If you are porting this component to other O.S., be careful,
	// as this is not a standard unix option
	setsockopt(SOCKET->socket, SOL_SOCKET, SO_REUSEADDR, &auth, sizeof(int));
	SOCKET_update_timeout(SOCKET);
	//
	if (THIS->type)
		retval=bind(SOCKET->socket,(struct sockaddr*)&THIS->so_server.in, \
			sizeof(struct sockaddr_in));
	else
		retval=bind(SOCKET->socket,(struct sockaddr*)&THIS->so_server.un, \
			sizeof(struct sockaddr_un));
	if (retval==-1)
	{
		close(SOCKET->socket);
		SOCKET->status=-10;
		GB.Ref(THIS);
		GB.Post(srvsock_post_error,(intptr_t)THIS);
		return 10;
	}

	ioctl(SOCKET->socket,FIONBIO,&NoBlock);

	if ( listen(SOCKET->socket,mymax) == -1 )
	{
		close(SOCKET->socket);
		SOCKET->status=-14;
		GB.Ref(THIS);
		GB.Post(srvsock_post_error,(intptr_t)THIS);
		return 14;
	}
	THIS->iCurConn=0;
	THIS->iMaxConn=mymax;
	SOCKET->status=1;

	//CServerSocket_AssignCallBack((intptr_t)THIS,SOCKET->socket);
	if (THIS->type)
		GB.Watch (SOCKET->socket , GB_WATCH_READ , (void *)CServerSocket_CallBack,(intptr_t)THIS);
	else
		GB.Watch (SOCKET->socket , GB_WATCH_READ , (void *)CServerSocket_CallBackUnix,(intptr_t)THIS);
	return 0;
}

BEGIN_METHOD(CSERVERSOCKET_Listen,GB_INTEGER MaxConn;)

	int retval;
	int mymax=0;
 	if (!MISSING(MaxConn)) mymax=VARG(MaxConn);
	retval=srvsock_listen(THIS,mymax);
	switch(retval)
	{
		case 1:
			GB.Error("Socket is already listening");
			return;
		case 7:
			GB.Error ("You must define Path");
			return;
		case 8:
			GB.Error ("Error. You must define port");
			return;
		case 13:
			GB.Error ("Invalid maximun connections value");
			return;
	}

END_METHOD

/******************************************************************
 To accept a pending connection and delegate it to a Socket object
*******************************************************************/

BEGIN_METHOD_VOID(CSERVERSOCKET_Accept)

	CSOCKET *cli_obj;
	struct sockaddr_in myhost;
	unsigned int mylen;

	if ( SOCKET->status != 2){ GB.Error("No connection to accept");return; }

	GB.New(POINTER(&cli_obj),GB.FindClass("Socket"),"Socket",NULL);
	cli_obj->common.socket = THIS->Client;
	cli_obj->common.status=7;
	cli_obj->c_parent=(void*)THIS;
	cli_obj->OnClose=CServerSocket_OnClose;
	THIS->iCurConn++;
	GB.FreeString ( &cli_obj->sRemoteHostIP);
	GB.FreeString ( &cli_obj->sLocalHostIP);
	GB.FreeString ( &cli_obj->sPath);
	cli_obj->iLocalPort=0;
	cli_obj->iPort=0;
	cli_obj->conn_type=0;
	if (THIS->type)
	{
		cli_obj->sRemoteHostIP = GB.NewZeroString (inet_ntoa(THIS->so_client.in.sin_addr));
		cli_obj->Host = GB.NewZeroString (inet_ntoa(THIS->so_client.in.sin_addr));
		mylen=sizeof(struct sockaddr);
		getsockname (cli_obj->common.socket,(struct sockaddr*)&myhost,&mylen);
		cli_obj->sLocalHostIP = GB.NewZeroString(inet_ntoa(myhost.sin_addr));
		cli_obj->iLocalPort=ntohs(myhost.sin_port);
		cli_obj->iPort=ntohs(THIS->so_client.in.sin_port);
		cli_obj->iUsePort=ntohs(THIS->so_client.in.sin_port);
	}
	else
	{
		cli_obj->conn_type=1;
		cli_obj->sPath = GB.NewZeroString(THIS->sPath);
		cli_obj->Path = GB.NewZeroString(THIS->sPath);
	}

	CSOCKET_init_connected(cli_obj);
	// Socket returned by accept is non-blocking by default
	GB.Stream.Block(&cli_obj->common.stream, FALSE);
	//cli_obj->stream._free[0]=(intptr_t)cli_obj;

	CServerSocket_NewChild(THIS,cli_obj);

	GB.Ref(cli_obj);
	GB.Post(CSocket_post_connected,(intptr_t)cli_obj);
	SOCKET->status=3;
	GB.ReturnObject((void*)cli_obj);

END_METHOD

// BM: Enumeration of child sockets

BEGIN_METHOD_VOID(CSERVERSOCKET_next)

  int *index = (int *)GB.GetEnum();

  if (*index >= THIS->nchildren)
    GB.StopEnum();
  else
  {
    GB.ReturnObject(THIS->children[*index]);
    (*index)++;
  }

END_METHOD

BEGIN_PROPERTY(CSERVERSOCKET_count)

	GB.ReturnInteger(THIS->nchildren);

END_PROPERTY


/****************************************************************
 Here we declare public structure of the ServerSocket class
*****************************************************************/
GB_DESC CServerSocketDesc[] =
{
  GB_DECLARE("ServerSocket", sizeof(CSERVERSOCKET)),

  GB_EVENT("Connection", NULL, "(RemoteHostIP)s", &EVENT_Connection),
  GB_EVENT("Error", NULL,NULL, &EVENT_Error),

  GB_METHOD("_new", NULL, CSERVERSOCKET_new,"[(Path)s(MaxConn)i]"),
  GB_METHOD("_free", NULL, CSERVERSOCKET_free, NULL),
  GB_METHOD("Listen",NULL, CSERVERSOCKET_Listen, "[(MaxConn)i]"),
  GB_METHOD("Pause", NULL, CSERVERSOCKET_Pause, NULL),
  GB_METHOD("Resume", NULL, CSERVERSOCKET_Resume, NULL),
  GB_METHOD("Accept","Socket",CSERVERSOCKET_Accept,NULL),
  GB_METHOD("Close",NULL,CSERVERSOCKET_Close,NULL),

  GB_PROPERTY("Type","i<Net,Internet,Local>",CSERVERSOCKET_Type),
  GB_PROPERTY("Path","s",CSERVERSOCKET_Path),
  GB_PROPERTY("Port", "i", CSERVERSOCKET_Port),
  GB_PROPERTY_READ("Status","i",CSERVERSOCKET_Status),

  GB_METHOD("_next", "Socket", CSERVERSOCKET_next, NULL),
  GB_PROPERTY_READ("Count", "i", CSERVERSOCKET_count),

	GB_PROPERTY("Timeout", "i", CSOCKET_Timeout),

  GB_CONSTANT("_IsControl", "b", TRUE),
  GB_CONSTANT("_IsVirtual", "b", TRUE),
  GB_CONSTANT("_Group", "s", "Network"),
  GB_CONSTANT("_Properties", "s", "Type=0,Path,Port"),
  GB_CONSTANT("_DefaultEvent", "s", "Connection"),

  GB_END_DECLARE
};




