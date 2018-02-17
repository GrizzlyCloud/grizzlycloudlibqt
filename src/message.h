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
#ifndef MESSAGE_H
#define MESSAGE_H

#include "grizzlycloud.h"
#include "allowed.h"
#include "proto.hpp"
#include "gcclient.h"
#include "gcserver.h"

class Message
{
public:
    Message();

#ifdef NO_TLS
    static MyClient *tunnelRequest(QMap<QByteArray, MyClient*> *endPoints, GCProto *gc,
                                   Tunnel *tunnelHolder, QTcpSocket *upstreamSocket,
                                   Allowed &allowedPorts, qint32 limitUpload);
#else
    static GCClient *tunnelRequest(QMap<QByteArray, GCClient*> *endPoints, GCProto *gc,
                                   Tunnel *tunnelHolder, QSslSocket *upstreamSocket,
                                   Allowed &allowedPorts, qint32 limitUpload);
#endif
    static void tunnelResponse(GCProto *gc, Tunnel *tunnelHolder,
                               QList<GCServer *> *servers);

    static enum gc_e tunnelEndpointClose(QMap<QByteArray, GCClient*> *endPoints, GCProto *gc);
signals:
    void signalTunnelAdd(QString cloud, QString device, quint16 portLocal,
                         quint16 portRemote, qintptr fd, bool type);

};

#endif // MESSAGE_H
