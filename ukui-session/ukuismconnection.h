/*****************************************************************
ksmserver - the KDE session management server

Copyright 2000 Matthias Ettrich <ettrich@kde.org>
Copyright 2005 Lubos Lunak <l.lunak@kde.org>
Copyright 2021 KylinSoft Co., Ltd.

relatively small extensions by Oswald Buddenhagen <ob6@inf.tu-dresden.de>

some code taken from the dcopserver (part of the KDE libraries), which is
Copyright 1999 Matthias Ettrich <ettrich@kde.org>
Copyright 1999 Preston Brown <pbrown@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef UKUISMCONNECTION_H
#define UKUISMCONNECTION_H

#include <QSocketNotifier>
#include <X11/ICE/ICElib.h>


class UKUISMConnection : public QSocketNotifier
{
public:
    UKUISMConnection(IceConn conn) : QSocketNotifier(IceConnectionNumber(conn), QSocketNotifier::Read )
    {
        iceConn = conn;
    }

    IceConn iceConn;
};

class UKUISMListener : public QSocketNotifier
{
public:
    UKUISMListener(IceListenObj obj) : QSocketNotifier(IceGetListenConnectionNumber(obj), QSocketNotifier::Read)
    {
        listenObj = obj;
    }

    IceListenObj listenObj;
};

#endif // UKUISMCONNECTION_H
