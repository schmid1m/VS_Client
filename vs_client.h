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

//#define BLOCK_SIZE 16500
#define BLOCK_SIZE 5000

typedef enum {
    BLK_STATE_TODO,
    BLK_STATE_RUNNING,
    BLK_STATE_DONE
}BLOCK_STATE;

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
    BLOCK_STATE *blocks;

    u_int16_t gp;

    QTimer broadcast_timer, server_active_timer, idle_timeout;

    MyQtSocket *sock;

    VS_Server** server_list;
    u_int16_t server_list_len;

    uint16_t in_data_buff[BLOCK_SIZE];

    void updateServer(u_int32_t server_ip);
    void setServerState(u_int32_t server_ip, SERVER_STATE state);
    u_int64_t readFromFile(u_int16_t* data, u_int64_t len, u_int64_t pos);
    void sendBlock(u_int32_t server_ip);
    u_int64_t getNextFreeBlock();
    u_int64_t resolveShortBid(u_int16_t bid);
    bool saveBlock(u_int8_t* data, u_int64_t data_len, u_int16_t bid);
    void saveFinalFile();

signals:

public slots:

private slots:
    void sendBroadcast();
    void cleanServerList();
    void msgReceive();
    void shutDownClient();
};

#endif // VS_CLIENT_H
