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
#ifndef ALLOWED_H
#define ALLOWED_H

#include <QObject>

class Allowed : public QObject
{
    Q_OBJECT

public:
    Allowed(QObject *parent = 0);

    bool add(quint16 port);
    bool remove(quint16 port, bool clear);
    bool exists(quint16 port);

private:
    QList<quint16> allowedPorts;
};

#endif // ALLOWED_H
