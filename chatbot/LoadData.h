//
//  LoadData.c
//  chatbot
//
//  Created by Ashish Ahuja on 10/22/16.
//  Copyright © 2016 Fortunate-MAN (Ashish Ahuja). All rights reserved.
//

#ifndef LoadData_h
#define LoadData_h

#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"
#include "ChatBot.h"
#include "Privileges.h"
//#include "Notifications.h"

Log **loadLogs ();
void saveLogs (Log **logs, unsigned totalLogs);

Filter **loadFilters ();
void saveFilters ();

PrivRequest **loadPrivRequests ();
void savePrivRequests ();

Notify **loadNotifications ();
void saveNotifications ();

PrivUser **loadPrivUsers ();
void savePrivUsers ();

cJSON *loadReports();
void saveReports ();

#endif // LoadData_h
