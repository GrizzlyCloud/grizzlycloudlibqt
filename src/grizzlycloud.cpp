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
#include <QList>
#include <QProcess>
#include <QVariant>
#include <QDataStream>

#include "utils.h"
#include "proto.hpp"
#include "message.h"
#include "gcserver.h"
#include "grizzlycloud.h"

GrizzlyCloud::GrizzlyCloud(QObject *parent) : QObject(parent)
{
    targetBytesLength = actualBytesLength = 0;
    actualBytes.clear();

    allowDownload = true;
    authenticated = false;

    upstreamSocket = NULL;

    tunnelHolder = new Tunnel(this);

    connect(tunnelHolder, SIGNAL(tunnel_signal_UpdatedTraffic(QString, QString, QString, QString, QString, QString, QString)),
            this, SLOT(updatedTraffic(QString, QString, QString, QString, QString, QString, QString)));

    connect(tunnelHolder, SIGNAL(tunnel_signal_TunnelRemove(QString)),
            this, SLOT(tunnelRemove(QString)));
}

qint32 GrizzlyCloud::availableHost(QString &useIP, QString &useHost)
{
    qint32 ping			= 999;
    QList<ServersList> servers;

#define LOCALHOST
#ifdef LOCALHOST
    servers.append(ServersList("localhost", "localhost"));
#else
    servers.append(ServersList("93.185.107.138", 	"cz01"));
    servers.append(ServersList("185.101.98.180", 	"us01"));
    servers.append(ServersList("176.126.245.114", 	"uk01"));
#endif

    if(servers.length() == 1) {
        useHost = servers[0].hostname;
        useIP = servers[0].ip;
        return GC_OK;
    }

    for(QList<ServersList>::iterator i = servers.begin(); i != servers.end(); i++) {
        QProcess pingProcess;

#ifdef Q_OS_WIN
        QString exe = "ping -n 1 " + (*i).ip;
#else
        QString exe = "ping -c 1 " + (*i).ip;
#endif
        pingProcess.start(exe);
        pingProcess.waitForFinished();
        QString output(pingProcess.readAll());

        QRegExp rx("(time=)(\\d+)((.*)ms)");
        int pos = rx.indexIn(output);
        QStringList list = rx.capturedTexts();

        if(pos != -1 && list.length() >= 3) {
            (*i).ping = QVariant(list[2]).toInt();
            if(ping > (*i).ping) {
                ping 	= (*i).ping;
                useHost = (*i).hostname;
                useIP	= (*i).ip;
            }
        }
    }

    if(useIP == "" || useHost == "") {
        return GC_ERR;
    }

    return GC_OK;
}

void GrizzlyCloud::setDownloadLimit(quint32 lLimitDownload)
{
    limitDownload = lLimitDownload;
    upstreamSocket->setReadBufferSize(Utils::limitBandwidth(limitDownload));
}

enum gc_e GrizzlyCloud::commandStartSession(QString ip, qint16 port)
{
    if(!upstreamSocket) {
#ifdef NO_TLS
        upstreamSocket = new QTcpSocket(this);

        connect(upstreamSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));
        connect(upstreamSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(socketError(QAbstractSocket::SocketError)));
        connect(upstreamSocket, SIGNAL(readyRead()),
                this, SLOT(socketReadyReadGuard()));
#else
        upstreamSocket = new QSslSocket(this);

        connect(upstreamSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));
        connect(upstreamSocket, SIGNAL(encrypted()),
                this, SLOT(socketEncrypted()));
        connect(upstreamSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(socketError(QAbstractSocket::SocketError)));
        connect(upstreamSocket, SIGNAL(sslErrors(QList<QSslError>)),
                this, SLOT(sslErrors(QList<QSslError>)));
        connect(upstreamSocket, SIGNAL(readyRead()),
                this, SLOT(socketReadyReadGuard()));
#endif
    }

#ifdef NO_TLS
    upstreamSocket->connectToHost(ip, port);
#else
    upstreamSocket->connectToHostEncrypted(ip, port);
#endif

    return GC_REGISTERED;
}

void GrizzlyCloud::sslErrors(QList<QSslError> list)
{
#ifndef NO_TLS
    upstreamSocket->ignoreSslErrors();
#endif
    (void) list;
}

void GrizzlyCloud::socketEncrypted()
{
}

void GrizzlyCloud::socketError(QAbstractSocket::SocketError)
{
    emit gc_signal_SocketError(upstreamSocket->error());
}

void GrizzlyCloud::socketReadyReadGuard()
{
    if(allowDownload == false) {
        return;
    }

    socketReadyRead();
}

void GrizzlyCloud::cleanupDownload()
{
    if(limitDownload > 0) {
        // Suspend signals until we clear out buffer
        allowDownload = false;
        QTimer::singleShot(200, this, SLOT(socketReadyRead()));
    } else {
        // Accept more signals since there is nothing left to read
        allowDownload = true;
    }
}

void GrizzlyCloud::socketReadyRead()
{
    QByteArray qb = upstreamSocket->readAll();

    if(qb.length() == 0) {
        allowDownload = true;
        return;
    }

    bool doEval = false;

    actualBytesLength   += qb.length();
    actualBytes         += qb;

again:
    if(actualBytesLength == 0) {
        cleanupDownload();
        return;
    }

    if(targetBytesLength == 0) {

        QDataStream ds(actualBytes);
        ds >> targetBytesLength;
        targetBytesLength      += sizeof(int);

        if(actualBytesLength < targetBytesLength) {
            cleanupDownload();
            return;
        }
    } else {
        if(targetBytesLength > actualBytesLength) {
            cleanupDownload();
            return;
        }
    }

    QByteArray input = actualBytes;
    if(actualBytesLength > targetBytesLength) {
        QByteArray tmp = actualBytes.mid(targetBytesLength);
        actualBytes.clear();
        actualBytes = tmp;
        actualBytesLength -= targetBytesLength;
        targetBytesLength = 0;
        doEval = true;
    } else {
        actualBytes.clear();
        actualBytesLength = 0;
    }

    targetBytesLength = 0;

    QByteArray inputParse = input.remove(0, sizeof(int));
    GCProto gc;

    if(gc.deserialize(inputParse) != 0) {
        emit gc_signal_VersionMismatch(gc.version_mismatch.master, gc.version_mismatch.slave);
        return;
    }

    if(gc.type == ACCOUNT_LOGIN_REPLY &&
       gc.account_login_reply.error != "ok" &&
       gc.account_login_reply.error != "ok_registered") {

        emit gc_signal_Login(userCloud, userDevice, gc.account_login_reply.error);
        cleanupDownload();
        return;
    }

    switch(gc.type) {
        case VERSION_MISMATCH:
            // it should never get here as gc.deserialize() returns error
            emit gc_signal_VersionMismatch(gc.version_mismatch.master, gc.version_mismatch.slave);
        break;
        case ACCOUNT_LOGIN_REPLY:
            if(gc.account_login_reply.error == "ok" && authenticated == false) {

                authenticated = true;
                emit gc_signal_Login(userCloud, userDevice, gc.account_login_reply.error);
            }
        break;
        case MESSAGE_TO_SET_REPLY:
            //qDebug() << "MessageToSetReply error: " << gc.message_to_set_reply.error;
        break;

        case ACCOUNT_SET_REPLY:
            emit gc_signal_AccountSet(gc.account_set_reply.error);
        break;
        case ACCOUNT_EXISTS_REPLY:
            emit gc_signal_AccountExists(gc.account_exists_reply.error);
        break;
        case MESSAGE_FROM:
            {
                QString tp(gc.message_from.tp);
                if(tp.startsWith("tunnel_request")) {
                    GCClient *c = Message::tunnelRequest(&endpoints, &gc, tunnelHolder,
                                                         upstreamSocket, allowedPorts, lUpload);
                    if(c == (GCClient *)1) {
                        QStringList p = QString(gc.message_from.tp).split(QRegExp("[/]+"), QString::SkipEmptyParts);
                        QString port(p[1]);

                        emit gc_signal_TunnelDeniedDefender(QString(gc.message_from.from_cloud),
                                                            QString(gc.message_from.from_device),
                                                            port);
                    } else if(c != NULL) {
                        connect(c, SIGNAL(signalTunnelAdd(QString, QString, quint16, quint16, qintptr, bool)),
                                this, SLOT(tunnelAdd(QString, QString, quint16, quint16, qintptr, bool)));
                        connect(c, SIGNAL(signalClientStateChanged(QByteArray,QByteArray,quint16,QAbstractSocket::SocketState)),
                                this, SLOT(clientStateChanged(QByteArray,QByteArray,quint16,QAbstractSocket::SocketState)));
                    }

                } else if(tp.startsWith("tunnel_response")) {
                    Message::tunnelResponse(&gc, tunnelHolder, &servers);
                } else if(tp.startsWith("tunnel_denied")) {
                    QStringList p = QString(gc.message_from.tp).split(QRegExp("[/]+"), QString::SkipEmptyParts);
                    emit gc_signal_TunnelDenied(p[1], gc.message_from.from_cloud, gc.message_from.from_device);
                } else if(tp.startsWith("tunnel_endpoint_close")) {
                    Message::tunnelEndpointClose(&endpoints, &gc);
                } else if(tp.startsWith("chat")) {
                    emit gc_signal_Chat(gc.message_from.from_cloud,
                                        gc.message_from.from_device,
                                        gc.message_from.body);
                }
            }
        break;
        case OFFLINE_SET: {
            stopEndpoint(gc.offline_set.address);
            emit gc_signal_DeviceOffline(gc.offline_set.cloud,
                                       gc.offline_set.device,
                                       gc.offline_set.address);
        }
        break;
        case ACCOUNT_LIST_REPLY:
            if(gc.account_list_reply.error == "ok") {
                QByteArray al(gc.account_list_reply.list);
                 if(al.isEmpty()) {
                    emit gc_signal_AccountList(QStringList(""));
                } else {
                    QDataStream stream(&al, QIODevice::ReadOnly);
                    QStringList r;
                    r.clear();
                    while(stream.atEnd() == false) {
                        QByteArray account;
                        stream >> account;
                        r.append(QString(account));
                    }
                    emit gc_signal_AccountList(r);
                }
            }
        break;
        case DEVICE_PAIR_REPLY:
            if(gc.device_pair_reply.error == "ok") {
                QByteArray qbl(gc.device_pair_reply.list);
                if(qbl.isEmpty()) {
                    emit gc_signal_PairDevice(QString(""),
                                            QString(""),
                                            QByteArray(""),
                                            QByteArray(""),
                                            QByteArray(""),
                                            QByteArray(""),
                                            QString("ok"));
                } else {
                    QDataStream stream(&qbl, QIODevice::ReadOnly);
                    while(stream.atEnd() == false) {
                        QByteArray pid;
                        QByteArray device;
                        QByteArray localPort;
                        QByteArray remotePort;
                        stream >> pid;
                        stream >> device;
                        stream >> localPort;
                        stream >> remotePort;
                        emit gc_signal_PairDevice(gc.device_pair_reply.cloud,
                                                  device, pid, localPort, remotePort, gc.device_pair_reply.type, "ok");
                    }
                }
            }
        break;
        case TRAFFIC_GET_REPLY: {
            QList<TrafficList> list;
            if(gc.traffic_get_reply.error == "ok" || gc.traffic_get_reply.error == "ok_partial") {
                QDataStream stream(&gc.traffic_get_reply.list, QIODevice::ReadOnly);
                while(stream.atEnd() == false) {
                    TrafficList traffic;
                    stream >> traffic.type;
                    stream >> traffic.cloud;
                    stream >> traffic.device;
                    stream >> traffic.upload;
                    stream >> traffic.download;
                    list.append(traffic);
                }
            }

            emit gc_signal_TrafficGet(list, gc.traffic_get_reply.error);
        }
        break;
        default:
            //qDebug() << "unknown type: " << gc.type << "length: " << inputParse;
        break;
    }

    if(doEval == true) {
        goto again;
    }

    cleanupDownload();
}

void GrizzlyCloud::stopEndpoint(QByteArray deviceAddress)
{
    for(QMap<QByteArray, GCClient*>::iterator i = endpoints.begin(); i != endpoints.end(); i++) {
        GCClient *c = i.value();
        if(c->deviceAddress == deviceAddress) {
            c->close();
            delete c;
            i = endpoints.erase(i);
            break;
        }
    }
}

void GrizzlyCloud::clientStateChanged(QByteArray cloud, QByteArray device, quint16 port, QAbstractSocket::SocketState state)
{
    emit gc_signal_ClientStateChanged(cloud, device, port, state);
}

void GrizzlyCloud::tunnelAdd(QString cloud, QString device, quint16 localPort,
                                     quint16 remotePort, qintptr fd, bool type)
{
    tunnelHolder->add(fd, NULL, NULL, cloud, device,
                      QString::number(localPort), QString::number(remotePort),
                      false);

    emit gc_signal_TunnelAdd(cloud, device, localPort, remotePort, fd, type);
}

void GrizzlyCloud::tunnelRemove(QString fd)
{
    emit gc_signal_TunnelRemove(fd);
}

void GrizzlyCloud::updatedTraffic(QString cloud, QString device, QString localPort, QString remotePort,
                                  QString fd, QString down, QString up)
{
    emit gc_signal_UpdatedTraffic(cloud, device, localPort, remotePort,
                                  fd, down, up);
}

void GrizzlyCloud::socketStateChanged(QAbstractSocket::SocketState)
{
    emit gc_signal_SocketStateChanged(upstreamSocket->state());
}

enum gc_e GrizzlyCloud::commandPairDevice(QString &cloud, QString &device, quint16 localPort, quint16 remotePort)
{
    GCProto gc;
    gc.type = DEVICE_PAIR;
    gc.device_pair.cloud  	   = cloud.toUtf8();
    gc.device_pair.device 	   = device.toUtf8();
    gc.device_pair.local_port  = QVariant(localPort).toByteArray();
    gc.device_pair.remote_port = QVariant(remotePort).toByteArray();

    Utils::send(upstreamSocket, gc);

    return GC_REGISTERED;
}

enum gc_e GrizzlyCloud::commandAllowedAdd(quint16 port, quint32 limitUpload)
{
    if(allowedPorts.add(port) == true) {
        lUpload = limitUpload;
        return GC_OK;
    }
    else return GC_ERR;
}

enum gc_e GrizzlyCloud::commandAllowedRemove(quint16 port, bool clear)
{
    if(allowedPorts.remove(port, clear) == true) return GC_OK;
    else return GC_ERR;
}

enum gc_e GrizzlyCloud::commandTrafficMi()
{
    GCProto gc;
    gc.type = TRAFFIC_MI;

    Utils::send(upstreamSocket, gc);

    return GC_REGISTERED;
}

enum gc_e GrizzlyCloud::commandAccountCreate(QString email, QString password)
{
    GCProto gc;
    gc.type = ACCOUNT_SET;
    gc.account_set.email		= email.toUtf8();
    gc.account_set.password		= password.toUtf8();

    Utils::send(upstreamSocket, gc);

    return GC_REGISTERED;
}

enum gc_e GrizzlyCloud::commandAccountExists(QString email, QString password)
{
    GCProto gc;
    gc.type = ACCOUNT_EXISTS;
    gc.account_exists.email		= email.toUtf8();
    gc.account_exists.password	= password.toUtf8();

    Utils::send(upstreamSocket, gc);

    return GC_REGISTERED;
}

enum gc_e GrizzlyCloud::commandAccountList()
{
    GCProto gc;
    gc.type = ACCOUNT_LIST;

    Utils::send(upstreamSocket, gc);

    return GC_REGISTERED;
}

enum gc_e GrizzlyCloud::commandChat(QByteArray address, QString body, QString to)
{
    GCProto gc;
    gc.type = MESSAGE_TO;
    gc.message_to.address 	= address;
    gc.message_to.body 		= body.toUtf8();
    gc.message_to.to 		= to.toUtf8();
    gc.message_to.tp 		= "chat";

    Utils::send(upstreamSocket, gc);

    return GC_REGISTERED;
}

enum gc_e GrizzlyCloud::commandLogin(QString &cloud, QString &password, QString &device)
{
    GCProto gc;
    gc.type = ACCOUNT_LOGIN;
    gc.account_login.email 		= cloud.toUtf8();
    gc.account_login.password 	= password.toUtf8();
    gc.account_login.devname 	= device.toUtf8();

    userDevice = device;
    userCloud  = cloud;

    Utils::send(upstreamSocket, gc);

    return GC_REGISTERED;
}

enum gc_e GrizzlyCloud::commandStopSession()
{
    if(upstreamSocket) upstreamSocket->close();

    // logout locally
    authenticated = false;

    // remove all tunnels
    tunnelHolder->removeAll();

    // remove all allowed ports
    allowedPorts.remove(-1, true);

    // remove endpoints
    for(QMap<QByteArray, GCClient*>::iterator i = endpoints.begin();
        i != endpoints.end(); i++) {
        //(*i)->close();
    }

    endpoints.clear();

    return GC_OK;
}

enum gc_e GrizzlyCloud::commandTrafficGet()
{
    GCProto gc;
    gc.type = TRAFFIC_GET;

    Utils::send(upstreamSocket, gc);

    return GC_REGISTERED;
}

enum gc_e GrizzlyCloud::commandTunnelStart(QString &cloud, QString &device, QByteArray address,
                                           quint16 tunnelLocalPort, quint16 tunnelRemotePort,
                                           quint32 limitUpload, qintptr *fd)
{
    GCServer *server = new GCServer(device, address, upstreamSocket, tunnelHolder);
    if(server->startServer(tunnelLocalPort, tunnelRemotePort, limitUpload) == false) {
        return GC_ERR;
    }

    servers.push_back(server);

    tunnelHolder->add(server->socketDescriptor(), server, NULL,
                      cloud, device,
                      QVariant(tunnelLocalPort).toString(),
                      QVariant(tunnelRemotePort).toString(),
                      true);

    *fd = server->socketDescriptor();

    return GC_OK;
}

enum gc_e GrizzlyCloud::commandTunnelStop(qint32 fd)
{
    tunnelHolder->remove(fd);

    return GC_OK;
}
