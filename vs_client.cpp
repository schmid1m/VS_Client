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

    server_list = NULL;
    server_list_len = 0;
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
    sock->my_sock.writeDatagram(NULL, 0, QHostAddress::Broadcast, SERVER_PORT);
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
    QHostAddress addr;

    err = sock->Read(&packet, &src_ip, &dummy_port);
    if(err != NO_ERROR)
    {
        cout << "Packet error: " << (int)err << endl;
        free_msg(&packet);
        return;
    }

    addr.setAddress(src_ip);
    FID fid = get_msg_type(&packet);
    switch(fid)
    {
        case BROADCAST_RSP:
            cout << "Received Broadcast response from " << addr.toString().toStdString() << endl;
            err = extract_brdcst_rsp(&packet);
            if (err != NO_ERROR)
            {
                cout << "Got error " << err << "during extraction\n";
            }
            else
            {
                updateServer(src_ip);
            }
            break;
        default:
            break;
    }
    free_msg(&packet);
}

void VS_Client::updateServer(u_int32_t server_ip)
{
    QHostAddress addr(server_ip);
    for(uint16_t i = 0; i < server_list_len; i++)
    {
        if(server_list[i]->getIp() == server_ip)
        {
            server_list[i]->refreshTTD();
            cout << "Updated server with ip " << addr.toString().toStdString() << endl;
            return;
        }
    }

    VS_Server** temp_list = new VS_Server*[server_list_len + 1];
    for(uint16_t i = 0; i < server_list_len; i++)
    {
        temp_list[i] = server_list[i];
    }
    delete(server_list);
    server_list = temp_list;
    server_list[server_list_len] = new VS_Server(server_ip, STATE_ONLINE);
    server_list_len ++;
    cout << "Added server with ip " << addr.toString().toStdString() << endl;
}
