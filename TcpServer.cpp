#include "TcpServer.h"

TcpServer::TcpServer(std::vector<std::vector<int> > *grid, QObject *parent) :
    QTcpServer(parent), grid(grid), mustRemap(false)
{
    if(!listen(QHostAddress::Any, 42042)) {
        throw std::runtime_error(errorString().toStdString());
    }

    connect(this, SIGNAL(newConnection()), this, SLOT(handleConnection()));
}

TcpServer::~TcpServer()
{
    close();
}

void TcpServer::iterate()
{
    for(int id=0; id < clients.size(); id++) {
        Client& client = clients[id];
        QByteArray msg;
        QDataStream stream(&msg, QIODevice::WriteOnly);
        stream << (quint8)MESSAGE_PROCESS;
        for(size_t col=0; col < grid->at(client.beg).size(); col++)
            stream << (quint16) grid->at(client.beg)[col];
        for(size_t col=0; col < grid->at(client.end).size(); col++)
            stream << (quint16) grid->at(client.end)[col];
        client.sock->write(msg);
    }
}

void TcpServer::handleConnection()
{
    while(hasPendingConnections()) {
        QTcpSocket* conn = nextPendingConnection();
        clients.append(Client(conn));
        mustRemap = true;
        connect(conn, SIGNAL(readyRead()), this, SLOT(incomingData()));
        connect(conn, SIGNAL(aboutToClose()), this, SLOT(clientClosing()));

        // Send width
        QByteArray msg;
        QDataStream stream(&msg,QIODevice::WriteOnly);
        stream << (quint8) TcpServer::MESSAGE_WIDTH;
        stream << (quint16) grid->at(0).size();
        conn->write(msg);

        // If first client, re-iterate.
        if(clients.size() == 1)
            iterate();
    }
}

void TcpServer::incomingData()
{
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    int sockId;
    for(int pos=0; pos < clients.size(); pos++) {
        if(sock->peerAddress() == clients[pos].sock->peerAddress() &&
                sock->peerPort() == clients[pos].sock->peerPort())
        {
            sockId = pos;
            break;
        }
    }
    Client& client = clients[sockId];

    QByteArray ba = sock->readAll();
    QDataStream stream(&ba, QIODevice::ReadOnly);
    if((size_t)ba.size() != (client.end-client.beg-1)*(grid->at(0).size())) {
        throw std::runtime_error("Too few data received");
    }

    for(int row=client.beg+1; row < client.end; row++) {
        for(size_t col=0; col < grid->at(row).size(); col++) {
            quint16 data;
            stream >> data;
            grid->at(row)[col] = data;
        }
    }
    client.receivedData = true;

    checkRecvStatus();
}

void TcpServer::clientClosing()
{
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    int sockId;
    for(int pos=0; pos < clients.size(); pos++) {
        if(sock->peerAddress() == clients[pos].sock->peerAddress() &&
                sock->peerPort() == clients[pos].sock->peerPort())
        {
            sockId = pos;
            break;
        }
    }

    //TODO handle that way better. This is BUGGY.
    clients.removeAt(sockId);
    mustRemap=true;
}

void TcpServer::remapAreas()
{
    mustRemap=false;
    int step = (grid->size()-2)/clients.size();
    for(int id=0; id < clients.size(); id++) {
        clients[id].beg = id * step;
        clients[id].end = (id + 1) * step + 1;
    }
    clients[clients.size()-1].end = grid->size()-1;

    for(int id=0; id < clients.size(); id++) {
        Client& cli = clients[id];
        QByteArray mess;
        QDataStream stream(&mess, QIODevice::WriteOnly);

        stream << (quint8) TcpServer::MESSAGE_REMAP;
        stream << (quint16) (cli.end-cli.beg+1);
        for(int row=cli.beg; row <= cli.end; row++) {
            for(size_t col=0; col < grid->at(row).size(); col++)
                stream << (quint16) grid->at(row)[col];
        }
        cli.sock->write(mess);
    }
}

void TcpServer::checkRecvStatus()
{
    for(int id=0; id < clients.size(); id++) {
        if(!clients[id].receivedData)
            return;
    }
    // Else, we've received all we could.
    for(int id=0; id < clients.size(); id++) {
        clients[id].receivedData = false;
    }
    if(mustRemap){
        remapAreas();
    }

    emit iterated();

    iterate();
}
