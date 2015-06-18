#include "TcpServer.h"

const int TcpServer::NETWORK_DELAY = 300;

TcpServer::TcpServer(std::vector<std::vector<int> > *grid, std::vector<std::vector<bool> > &isReservoir, QObject *parent) :
    QTcpServer(parent), grid(grid), isReservoir(isReservoir), mustRemap(false)
{
    if(!listen(QHostAddress::Any, 42042)) {
        throw std::runtime_error(errorString().toStdString());
    }

    connect(this, SIGNAL(newConnection()), this, SLOT(handleConnection()));
    connect(this, SIGNAL(areasRemapped()), this, SLOT(iterate()));
}

TcpServer::~TcpServer()
{
    close();
}



void TcpServer::iterate()
{
    for(int id=0; id < clients.size(); id++) {
        Client& client = clients[id];
        if(!client.ready)
            continue;

        QByteArray msg;
        QDataStream stream(&msg, QIODevice::WriteOnly);
        stream << (quint8)MESSAGE_PROCESS;
        for(size_t col=0; col < grid->at(client.beg).size(); col++)
            stream << dataOf(client.beg,col);
        for(size_t col=0; col < grid->at(client.end).size(); col++)
            stream << dataOf(client.end, col);
        client.sock->write(msg);
    }
    qDebug() << "Iterating";
}

void TcpServer::handleConnection()
{
    while(hasPendingConnections()) {
        QTcpSocket* conn = nextPendingConnection();
        clients.append(Client(conn));
        connect(conn, SIGNAL(readyRead()), this, SLOT(incomingData()));
        connect(conn, SIGNAL(disconnected()), this, SLOT(clientClosing()));

        // Send width
        QByteArray msg;
        QDataStream stream(&msg,QIODevice::WriteOnly);
        stream << (quint8) TcpServer::MESSAGE_WIDTH;
        stream << (quint16) grid->at(0).size();
        conn->write(msg);

        // If first client, re-iterate.
        if(clients.size() == 1) {
            qDebug() << "First client logged. Triggering remap...";

            QTimer::singleShot(TcpServer::NETWORK_DELAY,this, SLOT(remapAreas()));
        }
        else // Already iterating, wait for the iteration to finish
            mustRemap=true;

        emit nbWorkersChanged(clients.size());
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
    client.partialArray += ba;
    QDataStream stream(&client.partialArray, QIODevice::ReadOnly);

    /*
    quint8 type;
    stream >> type;

    qDebug() << "Received " << type;

    bool status=false;
    switch(type) {
        case MESSAGE_OK:
            status=handleOkPacket(sockId);
            break;
        case MESSAGE_DATAFEEDBACK:
            status=handleUpstreamData(sockId);
            break;
        default:
            status=false;
            client.partialArray.clear();
            break;
    }
    */
    bool status=handleUpstreamData(sockId);
    if(status) {
        client.partialArray.clear();
    }
}

bool TcpServer::handleUpstreamData(int clientId)
{
    Client& client = clients[clientId];

    if((size_t)client.partialArray.size() < (client.end-client.beg-1)*(grid->at(0).size())*sizeof(quint16))
        return false; // Wait to receive the full package.

    QDataStream stream(&client.partialArray, QIODevice::ReadOnly);

    // Ignore the first byte
    //stream.skipRawData(sizeof(quint8));

    for(int row=client.beg+1; row < client.end; row++) {
        for(size_t col=0; col < grid->at(row).size(); col++) {
            quint16 data;
            stream >> data;
            grid->at(row)[col] = (data & (~0x8000));
        }
    }

    client.receivedData = true;

    checkRecvStatus();
    return true;
}

bool TcpServer::handleOkPacket(int /*clientId*/)
{
    /*
    Client& client = clients[clientId];

    if(client.pendingOp == 0)
        return true;

    switch(client.pendingOp)
    {
    case MESSAGE_REMAP:
        sendRemapMessage(client);
        break;
    case MESSAGE_PROCESS:
        sendProcessMessage(client);
        break;
    default:
        // Dafuq.
        qDebug("invalid socket pending operation");
        break;
    }
    */ /// Ok has no purpose by now.

    return true;
}

void TcpServer::clientClosing()
{
    QTcpSocket* sock = qobject_cast<QTcpSocket*>(sender());
    int sockId=-1;
    for(int pos=0; pos < clients.size(); pos++) {
        if(sock->peerAddress() == clients[pos].sock->peerAddress() &&
                sock->peerPort() == clients[pos].sock->peerPort())
        {
            sockId = pos;
            break;
        }
    }

    if(sockId < 0)
        return;

    //FIXME handle that way better. This is BUGGY.
    clients.removeAt(sockId);
    mustRemap=true;

    emit nbWorkersChanged(clients.size());
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
        for(int row=cli.beg+1; row < cli.end; row++) { // We don't have to send borders!
            for(size_t col=0; col < grid->at(row).size(); col++)
                stream << dataOf(row,col);
        }
        cli.sock->write(mess);
        cli.ready=true;

        qDebug() << "\tMapped to " << cli.beg << "," << cli.end;
    }

    qDebug() << dataOf(50,50) * 300.0/16000.0 + 1000;

    QTimer::singleShot(TcpServer::NETWORK_DELAY, this, SIGNAL(areasRemapped()));
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

    emit iterated();

    if(mustRemap)
        QTimer::singleShot(2,this,SLOT(remapAreas()));
    else
        QTimer::singleShot(2,this,SLOT(iterate()));
}

quint16 TcpServer::dataOf(int row, int col)
{
    quint16 out=grid->at(row)[col];
    if(isReservoir[row][col])
        out |= 0x8000;
    return out;
}
