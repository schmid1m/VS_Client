#include "vs_client.h"
#include <iostream>
#include <QDir>

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

    QDir tmp_dir;
    tmp_dir.setPath("./tmp");
    if(!tmp_dir.exists())
    {
        tmp_dir.setPath(".");
        tmp_dir.makeAbsolute();
        tmp_dir.mkdir("./tmp");
    }
}

VS_Client::~VS_Client()
{
    for(u_int16_t i = 0; i < server_list_len; i++)
        delete(server_list[i]);
    delete(server_list);
}

bool VS_Client::start()
{
    if(init_client(cid,prio,bca) != SUCCESS)
    {
        cout << "ERROR: Could not init the client API\nAborting Execution\n";
        return false;
    }

    infile = new QFile(infile_str);
    if(! infile->exists())
    {
        cout << "ERROR: could not locate infile.\nAborting Execution\n";
        return false;
    }
    infile->open(QFile::ReadOnly);
    number_of_blocks = ((((infile->size() - 2) -1)/ 2) / BLOCK_SIZE) + 1;
    if(number_of_blocks % 2 == 1)
    {
        cout << "Fatal Error: Infile has odd number of bytes.\n";
        return false;
    }
    blocks = new BLOCK_STATE[number_of_blocks];
    for(u_int64_t i = 0; i < number_of_blocks; i++)
        blocks[i] = BLK_STATE_TODO;

    cout << "Opened infile " << (dec) << infile_str.toStdString() << ".\n" <<
            "File contains " << number_of_blocks << " Blocks at a block size of " << BLOCK_SIZE << " elements.\n\n";

    readFromFile(&gp, 1, 0);
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
    QHostAddress addr;
    for(u_int16_t i = 0; i < server_list_len; i++)
    {
        if(server_list[i]->getTTD() == 0)
        {
            addr.setAddress(server_list[i]->getIp());
            server_list[i]->setState(STATE_OFFLINE);
            u_int64_t block = server_list[i]->getBlockId();
            if(block < number_of_blocks)
                blocks[block] = BLK_STATE_TODO;
            cout << "Server " << addr.toString().toStdString() << " is offline\n";
        }
        else
            server_list[i]->decrementTTD();
    }

    u_int16_t len_old = server_list_len;
    for(u_int16_t i = 0; i < server_list_len; i++)
    {
        addr.setAddress(server_list[i]->getIp());
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
        else if(server_list[i]->getState() == STATE_ONLINE)
        {
            if(number_of_blocks > getNextFreeBlock())
            {
                send_gp_req(gp, server_list[i]->getIp(), SERVER_PORT);
                cout << "Sent GP request to server " << addr.toString().toStdString() << endl;
            }
        }
        else if(server_list[i]->getState() == STATE_LOCKED)
        {
            if(number_of_blocks > getNextFreeBlock())
            {
                sendBlock(server_list[i]->getIp());
            }
            else
            {
                send_unlock_req(server_list[i]->getIp(),SERVER_PORT);
                cout << "Unlocked server " << addr.toString().toStdString() << endl;
                server_list[i]->setState(STATE_ONLINE);
            }
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
    uint8_t err;
    msg packet;
    uint16_t dummy_port;
    uint32_t src_ip;
    QHostAddress addr;
    u_int16_t bid;
    u_int8_t* data;
    u_int32_t data_len;
    uint8_t err_code;

    while(sock->my_sock.bytesAvailable() != 0)
    {
        err = sock->Read(&packet, &src_ip, &dummy_port);
        if(err != NO_ERROR)
        {
            cout << "Packet error: " << (int)err << endl;
            free_msg(&packet);
            continue;
        }
        idle_timeout.stop();
        idle_timeout.start();

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
            case GP_RSP:
                cout << "Received GP response from " << addr.toString().toStdString() << endl;
                err = extract_gp_rsp(&packet);
                if (err != NO_ERROR)
                {
                    cout << "Got error " << err << "during extraction\n";
                }
                else
                {
                    setServerState(src_ip, STATE_LOCKED);
                    sendBlock(src_ip);
                }
                break;
            case DECRYPT_RSP:
                cout << "Received Decrypt response from " << addr.toString().toStdString() << endl;
                err = extract_dec_rsp(&packet, &bid, &data, &data_len);
                if (err != NO_ERROR)
                {
                    cout << "Got error " << err << "during extraction\n";
                }
                if(data_len < 65)
                {
                    data[data_len] = '\0';
                    cout << "Decrypted text\"" << data << "\"\n";
                }
                if(!saveBlock(data,data_len,bid))
                {
                    cout << "Unable to save block\n";
                }
                setServerState(src_ip, STATE_LOCKED);
                sendBlock(src_ip);
                break;
            case UNLOCK_RSP:
                cout << "Received Unlock response from " << addr.toString().toStdString() << endl;
                err = extract_unlock_rsp(&packet);
                if (err != NO_ERROR)
                {
                    cout << "Got error " << err << "during extraction\n";
                }
                break;
            case ERROR_RSP:
                cout << "Received Error frame from " << addr.toString().toStdString() << endl;
                err = extract_error_rsp(&packet, &err_code, &bid);
                if (err != NO_ERROR)
                {
                    cout << "Got error " << err << "during extraction\n";
                }
                cout << "Received error " << err_code << " from " << addr.toString().toStdString() << endl;
                break;
            default:
                cout<<"Unknown Packet Received from " << addr.toString().toStdString() << endl;
                switch(err_code)
                {
                    case ERR_SERVERINUSE:
                        break;
                    case ERR_LOCK_TIMEOUT:
                        break;
                    case ERR_NOTFORME:
                        break;
                    case ERR_NO_GP:
                        break;
                    case ERR_DECRYPT:
                        break;
                    default:
                        break;
                }
                break;
        }
        free_msg(&packet);
    }
}

void VS_Client::updateServer(u_int32_t server_ip)
{
    QHostAddress addr(server_ip);
    for(uint16_t i = 0; i < server_list_len; i++)
    {
        if(server_list[i]->getIp() == server_ip)
        {
            if(server_list[i]->getState() == STATE_DECRYPTING)
            {
                server_list[i]->refreshTTD();
                cout << "Updated server with ip " << addr.toString().toStdString() << endl;
            }
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
    if(number_of_blocks > getNextFreeBlock())
    {
        send_gp_req(gp, server_ip, SERVER_PORT);
        cout << "Sent GP request to server " << addr.toString().toStdString() << endl;
    }
}

void VS_Client::shutDownClient()
{
    cout << "No Servers answered broadcasts for 30 soconds\nShutting down\n\n";
    parent->exit();
}

u_int64_t VS_Client::readFromFile(u_int16_t* data, u_int64_t len, uint64_t pos)
{
    infile->seek(pos);
    u_int64_t read_len = (u_int64_t)infile->read((char*)data, 2 * len);
    for(u_int64_t i = 0; i < len; i++)
        data[i] = htons(data[i]);
    return read_len;
}

void VS_Client::setServerState(u_int32_t server_ip, SERVER_STATE state)
{
    QHostAddress addr(server_ip);
    for(uint16_t i = 0; i < server_list_len; i++)
    {
        if(server_list[i]->getIp() == server_ip)
        {
            server_list[i]->refreshTTD();
            server_list[i]->setState(state);
            server_list[i]->setBlockId(0xFFFFFFFF);
            cout << "Server " << addr.toString().toStdString() << " set to State " << state << endl;
            return;
        }
    }
}

void VS_Client::sendBlock(u_int32_t server_ip)
{
    QHostAddress addr(server_ip);

    u_int64_t blk = getNextFreeBlock();

    for(uint16_t i = 0; i < server_list_len; i++)
    {
        if(server_list[i]->getIp() == server_ip)
        {
            if(blk < number_of_blocks)
            {
                u_int64_t read_len = readFromFile(in_data_buff, BLOCK_SIZE, 2 + (2 * BLOCK_SIZE * blk));
                if(read_len % 2 == 1)
                {
                    cout << "Fatal Error: Could only read odd number of bytes from file\n";
                    parent->exit();
                }
                server_list[i]->setBlockId(blk);
                server_list[i]->setState(STATE_DECRYPTING);
                server_list[i]->refreshTTD();
                send_dec_req(blk & 0xFFFF, in_data_buff, read_len / 2, server_ip, SERVER_PORT);
                cout << "Sent out block " << blk << " to be decrypted by " << addr.toString().toStdString() << endl;
                blocks[blk] = BLK_STATE_RUNNING;
            }
            else
            {
                send_unlock_req(server_list[i]->getIp(),SERVER_PORT);
                cout << "Unlocked server " << addr.toString().toStdString() << endl;
                server_list[i]->setState(STATE_ONLINE);
            }
            return;
        }
    }
    cout << "Could not find server " << addr.toString().toStdString() << " in list!\n";
}

u_int64_t VS_Client::getNextFreeBlock()
{
    bool running_found = false;
    for(u_int64_t i = 0; i < number_of_blocks; i++)
    {
        if(blocks[i] == BLK_STATE_TODO)
            return i;
        if(blocks[i] == BLK_STATE_RUNNING)
            running_found = true;
    }
    if(!running_found)
    {
        saveFinalFile();
    }
    return number_of_blocks;
}

u_int64_t VS_Client::resolveShortBid(u_int16_t bid)
{
    for(u_int64_t i = 0; i < number_of_blocks; i++)
    {
        if((i & 0xFFFF) == bid)
        {
            if(blocks[i] == BLK_STATE_RUNNING)
                return i;
        }
    }
    return number_of_blocks;
}

bool VS_Client::saveBlock(u_int8_t *data, u_int64_t data_len, u_int16_t bid)
{
    u_int64_t full_bid = resolveShortBid(bid);
    if(full_bid < number_of_blocks)
    {
        QFile tmp_file;
        tmp_file.setFileName("tmp/" + infile_str + "_part" + QString::number(full_bid) + ".tmp");
        tmp_file.open(QFile::WriteOnly);
        tmp_file.write((const char*)data,data_len);
        tmp_file.close();
        blocks[full_bid] = BLK_STATE_DONE;
        return true;
    }
    else
    {
        return false;
    }
    return false;
}

void VS_Client::saveFinalFile()
{
    QFile file;
    bool blk_state_changed = false;
    for(u_int64_t i = 0; i < number_of_blocks; i++)
    {
        file.setFileName("tmp/" + infile_str + "_part" + QString::number(i) + ".tmp");
        if(!file.exists())
        {
            blocks[i] = BLK_STATE_TODO;
            blk_state_changed = true;
        }
    }
    if(blk_state_changed)
        return;

    QFile outfile;
    outfile.setFileName(outfile_str);
    outfile.open(QFile::WriteOnly);
    QByteArray txt;
    for(u_int64_t i = 0; i < number_of_blocks; i++)
    {
        file.setFileName("tmp/" + infile_str + "_part" + QString::number(i) + ".tmp");
        file.open(QFile::ReadOnly);
        txt = file.readAll();
        outfile.write(txt);
        file.close();
        file.remove();
    }
    infile->close();
    outfile.close();
    cout << "Decryption done!\n\n";
    parent->exit();
}
