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
#ifndef MYSERVER_H
#define MYSERVER_H

#include <QTcpServer>

#include "tunnel.h"
#include "client.h"

class GCServer : public QTcpServer
{
    Q_OBJECT
public:
#ifdef NO_TLS
    explicit MyServer(QString device, QByteArray deviceAddress, QTcpSocket *upstream, Tunnel *tunnel, QObject *parent = 0);
#else
    explicit GCServer(QString device, QByteArray deviceAddress, QSslSocket *upstream, Tunnel *tunnel, QObject *parent = 0);
#endif

    bool startServer(quint16 port, quint16 lRemotePort, quint32 lLimitUpload);

    void closeAll();

public:
    quint16 remoteListenPort;
    quint16 localListenPort;

    QList<Client> clients;

protected:
    void incomingConnection(qintptr socketDescriptor);

private:

    QString 	device;

#ifdef NO_TLS
    QTcpSocket *upstream;
#else
    QSslSocket *upstream;
#endif

    QByteArray 	deviceAddress;

    Tunnel 		*tunnel;

    quint32 	limitUpload;
};

#endif // MYSERVER_H
