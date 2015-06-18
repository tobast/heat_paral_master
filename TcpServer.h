#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QDebug> //FIXME remove
#include <vector>
#include <stdexcept>
#include <QTimer>

class TcpServer : public QTcpServer
{
Q_OBJECT

public:
    TcpServer(std::vector<std::vector<int> >* grid, std::vector<std::vector<bool> >& isReservoir,
              QObject* parent=0);
    ~TcpServer();

    enum MessageType {
        MESSAGE_PROCESS=1,
            /** Asks the slave to iterate, sends new reservoirs.
             * STRUCTURE: 2*size chars, representing the two lines.
             **/
        MESSAGE_REMAP=2,
            /** Informs the server of a change in map, and sends relevant info.
             * STRUCTURE: (qint16) newAmplitude, (quint16)*(newEnd-newBeg+1)*size representing the whole new area.
             **/
        MESSAGE_WIDTH=3,
            /** Informs the slave of the map width
             * STRUCTURE: (quint16) new width
             **/
        MESSAGE_OK=4,
            /** Does not carry any data, informs the peer that the packet was fully received. **/
        MESSAGE_DATAFEEDBACK=5
            /** Sends the processed data upstream
             * STRUCTURE: (quint16)*(end-beg-1)*width representing the processed area.
             **/
    };

public slots:
    void iterate();

signals:
    void iterated();
    void areasRemapped();
    void nbWorkersChanged(int num);

private slots:
    void handleConnection();
    void incomingData();
    bool handleUpstreamData(int clientId);
    bool handleOkPacket(int clientId);

    void clientClosing();
    void remapAreas();
    void checkRecvStatus();

private: //meth
    quint16 dataOf(int row, int col);

private:

    struct Client {
        Client(QTcpSocket* sock) : receivedData(true),sock(sock),partialArray(), ready(false){}
        int beg,end; /// beg,end are exclusive, ie. the client will process ]beg,end[ and treat {beg,end} as reservoirs.
        bool receivedData;
        QTcpSocket* sock;
        QByteArray partialArray;
//        quint8 pendingOp;
        bool ready;
    };

    std::vector<std::vector<int> >* grid;
    std::vector<std::vector<bool> > isReservoir;
    QList<Client> clients;
    bool mustRemap;

    static const int NETWORK_DELAY;
};

#endif // TCPSERVER_H
