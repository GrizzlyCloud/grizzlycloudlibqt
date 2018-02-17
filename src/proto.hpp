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
#ifndef PROTO_H_
#define PROTO_H_

#define PROTO_VERSION	1

enum proto_e {
	MESSAGE_TO_SET_REPLY,
	MESSAGE_TO,
	ACCOUNT_LIST,
	ACCOUNT_LIST_REPLY,
	TRAFFIC_MI,
	TRAFFIC_GET,
	TRAFFIC_GET_REPLY,
	MESSAGE_FROM,
	DEVICE_PAIR,
	DEVICE_PAIR_REPLY,
	OFFLINE_SET,
	ACCOUNT_SET,
	ACCOUNT_SET_REPLY,
	ACCOUNT_GET,
	ACCOUNT_LOGIN,
	ACCOUNT_LOGIN_REPLY,
	ACCOUNT_EXISTS,
	ACCOUNT_EXISTS_REPLY,
	VERSION_MISMATCH,
};

class Message_to_set_reply
{
public:
	QByteArray  error;
};

class Message_to
{
public:
	QByteArray  to;
	QByteArray  address;
	QByteArray  tp;
	QByteArray  body;
};

class Account_list
{
public:
	/* void */
};

class Account_list_reply
{
public:
	QByteArray  error;
	QByteArray  list;
};

class Traffic_mi
{
public:
	/* void */
};

class Traffic_get
{
public:
	/* void */
};

class Traffic_get_reply
{
public:
	QByteArray  list;
	QByteArray  error;
};

class Message_from
{
public:
	QByteArray  from_cloud;
	QByteArray  from_device;
	QByteArray  from_address;
	QByteArray  tp;
	QByteArray  body;
};

class Device_pair
{
public:
	QByteArray  cloud;
	QByteArray  device;
	QByteArray  local_port;
	QByteArray  remote_port;
};

class Device_pair_reply
{
public:
	QByteArray  cloud;
	QByteArray  error;
	QByteArray  list;
	QByteArray  type;
};

class Offline_set
{
public:
	QByteArray  address;
	QByteArray  cloud;
	QByteArray  device;
};

class Account_set
{
public:
	QByteArray  email;
	QByteArray  password;
};

class Account_set_reply
{
public:
	QByteArray  error;
};

class Account_get
{
public:
	/* void */
};

class Account_login
{
public:
	QByteArray  email;
	QByteArray  password;
	QByteArray  devname;
};

class Account_login_reply
{
public:
	QByteArray  error;
};

class Account_exists
{
public:
	QByteArray  email;
	QByteArray  password;
};

class Account_exists_reply
{
public:
	QByteArray  error;
};

class Version_mismatch
{
public:
	QByteArray  master;
	QByteArray  slave;
};

class GCProto
{
public:
	GCProto();
	QByteArray serialize();
	int deserialize(QByteArray &qb);

	qint32 type;

	Message_to_set_reply message_to_set_reply;
	Message_to message_to;
	Account_list account_list;
	Account_list_reply account_list_reply;
	Traffic_mi traffic_mi;
	Traffic_get traffic_get;
	Traffic_get_reply traffic_get_reply;
	Message_from message_from;
	Device_pair device_pair;
	Device_pair_reply device_pair_reply;
	Offline_set offline_set;
	Account_set account_set;
	Account_set_reply account_set_reply;
	Account_get account_get;
	Account_login account_login;
	Account_login_reply account_login_reply;
	Account_exists account_exists;
	Account_exists_reply account_exists_reply;
	Version_mismatch version_mismatch;
};


#endif