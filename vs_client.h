#ifndef VS_CLIENT_H
#define VS_CLIENT_H

#include <QObject>
#include <QCoreApplication>
#include <QFile>
#include <QTimer>

#include "VS_LAB/clientAPI.h"
#include "VS_LAB/commonAPI.h"
#include "VS_LAB/Macros.h"

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

    QTimer broadcastTimer, serverActiveTimer;

signals:
    void startReceiving();
    void newMsg(msg packet, u_int8_t err, u_int32_t src_ip);

public slots:

private slots:
    void sendBroadcast();
    void cleanServerList();
    void msgReceive();
    void decodeMsg(msg packet, u_int8_t err, u_int32_t src_ip);
};

#endif // VS_CLIENT_H
