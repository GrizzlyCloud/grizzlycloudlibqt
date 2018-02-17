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
#include <QObject>

#include "allowed.h"

Allowed::Allowed(QObject *parent):
    QObject(parent)
{
    allowedPorts.clear();
}

bool Allowed::add(quint16 port)
{
    if(!exists(port)) {
        allowedPorts.push_back(port);
        return true;
    }

    return false;
}

bool Allowed::remove(quint16 port, bool clear)
{
    if(clear == true) {
        allowedPorts.clear();
        return true;
    } else {
        for(QList<quint16>::iterator i = allowedPorts.begin();
            i != allowedPorts.end();
            i++) {
            if((*i) == port) {
                i = allowedPorts.erase(i);
                return true;
            }
        }
    }

    return false;
}

bool Allowed::exists(quint16 port)
{
    for(QList<quint16>::iterator i = allowedPorts.begin(); i != allowedPorts.end(); i++) {
        if(*i == port) {
            return true;
        }
    }

    return false;
}
