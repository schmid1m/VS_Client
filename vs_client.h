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

#include "vs_server.h"

#define BLOCK_SIZE 16//500

class VS_Client : public QObject
{
    Q_OBJECT
public:
    VS_Client(QCoreApplication* parent, QString infile, QString outfile, int16_t cid, u_int8_t priority, u_int32_t bca);
    ~VS_Client();

    bool start();

private:
    QCoreApplication* parent;
    u_int8_t prio;
    u_int32_t bca;
    int16_t cid;
    QFile* infile;
    QString infile_str;
    QString outfile_str;

    u_int64_t number_of_blocks;
    u_int16_t gp;

    QTimer broadcast_timer, server_active_timer, idle_timeout;

    MyQtSocket *sock;

    VS_Server** server_list;
    u_int16_t server_list_len;

    void updateServer(u_int32_t server_ip);
    u_int64_t readFromFile(u_int16_t* data, u_int64_t len);

signals:

public slots:

private slots:
    void sendBroadcast();
    void cleanServerList();
    void msgReceive();
    void shutDownClient();
};

#endif // VS_CLIENT_H
