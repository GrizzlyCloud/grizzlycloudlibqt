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
#include <QDateTime>

#include "proto.hpp"
#include "utils.h"

Utils::Utils()
{
}

Utils::~Utils()
{
}

void Utils::swap_memory(char *dst, int ndst)
{
    int i, j;

    for(i = 0, j = ndst - 1; (i < (ndst / 2) && j > 0); i++, j--) {
        dst[j] ^= dst[i];
        dst[i] ^= dst[j];
        dst[j] ^= dst[i];
    }
}

#ifdef NO_TLS
int Utils::send(QTcpSocket *socket, GCProto &gc)
#else
int Utils::send(QSslSocket *socket, GCProto &gc)
#endif
{
    QByteArray m = gc.serialize();
    int len = m.length();
    char *dst = new char[len * 2];

    memcpy(dst, &len, sizeof(len));
    Utils::swap_memory(dst, sizeof(len));
    memcpy(dst + sizeof(len), m.data(), len);

    socket->write(dst, len + sizeof(len));

    delete[] dst;
    return 0;
}

QString Utils::time()
{
    QDateTime current = QDateTime::currentDateTime();
    return current.toString("dd/MM hh:mm:ss");
}

#ifdef NO_TLS
int Utils::sendMessage(QTcpSocket *socket, QByteArray message, QString dst, QByteArray address, QString type)
#else
int Utils::sendMessage(QSslSocket *socket, QByteArray message, QString dst, QByteArray address, QString type)
#endif
{
    GCProto gc;
    gc.type = MESSAGE_TO;
    gc.message_to.address   = address;
    gc.message_to.body      = message;
    gc.message_to.to        = dst.toUtf8();
    gc.message_to.tp        = type.toUtf8();

    return send(socket, gc);
}

quint32 Utils::limitBandwidth(quint32 limitUpload)
{
    if(limitUpload > 5) return limitUpload / 5;
    else 				return 0;
}
