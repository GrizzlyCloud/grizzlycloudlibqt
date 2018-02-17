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
#ifndef TUNNEL_H
#define TUNNEL_H

#include <QTimer>

class GCServer;
class GCClient;

class ActiveTunnelTraffic {
public:
    qint32 oldTraffic;
    qint32 newTraffic;
    qint32 currentSecond;
};

class ActiveTunnel
{
public:
    qint32  	index;
    QString		cloud;
    QString 	device;
    QString 	localPort;
    QString		remotePort;
    GCServer 	*server;
    GCClient 	*client;
    qintptr 	fd;

    ActiveTunnelTraffic download;
    ActiveTunnelTraffic upload;
};

class Tunnel : public QObject
{
    Q_OBJECT

public:
    Tunnel(QObject *parent = 0);
    ~Tunnel();

    bool add(qintptr fd, GCServer *server, GCClient *client,
             QString cloud, QString device,
             QString localPort, QString remotePort,
             bool out);
    void remove(qint32 fd);
    void removeAll();

    void calculateTraffic(qintptr fd, qint32 length, bool upload);

private slots:
    void updateTrafficTunnels();

signals:
    void tunnel_signal_UpdatedTraffic(QString, QString, QString, QString, QString, QString, QString);
    void tunnel_signal_TunnelRemove(QString);

private:
    QList<ActiveTunnel>     tunnels;
    QTimer                  *timer;
};

#endif // TUNNEL_H
