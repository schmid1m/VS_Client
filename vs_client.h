#ifndef VS_CLIENT_H
#define VS_CLIENT_H

#include <QObject>
#include <QCoreApplication>
#include <QFile>
#include <QTimer>

#include "VS_LAB/clientAPI.h"
#include "VS_LAB/commonAPI.h"
#include "VS_LAB/Macros.h"

#include "VS_LAB/myqtsocket.h"

class VS_Client : public QObject
{
    Q_OBJECT
public:
    VS_Client(QCoreApplication* parent, QString infile, QString outfile, int16_t cid, u_int8_t priority, u_int32_t bca);
    ~VS_Client();

    void start();

private:
    QCoreApplication* parent;
    u_int8_t prio;
    u_int32_t bca;
    int16_t cid;
    QFile* infile;
    QString infile_str;
    QString outfile_str;

    QTimer broadcast_timer, server_active_timer;

    MyQtSocket *sock;

signals:

public slots:

private slots:
    void sendBroadcast();
    void cleanServerList();
    void msgReceive();
};

#endif // VS_CLIENT_H
