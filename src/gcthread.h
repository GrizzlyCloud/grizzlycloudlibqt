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
#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>

#ifdef NO_TLS
#include <QTcpSocket>
#else
#include <QSslSocket>
#endif

#include "client.h"
#include "gcserver.h"

class GCThread : public QThread
{
    Q_OBJECT

public:
#ifdef NO_TLS
    MyThread(qintptr incomingSD, QTcpSocket *rsocket, QTcpSocket *upstream, QString device,
             QByteArray deviceAddress, QList<Client> *clients, quint16 serverListenPort,
             quint16 localListenPort, MyServer *server, Tunnel *tunnel, quint32 limitUpload,
             QObject *parent = 0);
#else
   GCThread(qintptr incomingSD, QTcpSocket *rsocket, QSslSocket *upstream, QString device,
            QByteArray deviceAddress, QList<Client> *clients, quint16 serverListenPort,
            quint16 localListenPort, GCServer *server, Tunnel *tunnel, quint32 limitUpload,
            QObject *parent = 0);
#endif
    void run();

signals:
    void error(QTcpSocket::SocketError socketerror);

public slots:
    void socketRead();
    void disconnected();
    void socketReadGuard();

private:
    int closeEndPoint(qintptr fd);

private:
    QTcpSocket *socket;

    QString device;

#ifdef NO_TLS
    QTcpSocket *upstream;
#else
    QSslSocket *upstream;
#endif

    QList<Client> *clients;

    quint16 serverListenPort;
    quint16 localListenPort;

    QByteArray deviceAddress;

    QByteArray writeBuffer;

    GCServer *server;

    Tunnel *tunnel;

    bool allowUpload;

    quint32 limitUpload;
};

#endif // MYTHREAD_H
