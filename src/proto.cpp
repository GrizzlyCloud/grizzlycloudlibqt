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
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include "proto.hpp"

GCProto::GCProto()
{
}

QByteArray GCProto::serialize()
{
    QByteArray b;
    QDataStream stream(&b, QIODevice::WriteOnly);
    qint32 version = PROTO_VERSION;

    stream << version;
    stream << type;

	switch(type) {
		case MESSAGE_TO_SET_REPLY:
			if(
			(message_to_set_reply.error == "ok") ||
			(message_to_set_reply.error == "ok_registered") ||
			(message_to_set_reply.error == "login") ||
			(message_to_set_reply.error == "enoexists") ||
			(message_to_set_reply.error == "general_failure") 			) {
				stream << message_to_set_reply.error;
			} else { return NULL; }
		break;
		case MESSAGE_TO:
			stream << message_to.to;
			stream << message_to.address;
			stream << message_to.tp;
			stream << message_to.body;
		break;
		case ACCOUNT_LIST:
		break;
		case ACCOUNT_LIST_REPLY:
			if(
			(account_list_reply.error == "ok") ||
			(account_list_reply.error == "general_failure") 			) {
				stream << account_list_reply.error;
			} else { return NULL; }
			stream << account_list_reply.list;
		break;
		case TRAFFIC_MI:
		break;
		case TRAFFIC_GET:
		break;
		case TRAFFIC_GET_REPLY:
			stream << traffic_get_reply.list;
			if(
			(traffic_get_reply.error == "ok") ||
			(traffic_get_reply.error == "ok_partial") ||
			(traffic_get_reply.error == "login") ||
			(traffic_get_reply.error == "general_failure") ||
			(traffic_get_reply.error == "denied") ||
			(traffic_get_reply.error == "empty") 			) {
				stream << traffic_get_reply.error;
			} else { return NULL; }
		break;
		case MESSAGE_FROM:
			stream << message_from.from_cloud;
			stream << message_from.from_device;
			stream << message_from.from_address;
			stream << message_from.tp;
			stream << message_from.body;
		break;
		case DEVICE_PAIR:
			stream << device_pair.cloud;
			stream << device_pair.device;
			stream << device_pair.local_port;
			stream << device_pair.remote_port;
		break;
		case DEVICE_PAIR_REPLY:
			stream << device_pair_reply.cloud;
			if(
			(device_pair_reply.error == "ok") ||
			(device_pair_reply.error == "ok_registered") ||
			(device_pair_reply.error == "login") ||
			(device_pair_reply.error == "general_failure") 			) {
				stream << device_pair_reply.error;
			} else { return NULL; }
			stream << device_pair_reply.list;
			stream << device_pair_reply.type;
		break;
		case OFFLINE_SET:
			stream << offline_set.address;
			stream << offline_set.cloud;
			stream << offline_set.device;
		break;
		case ACCOUNT_SET:
			stream << account_set.email;
			stream << account_set.password;
		break;
		case ACCOUNT_SET_REPLY:
			if(
			(account_set_reply.error == "ok") ||
			(account_set_reply.error == "try_again") ||
			(account_set_reply.error == "already_exists") ||
			(account_set_reply.error == "general_failure") 			) {
				stream << account_set_reply.error;
			} else { return NULL; }
		break;
		case ACCOUNT_GET:
		break;
		case ACCOUNT_LOGIN:
			stream << account_login.email;
			stream << account_login.password;
			stream << account_login.devname;
		break;
		case ACCOUNT_LOGIN_REPLY:
			if(
			(account_login_reply.error == "ok") ||
			(account_login_reply.error == "ok_registered") ||
			(account_login_reply.error == "version") ||
			(account_login_reply.error == "try_again") ||
			(account_login_reply.error == "invalid_login") ||
			(account_login_reply.error == "general_failure") ||
			(account_login_reply.error == "already_logged") 			) {
				stream << account_login_reply.error;
			} else { return NULL; }
		break;
		case ACCOUNT_EXISTS:
			stream << account_exists.email;
			stream << account_exists.password;
		break;
		case ACCOUNT_EXISTS_REPLY:
			if(
			(account_exists_reply.error == "ok") ||
			(account_exists_reply.error == "invalid_login") ||
			(account_exists_reply.error == "general_failure") 			) {
				stream << account_exists_reply.error;
			} else { return NULL; }
		break;
		case VERSION_MISMATCH:
			stream << version_mismatch.master;
			stream << version_mismatch.slave;
		break;

	}

	return b;
}

int GCProto::deserialize(QByteArray &b)
{
    QDataStream stream(&b, QIODevice::ReadOnly);
    qint32 version;

    stream >> version;

    if(version != PROTO_VERSION) {
        return -1;
    }

    stream >> type;

	switch(type) {
		case MESSAGE_TO_SET_REPLY:
			stream >> message_to_set_reply.error;
		break;
		case MESSAGE_TO:
			stream >> message_to.to;
			stream >> message_to.address;
			stream >> message_to.tp;
			stream >> message_to.body;
		break;
		case ACCOUNT_LIST:
		break;
		case ACCOUNT_LIST_REPLY:
			stream >> account_list_reply.error;
			stream >> account_list_reply.list;
		break;
		case TRAFFIC_MI:
		break;
		case TRAFFIC_GET:
		break;
		case TRAFFIC_GET_REPLY:
			stream >> traffic_get_reply.list;
			stream >> traffic_get_reply.error;
		break;
		case MESSAGE_FROM:
			stream >> message_from.from_cloud;
			stream >> message_from.from_device;
			stream >> message_from.from_address;
			stream >> message_from.tp;
			stream >> message_from.body;
		break;
		case DEVICE_PAIR:
			stream >> device_pair.cloud;
			stream >> device_pair.device;
			stream >> device_pair.local_port;
			stream >> device_pair.remote_port;
		break;
		case DEVICE_PAIR_REPLY:
			stream >> device_pair_reply.cloud;
			stream >> device_pair_reply.error;
			stream >> device_pair_reply.list;
			stream >> device_pair_reply.type;
		break;
		case OFFLINE_SET:
			stream >> offline_set.address;
			stream >> offline_set.cloud;
			stream >> offline_set.device;
		break;
		case ACCOUNT_SET:
			stream >> account_set.email;
			stream >> account_set.password;
		break;
		case ACCOUNT_SET_REPLY:
			stream >> account_set_reply.error;
		break;
		case ACCOUNT_GET:
		break;
		case ACCOUNT_LOGIN:
			stream >> account_login.email;
			stream >> account_login.password;
			stream >> account_login.devname;
		break;
		case ACCOUNT_LOGIN_REPLY:
			stream >> account_login_reply.error;
		break;
		case ACCOUNT_EXISTS:
			stream >> account_exists.email;
			stream >> account_exists.password;
		break;
		case ACCOUNT_EXISTS_REPLY:
			stream >> account_exists_reply.error;
		break;
		case VERSION_MISMATCH:
			stream >> version_mismatch.master;
			stream >> version_mismatch.slave;
		break;
	}


	return 0;
}
