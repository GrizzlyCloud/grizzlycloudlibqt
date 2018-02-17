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
#include <QDebug>
#include <QTimer>
#include <QTime>

#include "gcserver.h"
#include "tunnel.h"

Tunnel::Tunnel(QObject *parent):
    QObject(parent)
{
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTrafficTunnels()));
    timer->start(1000);
}

Tunnel::~Tunnel() {
    timer->stop();
    delete timer;
}

void Tunnel::updateTrafficTunnels()
{
    for(QList<ActiveTunnel>::iterator i = tunnels.begin(); i != tunnels.end(); i++) {
        QString down = "0";
        QString up = "0";

        if((*i).upload.oldTraffic > 0) {
            if((*i).upload.oldTraffic < 1000) {
                up = QVariant((*i).upload.oldTraffic).toString() + " Bytes";
            } else if((*i).upload.oldTraffic < 1000000) {
                up = QVariant((*i).upload.oldTraffic / 1000).toString() + " KB";
            } else {
                up = QVariant((*i).upload.oldTraffic / 1000000).toString() + " MB";
            }
            (*i).upload.oldTraffic = 0;
        }
        if((*i).download.oldTraffic > 0) {
            if((*i).download.oldTraffic < 1000) {
                down = QVariant((*i).download.oldTraffic).toString() + " Bytes";
            } else if((*i).download.oldTraffic < 1000000) {
                down = QVariant((*i).download.oldTraffic / 1000).toString() + " KB";
            } else {
                down = QVariant((*i).download.oldTraffic / 1000000).toString() + " MB";
            }
            (*i).download.oldTraffic = 0;
        }

        emit tunnel_signal_UpdatedTraffic(i->cloud, i->device, i->localPort, i->remotePort,
                                          QVariant((*i).fd).toString(), down, up);
    }
}

void Tunnel::calculateTraffic(qintptr fd, qint32 length, bool upload)
{
    QTime t = QTime::currentTime();
    int second = t.second();

    for(QList<ActiveTunnel>::iterator i = tunnels.begin(); i != tunnels.end(); i++)
    {
        if((*i).fd == fd) {
            if(upload == true) {
                if(second != (*i).upload.currentSecond) {
                    (*i).upload.currentSecond = second;
                    (*i).upload.oldTraffic = (*i).upload.newTraffic;
                    (*i).upload.newTraffic = 0;
                }

                (*i).upload.newTraffic += length;
            } else {
                if(second != (*i).download.currentSecond) {
                    (*i).download.currentSecond = second;
                    (*i).download.oldTraffic = (*i).download.newTraffic;
                    (*i).download.newTraffic = 0;
                }

                (*i).download.newTraffic += length;
            }

            break;
        }
    }
}

bool Tunnel::add(qintptr fd, GCServer *server, GCClient *client,
                 QString cloud, QString device,
                 QString localPort, QString remotePort,
                 bool out)
{
    ActiveTunnel at;

    at.cloud        = cloud;
    at.device       = device;
    at.localPort   	= localPort;
    at.remotePort	= remotePort;
    at.server      	= server;
    at.client		= client;
    at.fd			= fd;
    at.download.oldTraffic   = at.download.newTraffic = at.download.currentSecond = 0;
    at.upload.oldTraffic   = at.upload.newTraffic = at.upload.currentSecond = 0;

    (void) out;

    tunnels.push_back(at);

    return true;
}

void Tunnel::remove(qint32 fd)
{
    for(QList<ActiveTunnel>::iterator j = tunnels.begin(); j != tunnels.end(); j++) {
        if(fd == (*j).fd) {
            GCServer *server = (*j).server;
            if(server) {
                server->closeAll();
            }

            emit tunnel_signal_TunnelRemove(QVariant((*j).fd).toString());

            j = tunnels.erase(j);

            break;
        }
    }
}

void Tunnel::removeAll()
{
    for(QList<ActiveTunnel>::iterator j = tunnels.begin(); j != tunnels.end(); j++) {
        GCServer *server = (*j).server;
        if(server) {
            server->closeAll();
        }

        emit tunnel_signal_TunnelRemove(QVariant((*j).fd).toString());
    }

    tunnels.clear();
}
