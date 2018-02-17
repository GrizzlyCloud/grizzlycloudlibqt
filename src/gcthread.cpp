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
#include "client.h"
#include "utils.h"

#include "gcthread.h"

#ifdef NO_TLS
MyThread::MyThread(qintptr incomingSD, QTcpSocket *rsocket, QTcpSocket *upstream,
                   QString device, QByteArray deviceAddress, QList<Client> *clients,
                   quint16 serverListenPort, quint16 localListenPort, MyServer *server,
                   Tunnel *tunnel, quint32 lLimitUpload, QObject *parent) :
    QThread(parent),
    socket(rsocket),
    device(device),
    upstream(upstream),
    clients(clients),
    serverListenPort(serverListenPort),
    localListenPort(localListenPort),
    deviceAddress(deviceAddress),
    server(server),
    tunnel(tunnel)
#else
GCThread::GCThread(qintptr incomingSD, QTcpSocket *rsocket, QSslSocket *upstream,
                   QString device, QByteArray deviceAddress, QList<Client> *clients,
                   quint16 serverListenPort, quint16 localListenPort, GCServer *server,
                   Tunnel *tunnel, quint32 lLimitUpload, QObject *parent) :
    QThread(parent),
    socket(rsocket),
    device(device),
    upstream(upstream),
    clients(clients),
    serverListenPort(serverListenPort),
    localListenPort(localListenPort),
    deviceAddress(deviceAddress),
    server(server),
    tunnel(tunnel)
#endif
{
    if(!socket->setSocketDescriptor(incomingSD))
    {
        emit error(socket->error());
        return;
    }

    // connect socket and signal
    // note - Qt::DirectConnection is used because it's multithreaded
    //        This makes the slot to be invoked immediately, when the signal is emitted.
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadGuard()), Qt::DirectConnection);
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));

    allowUpload = true;
    limitUpload = lLimitUpload;
    socket->setReadBufferSize(Utils::limitBandwidth(limitUpload));
}

void GCThread::run()
{
    exec();
}

void GCThread::socketReadGuard()
{
    if(allowUpload == false) {
        return;
    }

    socketRead();
}

void GCThread::socketRead()
{
    if(socket->bytesAvailable() < 1) {
        // If there is no more data available, accept signals
        allowUpload = true;
        return;
    }

    QByteArray Data = socket->readAll();

    QString tp = QString("tunnel_request/" +
                         QVariant(serverListenPort).toString() + "/" +
                         QVariant(socket->socketDescriptor()).toString() + "/" +
                         QVariant(localListenPort).toString());

    writeBuffer.append(Data);

    QByteArray compressed = writeBuffer; //qCompress(writeBuffer, 0);

    tunnel->calculateTraffic(server->socketDescriptor(), compressed.length(), true);

    if(upstream->isWritable()) {
        Utils::sendMessage(upstream, compressed, device, deviceAddress, tp);
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

void GCThread::disconnected()
{
    for(QList<Client>::iterator i = clients->begin(); i != clients->end(); i++) {
        if((*i).socket == socket) {
            closeEndPoint(i->fd);
            i = clients->erase(i);
            break;
        }
    }

    socket->deleteLater();
}

int GCThread::closeEndPoint(qintptr fd)
{
    QString tp = QString("tunnel_endpoint_close/" + QVariant(serverListenPort).toString() + "/" + QVariant(fd).toString());

    return Utils::sendMessage(upstream, QByteArray(""), device, deviceAddress, tp);
}
