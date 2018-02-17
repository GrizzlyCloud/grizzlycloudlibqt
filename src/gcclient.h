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
#ifndef MYCLIENT_H
#define MYCLIENT_H

#include "tunnel.h"

#ifdef NO_TLS
#include <QTcpSocket>
#else
#include <QSslSocket>
#endif

class Tunnel;

class GCClient : public QTcpSocket
{
    Q_OBJECT

public:
#ifdef NO_TLS
    explicit MyClient(QByteArray device,
                      QByteArray deviceAddress,
                      QByteArray deviceCloud,
                      QTcpSocket *upstream,
                      QString socketDescriptor,
                      QMap<QByteArray, MyClient*> *endpoints,
                      quint16 lRemotePort,
                      Tunnel *tunnel,
                      QString requestClientPort,
                      quint32 limitUpload,
                      QObject *parent = 0);
#else
    explicit GCClient(QByteArray device,
                      QByteArray deviceAddress,
                      QByteArray deviceCloud,
                      QSslSocket *upstream,
                      QString socketDescriptor,
                      QMap<QByteArray, GCClient*> *endpoints,
                      quint16 lRemotePort,
                      Tunnel *tunnel,
                      QString requestClientPort,
                      quint32 limitUpload,
                      QObject *parent = 0);
#endif
    QByteArray writeBuffer;

    void writeToEndpoint();

    QByteArray deviceAddress;
private slots:
    void displayError(QAbstractSocket::SocketError socketError);
    void socketStateChanged(QAbstractSocket::SocketState state);
    void socketReadGuard();
    void socketRead();

signals:
    void signalTunnelAdd(QString cloud, QString device, quint16 localPort, quint16 remotePort,
                         qintptr fd, bool type);
    void signalClientStateChanged(QByteArray cloud, QByteArray device, quint16 port, QAbstractSocket::SocketState state);

private:

    QByteArray device;

    QByteArray deviceCloud;

#ifdef NO_TLS
    QTcpSocket *upstream;
#else
    QSslSocket *upstream;
#endif
    QString lSocketDescriptor;

    quint16 remotePort;

    QMap<QByteArray, GCClient*> *endpoints;

    Tunnel *tunnel;

    QString requestClientPort;

    bool connected;

    bool allowUpload;

    quint32 limitUpload;
};

#endif // MYCLIENT_H
