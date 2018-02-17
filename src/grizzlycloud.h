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
#ifndef GRIZZLYCLOUD_H
#define GRIZZLYCLOUD_H

#include "gc_client_lib_global.h"

#include "allowed.h"
#include "tunnel.h"
#include "utils.h"

#define GC_CLOUD_PORT	17041

enum gc_e {
    GC_OK,
    GC_REGISTERED,
    GC_ERR,
};

class ServersList {
public:
    ServersList(QString ip, QString hostname, qint32 ping = -1) :
        ip(ip), hostname(hostname), ping(ping) {}

    QString ip;
    QString hostname;
    qint32 ping;
};

class TrafficList {
public:
    QByteArray type;
    QByteArray cloud;
    QByteArray device;
    QByteArray upload;
    QByteArray download;
};

class GC_CLIENT_LIBSHARED_EXPORT GrizzlyCloud : public QObject
{
    Q_OBJECT

public:
    GrizzlyCloud(QObject *parent);

    qint32 	availableHost(QString &useIP, QString &useHost);
    void 	setDownloadLimit(quint32 limitDownload);

    // commands
    enum gc_e commandStartSession(QString ip, qint16 port);
    enum gc_e commandStopSession();

    enum gc_e commandPairDevice(QString &cloud, QString &device, quint16 localPort, quint16 remotePort);
    enum gc_e commandLogin(QString &cloud, QString &password, QString &device);

    enum gc_e commandTunnelStart(QString &cloud, QString &device, QByteArray address,
                                 quint16 tunnelLocalPort, quint16 tunnelRemotePort,
                                 quint32 limitUpload, qintptr *fd);
    enum gc_e commandTunnelStop(qint32 fd);
    enum gc_e commandChat(QByteArray address, QString body, QString to);

    enum gc_e commandTrafficGet();

    enum gc_e commandAllowedRemove(quint16 port, bool clear);
    enum gc_e commandAllowedAdd(quint16 port, quint32 limitUpload);

    enum gc_e commandAccountCreate(QString email, QString password);

    enum gc_e commandAccountList();

    enum gc_e commandTrafficMi();

    enum gc_e commandAccountExists(QString email, QString password);

    //void daemonize();
signals:
    void gc_signal_Login(QString error, QString cloud, QString device);
    void gc_signal_SocketError(QAbstractSocket::SocketError);
    void gc_signal_SocketStateChanged(QAbstractSocket::SocketState);
    void gc_signal_PairDevice(QString cloud, QString device, QByteArray pid, QByteArray localPort,
                              QByteArray remotePort, QByteArray type, QString error);
    void gc_signal_UpdatedTraffic(QString, QString, QString, QString, QString, QString, QString);
    void gc_signal_TunnelAdd(QString, QString, quint16, quint16, qintptr, bool);
    void gc_signal_TunnelRemove(QString);
    void gc_signal_DeviceOffline(QString cloud, QString device, QByteArray address);
    void gc_signal_TunnelDenied(QString port, QString cloud, QString device);
    void gc_signal_Chat(QString cloud, QString device, QString body);
    void gc_signal_TrafficGet(QList<TrafficList> list, QString error);
    void gc_signal_TunnelDeniedDefender(QString cloud, QString device, QString port);
    void gc_signal_AccountSet(QByteArray error);
    void gc_signal_AccountList(QStringList list);
    void gc_signal_AccountExists(QByteArray error);
    void gc_signal_ClientStateChanged(QByteArray cloud, QByteArray device, quint16 port, QAbstractSocket::SocketState state);
    void gc_signal_VersionMismatch(QByteArray master, QByteArray slave);

private slots:
    void socketReadyRead();
    void socketError(QAbstractSocket::SocketError);
    void socketStateChanged(QAbstractSocket::SocketState);
    void socketEncrypted();
    void sslErrors(QList<QSslError> list);

    void updatedTraffic(QString cloud, QString device, QString localPort, QString remotePort, QString fd, QString down, QString up);

    void tunnelAdd(QString, QString, quint16, quint16, qintptr, bool);
    void tunnelRemove(QString fd);

    void clientStateChanged(QByteArray cloud, QByteArray device, quint16 port, QAbstractSocket::SocketState state);

    void socketReadyReadGuard();

private:
    void stopEndpoint(QByteArray deviceAddress);
    void cleanupDownload();

#ifdef NO_TLS
    QTcpSocket *upstreamSocket;
#else
    QSslSocket *upstreamSocket;
#endif
    bool authenticated;

    // readAll
    int targetBytesLength;
    int actualBytesLength;
    QByteArray actualBytes;

    QString userDevice;
    QString userCloud;

    Tunnel *tunnelHolder;
    QList<GCServer *> servers;

    QMap<QByteArray, GCClient*> endpoints;

    Allowed allowedPorts;

    bool allowDownload;

    quint32 lUpload; 		// upload limit
    quint32 limitDownload;	// download limit
};

#endif // GRIZZLYCLOUD_H
