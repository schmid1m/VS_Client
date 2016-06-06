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
    for(u_int16_t i; i < server_list_len; i++)
        delete(server_list[i]);
    delete(server_list);
}

bool VS_Client::start()
{
    if(init_client(cid,prio,bca) != SUCCESS)
    {
        cout << "ERROR: Could not init the client API\nAborting Execution\n";
        parent->exit();
        return false;
    }

    infile = new QFile(infile_str);
    if(! infile->exists())
    {
        cout << "ERROR: could not locate infile.\nAborting Execution\n";
        parent->exit();
        return false;
    }
    infile->open(QFile::ReadOnly);
    number_of_blocks = ((((infile->size() - 2) -1)/ 2) / BLOCK_SIZE) + 1;

    cout << "Opened infile " << (dec) << infile_str.toStdString() << ".\n" <<
            "File contains " << number_of_blocks << " Blocks at a block size of " << BLOCK_SIZE << " elements.\n\n";

    readFromFile(&gp, 1);
    cout << "GP set to " << (hex) << gp << (dec) << endl;

//    connect(this, SIGNAL(newMsg(msg,u_int8_t,u_int32_t)),this,SLOT(decodeMsg(msg,u_int8_t,u_int32_t)));

    connect(&MyQtSocket::my_sock, SIGNAL(readyRead()), this,SLOT(msgReceive()));

    broadcast_timer.setInterval(1000);
    connect(&broadcast_timer, SIGNAL(timeout()), this,SLOT(sendBroadcast()));
    broadcast_timer.start();

    server_active_timer.setInterval(1000);
    connect(&server_active_timer, SIGNAL(timeout()), this, SLOT(cleanServerList()));
    server_active_timer.start();

    idle_timeout.setInterval(30000);
    connect(&idle_timeout, SIGNAL(timeout()), this, SLOT(shutDownClient()));
    idle_timeout.start();

    return true;
}

void VS_Client::sendBroadcast()
{
    send_brdcst_req(SERVER_PORT);
    cout << "Sent Broadcast\n";
}

void VS_Client::cleanServerList()
{
    for(u_int16_t i = 0; i < server_list_len; i++)
    {
        if(server_list[i]->getTTD() == 0)
        {
            QHostAddress addr(server_list[i]->getIp());
            server_list[i]->setState(STATE_OFFLINE);
            cout << "Server " << addr.toString().toStdString() << " is offline\n";
        }
        else
            server_list[i]->decrementTTD();
    }

    u_int16_t len_old = server_list_len;
    for(u_int16_t i = 0; i < server_list_len; i++)
    {
        if(server_list[i]->getState() == STATE_OFFLINE)
        {
            delete(server_list[i]);
            for(u_int16_t j = i + 1; j < server_list_len; j++)
            {
                server_list[j - 1] = server_list[j];
            }
            server_list_len--;
            i--;
        }
    }

    if(len_old == server_list_len)
    {
        return;
    }
    if(server_list_len == 0)
        server_list = NULL;
    else
    {
        VS_Server** tmp = new VS_Server*[server_list_len];
        for(u_int16_t i = 0; i < server_list_len; i++)
            tmp[i] = server_list[i];
        delete(server_list);
        server_list = tmp;
    }
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
            idle_timeout.stop();
            idle_timeout.start();
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
    send_gp_req(gp, server_ip, SERVER_PORT);
    cout << "Sent GP request to server " << addr.toString().toStdString() << endl;
}

void VS_Client::shutDownClient()
{
    cout << "No Servers answered broadcasts for 30 soconds\nShutting down\n\n";
    parent->exit();
}

u_int64_t VS_Client::readFromFile(u_int16_t* data, u_int64_t len)
{
    u_int64_t read_len = (u_int64_t)infile->read((char*)data, 2 * len);
    for(u_int64_t i = 0; i < len; i++)
        data[i] = htons(data[i]);
    return read_len;
}
