/*
 *
 * GrizzlyCloud QT - simplified VPN for IoT library
 * Copyright (C) 2016 - 2017 Filip Pancik
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "gcthread.h"
#include "gcserver.h"

#ifdef NO_TLS
MyServer::MyServer(QString   	device,
                   QByteArray 	deviceAddress,
                   QTcpSocket 	*upstream,
                   Tunnel		*tunnel,
                   QObject *parent) :
    QTcpServer(parent),
    device(device),
    upstream(upstream),
    deviceAddress(deviceAddress),
    tunnel(tunnel)
#else
GCServer::GCServer(QString device,
                   QByteArray deviceAddress,
                   QSslSocket *upstream,
                   Tunnel	  *tunnel,
                   QObject *parent) :
    QTcpServer(parent),
    device(device),
    upstream(upstream),
    deviceAddress(deviceAddress),
    tunnel(tunnel)
#endif
{
}

bool GCServer::startServer(quint16 port, quint16 lRemotePort, quint32 lLimitUpload)
{
    remoteListenPort = lRemotePort;
    localListenPort  = port;
    limitUpload 	 = lLimitUpload;

    if(!this->listen(QHostAddress::Any, localListenPort)) return false;
    else                                        		  return true;
}

void GCServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *socket = new QTcpSocket();
    GCThread *thread = new GCThread(socketDescriptor, socket, upstream, device,
                                    deviceAddress, &clients, remoteListenPort,
                                    localListenPort, this, tunnel, limitUpload);

    Client c;
    c.socket = socket;
    c.fd 	 = socketDescriptor;
    clients.push_back(c);

    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
}

void GCServer::closeAll()
{
    for(QList<Client>::iterator i = clients.begin(); i != clients.end(); i++) {
        (*i).socket->close();
    }
    close();
}
