#ifndef VS_SERVER_H
#define VS_SERVER_H

#include <QObject>

#define TIME_TO_DEATH 5

typedef enum {
    STATE_OFFLINE,
    STATE_ONLINE,
    STATE_LOCKED,
    STATE_DECRYPTING,
    STATE_ERROR,
    STATE_UNKNOWN
}SERVER_STATE;

class VS_Server : public QObject
{
    Q_OBJECT
public:
    VS_Server(u_int32_t ip, SERVER_STATE state = STATE_OFFLINE);
    ~VS_Server();

    u_int32_t getIp();
    SERVER_STATE getState();
    u_int8_t getTTD();
    void setState(SERVER_STATE state);
    void refreshTTD();
    void decrementTTD();

private:
    u_int32_t server_ip;
    SERVER_STATE server_status;
    u_int8_t time_to_death;
};

#endif // VS_SERVER_H
