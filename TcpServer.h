#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <vector>
#include <stdexcept>

class TcpServer : public QTcpServer
{
Q_OBJECT

public:
    TcpServer(std::vector<std::vector<int> >* grid, QObject* parent=0);
    ~TcpServer();

    enum MessageType {
        MESSAGE_PROCESS=1,
            /** Asks the slave to iterate, sends new reservoirs.
             * STRUCTURE: 2*size chars, representing the two lines.
             **/
        MESSAGE_REMAP=2,
            /** Informs the server of a change in map, and sends relevant info.
             * STRUCTURE: (qint16) newAmplitude, (newEnd-newBeg+1)*size characters representing the whole new area.
             **/
        MESSAGE_WIDTH=3
    };

public slots:
    void iterate();

signals:
    void iterated();

private slots:
    void handleConnection();
    void incomingData();
    void clientClosing();
    void remapAreas();
    void checkRecvStatus();

private:

    struct Client {
        Client(QTcpSocket* sock) : receivedData(true),sock(sock){}
        int beg,end; /// beg,end are exclusive, ie. the client will process ]beg,end[ and treat {beg,end} as reservoirs.
        bool receivedData;
        QTcpSocket* sock;
    };

    std::vector<std::vector<int> >* grid;
    QList<Client> clients;
    bool mustRemap;
};

#endif // TCPSERVER_H
