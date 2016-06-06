#include "vs_client.h"
#include <iostream>

using namespace std;

VS_Client::VS_Client(QCoreApplication *parent, QString infile, QString outfile, int16_t cid, u_int8_t priority, u_int32_t bca)
{
    this->parent = parent;
    this->cid = cid;
    this->prio = priority;
    this->bca = bca;
    this->infile_str = infile;
    this->outfile_str = outfile;

    sock = new MyQtSocket;
}

VS_Client::~VS_Client()
{

}

void VS_Client::start()
{
    if(init_client(cid,prio,bca) != SUCCESS)
    {
        cout << "ERROR: Could not init the client API\n Aborting Execution\n";
        parent->exit();
    }
//    connect(this, SIGNAL(newMsg(msg,u_int8_t,u_int32_t)),this,SLOT(decodeMsg(msg,u_int8_t,u_int32_t)));

    connect(&MyQtSocket::my_sock, SIGNAL(readyRead()), this,SLOT(msgReceive()));

    broadcast_timer.setInterval(1000);
    connect(&broadcast_timer, SIGNAL(timeout()), this,SLOT(sendBroadcast()));
    broadcast_timer.start();

    server_active_timer.setInterval(1000);
    connect(&server_active_timer, SIGNAL(timeout()), this, SLOT(cleanServerList()));
    server_active_timer.start();
}

void VS_Client::sendBroadcast()
{
    send_brdcst_req(SERVER_PORT);
    cout << "Sent Broadcast\n";
}

void VS_Client::cleanServerList()
{

}

void VS_Client::msgReceive()
{
    cout<<"Packet Received\n";
    uint8_t err;
    msg packet;
    uint16_t dummy_port;
    uint32_t src_ip;
    err = sock->Read(&packet, &src_ip, &dummy_port);
    cout << "error: " << (int)err << endl;
}
