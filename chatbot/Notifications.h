//
//  Notifications.h
//  chatbot
//
//  Created by Ashish Ahuja on 4/29/16.
//  Copyright © 2016 Fortunate-MAN. All rights reserved.
//

#ifndef Notifications_h
#define Notifications_h

typedef struct {
  long userID;
  int type;  // 0 if user has opted-in, 1 if user has notified in
}Notify;

#endif /* Notifications.h */

