#include <QCoreApplication>
#include <QHostAddress>
#include <iostream>
#include "vs_client.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    VS_Client *client;

    QString   infile, outfile;
    int16_t   clientID;
    u_int16_t  priority;
    u_int32_t broadcastIP;

    if(argc != 6)
    {
        cout << "Wrong number of arguments!" << endl <<
                "Usage: <infile> <outfile> <cid> <prio> <ip_address>\n";
                return 0;
    }

    infile   = argv[1];
    outfile  = argv[2];
    clientID = QString(argv[3]).toShort();
    priority = QString(argv[4]).toUShort();
    broadcastIP = QHostAddress(argv[5]).toIPv4Address();

    cout << "VS_lab Client\n" <<
            "LAB          : 3\n" <<
            "Version      : 0.1\n" <<
            "Author       : Michel Schmidt\n" <<
            "=============================\n" <<
            "Infile       : " << infile.toStdString() << endl <<
            "outfile      : " << outfile.toStdString() << endl <<
            "client IP    : " << clientID << endl <<
            "Priority     : " << (u_int16_t)priority << endl <<
            "Broadcast IP : " << argv[5] << endl <<
            "               " << (hex) << broadcastIP << "\n\n";


    client = new VS_Client(&a,  infile, outfile, clientID, (u_int8_t)priority, broadcastIP);

    client->start();

    return a.exec();
}
