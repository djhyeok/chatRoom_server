#pragma once
#ifndef LINKEDLIST_H
#define LINKEDLIST_H

USERNODE* createUserNode();
void addUserNode(USERNODE*, USERDATA);
void deleteUserNode(USERNODE*, SOCKET);
void deleteAllUserNode(USERNODE*);
USERNODE* findUserNode(THREADDATA);
ROOMNODE* createRoomNode();
int addRoomNode(ROOMNODE*, char*, USERDATA);
void deleteRoomNode(ROOMNODE*, unsigned int);
void deleteAllRoomNode(ROOMNODE*);
void showUserList(USERNODE*);
void showRoomList(ROOMNODE*);
ROOMNODE* findRoomNode(ROOMNODE* ,int);

void printUserNode(USERNODE*);

#endif