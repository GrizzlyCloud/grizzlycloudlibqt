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
#include <QHostAddress>
#include <QString>

#include "utils.h"

#include "grizzlycloud.h"
#include "proto.hpp"
#include "gcclient.h"
#include "message.h"

Message::Message()
{
}

void Message::tunnelResponse(GCProto *gc, Tunnel *tunnelHolder, QList<GCServer *> *servers)
{
    // Type/Remote_port/Local_port
    // eg. "tunnel_response/5192/22"
    QStringList p = QString(gc->message_from.tp).split(QRegExp("[/]+"), QString::SkipEmptyParts);

    if(p.length() != 3) {
        return;
    }

    // Select from all servers
    for(QList<GCServer *>::iterator i = servers->begin(); i != servers->end(); i++) {

        // Find specific port
        if((*i)->remoteListenPort == QString(p[1]).toInt()) {

            // Deliver message only to one client on server
            bool found = false;

            for(QList<Client>::iterator j = (*i)->clients.begin();
                j != ((*i)->clients).end(); j++) {

                if((*j).socket &&
                        QVariant((*j).socket->socketDescriptor()).toInt() == QString(p[2]).toInt()) {

                    tunnelHolder->calculateTraffic((*i)->socketDescriptor(), gc->message_from.body.length(), false);
                    (*j).writeBuffer.append(gc->message_from.body);

                    QByteArray tmp = (*j).writeBuffer;
                    QByteArray uncompressed = tmp; //qUncompress(tmp);

                    if(((*j).socket)->isWritable()) {
                        (*j).socket->write(uncompressed);
                        (*j).writeBuffer.clear();
                    }
                    found = true;
                    break;
                }
            }

            if(found) break;
        }
    }
}

enum gc_e Message::tunnelEndpointClose(QMap<QByteArray, GCClient*> *endPoints, GCProto *gc)
{
    QStringList p = QString(gc->message_from.tp).split(QRegExp("[/]+"), QString::SkipEmptyParts);
    if(p.length() != 3) {
        return GC_ERR;
    }

    QString fd(p[2]);
    QByteArray key = gc->message_from.from_address + fd.toLocal8Bit();

    if(endPoints->contains(key)) {
        GCClient *c = endPoints->value(key);
        c->close();
        delete c;
        endPoints->remove(key);
    }

    return GC_OK;
}

#ifdef NO_TLS
MyClient *Message::tunnelRequest(QMap<QByteArray, MyClient*> *endPoints, GCProto *gc, Tunnel *tunnelHolder,
                                 QTcpSocket *upstreamSocket, Allowed &allowedPorts, qint32 limitUpload)
#else
GCClient *Message::tunnelRequest(QMap<QByteArray, GCClient*> *endPoints, GCProto *gc, Tunnel *tunnelHolder,
                                 QSslSocket *upstreamSocket, Allowed &allowedPorts, qint32 limitUpload)
#endif
{
    QStringList p 		= QString(gc->message_from.tp).split(QRegExp("[/]+"), QString::SkipEmptyParts);

    GCClient *c, *await = NULL;
    QString port(p[1]);
    QString fd(p[2]);
    QString requestClientPort(p[3]);
    QByteArray key 		= gc->message_from.from_address + fd.toLocal8Bit();

    if(endPoints->contains(key)) {
        c = endPoints->value(key);
    } else {
        if(!allowedPorts.exists(port.toInt())) {
            GCProto to;
            to.type = MESSAGE_TO;
            to.message_to.address = gc->message_from.from_address;
            to.message_to.body	  = "NULL";
            to.message_to.to	  = gc->message_from.from_device;
            to.message_to.tp	  = QString("tunnel_denied/" + port).toUtf8();

            Utils::send(upstreamSocket, to);

            return (GCClient *)1;
        }

        c = new GCClient(gc->message_from.from_device,
                         gc->message_from.from_address,
                         gc->message_from.from_cloud,
                         upstreamSocket,
                         fd,
                         endPoints,
                         port.toUInt(),
                         tunnelHolder,
                         requestClientPort,
                         limitUpload);

        QHostAddress qh(QHostAddress::Any);
        c->connectToHost(qh, port.toInt());
        endPoints->insert(key, c);

        await = c;
    }

    c->writeBuffer.append(gc->message_from.body);
    c->writeToEndpoint();

    return await;
}
