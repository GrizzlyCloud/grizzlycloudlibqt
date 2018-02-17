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
#ifndef UTILS_H
#define UTILS_H

#ifdef NO_TLS
#include <QTcpSocket>
#else
#include <QSslSocket>
#endif

#include "proto.hpp"

namespace Ui {
class Utils;
}

class Utils
{
public:
    explicit Utils();
    ~Utils();

    static QString time();

#ifdef NO_TLS
    static int send(QTcpSocket *socket, GCProto &gc);
#else
    static int send(QSslSocket *socket, GCProto &gc);
#endif
    static void swap_memory(char *dst, int ndst);
#ifdef NO_TLS
    static int sendMessage(QTcpSocket *socket, QByteArray message, QString dst, QByteArray address, QString type);
#else
    static int sendMessage(QSslSocket *socket, QByteArray message, QString dst, QByteArray address, QString type);
#endif
    static quint32 limitBandwidth(quint32 limitUpload);
};

#endif // UTILS_H
