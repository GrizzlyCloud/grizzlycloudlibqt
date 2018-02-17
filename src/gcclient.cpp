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
#include <QByteArray>

#include "proto.hpp"
#include "utils.h"

#include "tunnel.h"
#include "gcclient.h"

#ifdef NO_TLS
MyClient::MyClient(QByteArray device,
                   QByteArray deviceAddress,
                   QByteArray deviceCloud,
                   QTcpSocket *upstream, QString rSocketDescriptor,
                   QMap<QByteArray, MyClient*> *endpoints, quint16 lRemotePort, Tunnel *tunnel,
                   QString requestClientPort, quint32 lLimitUpload,
                   QObject *parent) :
    QTcpSocket(parent), device(device), deviceAddress(deviceAddress), deviceCloud(deviceCloud),
    upstream(upstream), lSocketDescriptor(rSocketDescriptor), remotePort(lRemotePort),
    endpoints(endpoints), tunnel(tunnel), requestClientPort(requestClientPort)
#else
GCClient::GCClient(QByteArray device,
                   QByteArray deviceAddress,
                   QByteArray deviceCloud,
                   QSslSocket *upstream, QString rSocketDescriptor,
                   QMap<QByteArray, GCClient*> *endpoints, quint16 lRemotePort, Tunnel *tunnel,
                   QString requestClientPort, quint32 lLimitUpload,
                   QObject *parent) :
    QTcpSocket(parent), deviceAddress(deviceAddress), device(device), deviceCloud(deviceCloud),
    upstream(upstream), lSocketDescriptor(rSocketDescriptor), remotePort(lRemotePort),
    endpoints(endpoints), tunnel(tunnel), requestClientPort(requestClientPort)
#endif
{
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(this, SIGNAL(readyRead()),
            this, SLOT(socketReadGuard()));
    connect(this, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));

    connected = false;

    allowUpload = true;
    limitUpload = lLimitUpload;
    setReadBufferSize(Utils::limitBandwidth(limitUpload));
}

void GCClient::displayError(QAbstractSocket::SocketError socketError)
{
    (void) socketError;

    tunnel->remove(socketDescriptor());
    endpoints->remove(deviceAddress);
}

void GCClient::socketReadGuard()
{
    if(allowUpload == false) {
        return;
    }

    socketRead();
}

void GCClient::socketRead()
{
    if(bytesAvailable() < 1) {
        // If there is no more data available, accept signals
        allowUpload = true;
        return;
    }

    QByteArray qb = readAll();

    writeBuffer.append(qb);
    QByteArray compressed = writeBuffer; //qCompress(writeBuffer, 0);

    if(upstream->isWritable()) {
        tunnel->calculateTraffic(socketDescriptor(), compressed.length(), true);
        Utils::sendMessage(upstream, compressed, device, deviceAddress, "tunnel_response/" + QVariant(remotePort).toString() + "/" + lSocketDescriptor);
        writeBuffer.clear();
    }

    if(limitUpload > 0) {
        // Suspend signals until we clear out buffer
        allowUpload = false;
        QTimer::singleShot(200, this, SLOT(socketRead()));
    } else {
        // Accept more signals since there is nothing left to read
        allowUpload = true;
    }
}

void GCClient::writeToEndpoint()
{
    if(connected == true && !writeBuffer.isEmpty()) {
        QByteArray uncompressed = writeBuffer; //qUncompress(writeBuffer);

        tunnel->calculateTraffic(socketDescriptor(), uncompressed.length(), false);
        write(uncompressed);
        writeBuffer.clear();
    }
}

void GCClient::socketStateChanged(QAbstractSocket::SocketState state)
{
    emit signalClientStateChanged(deviceCloud, device, remotePort, state);

    if(state == QAbstractSocket::ConnectedState && connected == false) {
        connected = true;
        writeToEndpoint();

        emit signalTunnelAdd(QString(deviceCloud),
                             QString(device),
                             remotePort,
                             QVariant(requestClientPort).toUInt(),
                             socketDescriptor(),
                             false);
    }
}
